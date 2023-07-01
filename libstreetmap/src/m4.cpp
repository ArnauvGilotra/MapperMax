/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */

#include "m4.h"
#include <iostream>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

//libraries being added
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <thread>
#include <chrono>
#include "globals.h"
#include <queue>
#include <utility>

// This routine takes in a vector of D deliveries (pickUp, dropOff
// intersection pairs), another vector of N intersections that
// are legal start and end points for the path (depots), and a turn 
// penalty in seconds (see m3.h for details on turn penalties).
//
// The first vector 'deliveries' gives the delivery information.  Each delivery
// in this vector has pickUp and dropOff intersection ids.
// A delivery can only be dropped-off after the associated item has been picked-up. 
// 
// The second vector 'depots' gives the intersection ids of courier company
// depots containing trucks; you start at any one of these depots and end at
// any one of the depots.
//
// This routine returns a vector of CourierSubPath objects that form a delivery route.
// The CourierSubPath is as defined above. The first street segment id in the
// first subpath is connected to a depot intersection, and the last street
// segment id of the last subpath also connects to a depot intersection.
// A package will not be dropped off if you haven't picked it up yet.
//
// The start_intersection of each subpath in the returned vector should be 
// at least one of the following (a pick-up and/or drop-off can only happen at 
// the start_intersection of a CourierSubPath object):
//      1- A start depot.
//      2- A pick-up location
//      3- A drop-off location. 
//
// You can assume that D is always at least one and N is always at least one
// (i.e. both input vectors are non-empty).
//
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages. Additionally, 
// one traversal of an intersection is sufficient to drop off all the 
// (already picked up) packages that need to be dropped off at that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.

IntersectionIndex closestLegalStop(IntersectionIndex currNode, std::vector<std::vector<double>> array,
                                   bool iterator, int &iCounter, int pickedRandom);
IntersectionIdx findClosestDepot(IntersectionIndex currNode, std::vector<std::vector<double>> array,
        std::vector<IntersectionIdx> depots, int delSize);

void populateArray(std::vector<std::vector<double>>& arrayBestTimes, int initial, int end, const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const double turn_penalty);

int iterationGlobal;

struct distanceVector{
    IntersectionIdx inter = -1;
    int index = -1;
    double distance = -1;
    distanceVector(IntersectionIdx i, int ind, double d){
        inter = i;
        index = ind;
        distance = d;
    }

};

std::vector<CourierSubPath> PartTravelingCourier(
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const float turn_penalty,
                            int startingDepot,
                            std::vector<std::vector<double>> arrayBestTimes,
                            bool iterator);


std::vector<m4Node> stopInfo;




std::vector<CourierSubPath> PartTravelingCourier(
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const float turn_penalty,
                            int startingDepot,
                            std::vector<std::vector<double>> arrayBestTimes,
                            bool iterative){
    

    //vector we will return
    int iterativeCounter = 0;
    std::vector<CourierSubPath> finalPaths;
    finalPaths.clear();
    
    //first store in a vector of structs
    
    stopInfo.clear();
    int totalDeliveries = deliveries.size();
    //int numStops = 0;
    
    for(int i=0;i<totalDeliveries;i++){
        stopInfo.push_back(m4Node(false, deliveries[i].pickUp, deliveries[i].dropOff));
    }
    
    int rIterator = rand() % stopInfo.size(); 
    
    CourierSubPath path;
    IntersectionIndex currDepot(depots[startingDepot], startingDepot+2*deliveries.size());
    IntersectionIndex closestStop = closestLegalStop(currDepot,
            arrayBestTimes, iterative, iterativeCounter, rIterator);
    std::pair<int, int> myPair(depots[startingDepot],closestStop.intersectionID);
    path.subpath=findPathBetweenIntersections(myPair, turn_penalty);
    path.start_intersection=depots[startingDepot];
    path.end_intersection=closestStop.intersectionID;
    finalPaths.push_back(path);
    
    
    for(int i=0;i<(totalDeliveries*2-1);i++){

        IntersectionIndex nextClosest = closestLegalStop(closestStop, 
                                arrayBestTimes, iterative, iterativeCounter,
                                rIterator);
        if (nextClosest.intersectionID == -1) break;

        std::pair<int, int> interPair(closestStop.intersectionID,nextClosest.intersectionID);
        path.subpath=findPathBetweenIntersections(interPair, turn_penalty);
        if((path.subpath).size()==0){ //if no legal route
            finalPaths.clear();
            return finalPaths; //return 0 vector
        }
        path.start_intersection=closestStop.intersectionID;
        path.end_intersection=nextClosest.intersectionID;
        finalPaths.push_back(path);    
        closestStop = nextClosest;
    }
    
    
    IntersectionIdx finalDepot = findClosestDepot(closestStop, arrayBestTimes,depots, totalDeliveries*2);
    std::pair<int, int> lastPair(closestStop.intersectionID,finalDepot);
    path.subpath=findPathBetweenIntersections(lastPair, turn_penalty);
    path.start_intersection=closestStop.intersectionID;
    path.end_intersection=finalDepot;
    finalPaths.push_back(path);
    

    
    return finalPaths;

}

//using crow-flies
IntersectionIndex closestLegalStop(IntersectionIndex currNode, 
                                    std::vector<std::vector<double>> arrayBestTimes,
                                    bool iterative, int& iCounter, int pickedRandom){
    IntersectionIndex closestNode(-1,-1);
    iCounter++;
    std::vector<distanceVector> distancesV;
    double shortestPath = INFINITY;

    for(int i=0; i<stopInfo.size(); i++){

        if(stopInfo[i].delCompleted) continue;

        if(stopInfo[i].dropoffAvailable){
            double distanceBetweenPoints = arrayBestTimes[currNode.index][2*i+1];

            distancesV.push_back(distanceVector(stopInfo[i].drop, 2*i+1, distanceBetweenPoints));
            if(distanceBetweenPoints<shortestPath){
                

                shortestPath = arrayBestTimes[currNode.index][2*i+1];
                closestNode.intersectionID=stopInfo[i].drop;
                closestNode.index = 2*i+1;

            }
        }
        else{
            double distanceBetweenPoints = arrayBestTimes[currNode.index][2*i];
            distancesV.push_back(distanceVector(stopInfo[i].pick, 2*i, distanceBetweenPoints));
            if(distanceBetweenPoints<shortestPath){
                shortestPath = arrayBestTimes[currNode.index][2*i];
                closestNode.intersectionID=stopInfo[i].pick;
                closestNode.index = 2*i;

            }
        }
    }
    if(iterative&&iCounter == pickedRandom){
        int numOfNodes = distancesV.size();
        int randIndex = rand()%numOfNodes;
//        std::cout<<"randIndex ="<<randIndex<<std::endl;
        closestNode.intersectionID = distancesV[randIndex].inter;
        closestNode.index = distancesV[randIndex].index;
    }
        
    for(auto i = stopInfo.begin() ; i!=stopInfo.end() ; i++){
        if(i->pick == closestNode.intersectionID)
        i->dropoffAvailable=true;
        if(i->drop == closestNode.intersectionID && i->dropoffAvailable == true)
        i->delCompleted = true;
    }
    

    return closestNode;
    
}


IntersectionIdx findClosestDepot(IntersectionIndex currNode, std::vector<std::vector<double>> arrayBestTimes, 
        std::vector<IntersectionIdx> depots, int delSize){
    double smallestDist = INFINITY;
    IntersectionIdx closestDepot = -1;
    for(int i=0;i<depots.size();i++){
        double distBetween = arrayBestTimes[currNode.index][i+delSize];
        if(distBetween<smallestDist){
            smallestDist = distBetween;
            closestDepot = depots[i];
        }
        
    }
    
    return closestDepot;
}

void populateArray(std::vector<std::vector<double>>& arrayBestTimes, int initial, int end, const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const double turn_penalty){
    for(int i = initial ; i<end;i++){
        arrayBestTimes[2*i]=(multidjikstra(deliveries[i].pickUp, deliveries, depots, turn_penalty));
        arrayBestTimes[2*i+1]=(multidjikstra(deliveries[i].dropOff, deliveries, depots, turn_penalty));
    }
}

std::vector<CourierSubPath> travelingCourier(
                            const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const float turn_penalty){
    
    srand(time(0));
    
    auto startTime = std::chrono::high_resolution_clock::now();
    bool timeOut = false;
    std::vector<std::vector<CourierSubPath>> testPath;
    int finalPath=-1;
    double bestPathTime=INFINITY;
    testPath.clear();
    std::vector<std::vector<double>> arrayBestTimes;
    int arraySize = depots.size()+deliveries.size()*2;
    arrayBestTimes.resize(arraySize);
    
        

    std::thread thread1(populateArray,std::ref(arrayBestTimes), 0, deliveries.size()/8, deliveries, depots, turn_penalty);
    std::thread thread2(populateArray,std::ref(arrayBestTimes), deliveries.size()/8, deliveries.size()/4, deliveries, depots, turn_penalty);
    std::thread thread3(populateArray,std::ref(arrayBestTimes), deliveries.size()/4, deliveries.size()*3/8, deliveries, depots, turn_penalty);
    std::thread thread4(populateArray,std::ref(arrayBestTimes), deliveries.size()*3/8, deliveries.size()/2, deliveries, depots, turn_penalty);
    std::thread thread5(populateArray,std::ref(arrayBestTimes), deliveries.size()/2, deliveries.size()*5/8, deliveries, depots, turn_penalty);
    std::thread thread6(populateArray,std::ref(arrayBestTimes), deliveries.size()*5/8, deliveries.size()*3/4, deliveries, depots, turn_penalty);
    std::thread thread7(populateArray,std::ref(arrayBestTimes), deliveries.size()*3/4, deliveries.size()*7/8, deliveries, depots, turn_penalty);
    std::thread thread8(populateArray,std::ref(arrayBestTimes), deliveries.size()*7/8, deliveries.size(), deliveries, depots, turn_penalty);

    for(int i = 0 ; i<depots.size(); i++){
        int index = (deliveries.size()*2)+i;
        arrayBestTimes[index]=(multidjikstra(depots[i], deliveries, depots, turn_penalty));
    }
    
    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();
    thread7.join();
    thread8.join();

    for(int i=0; i<depots.size(); i++){
        double totalPathTime=0;
        testPath.push_back(PartTravelingCourier(deliveries, depots, turn_penalty, i, arrayBestTimes, false));
        for(int j=0; j<testPath[i].size(); j++){
            totalPathTime += computePathTravelTime(testPath[i][j].subpath, turn_penalty);
        }
//        std::cout<<"Path time "<<i<<":"<<totalPathTime<<std::endl;
        if(totalPathTime<bestPathTime){
            bestPathTime = totalPathTime;
            finalPath=i;
        }
    }
        
    std::vector<CourierSubPath> newBestPath = testPath[finalPath];    
    double newBestTime = bestPathTime;
    
    while(!timeOut){
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>> (
                                                    currentTime - startTime);
        
        if (wallClock.count() >0.95 * 50){
            timeOut = true;
            break;
        }
        
        double iterativeCurrentTime = 0;
        std::vector<CourierSubPath> iterativeTest = PartTravelingCourier(deliveries, depots, 
                turn_penalty, finalPath, arrayBestTimes, true);
        
        for(int i=0; i<iterativeTest.size();i++){    
            iterativeCurrentTime += computePathTravelTime(iterativeTest[i].subpath, turn_penalty);      
        }
        
//        std::cout<< "new Iterative CurrTime = " << iterativeCurrentTime <<std::endl;
        if(iterativeCurrentTime<newBestTime){
            newBestPath = iterativeTest;
            newBestTime = iterativeCurrentTime;
        } 
        
    }
    return newBestPath;
    
}

////------------------

//std::vector<CourierSubPath> travelingCourier(
//                            const std::vector<DeliveryInf>& deliveries,
//                            const std::vector<IntersectionIdx>& depots,
//                            const float turn_penalty){
//    std::vector<CourierSubPath> finalPaths;
//    finalPaths.clear();
//    CourierSubPath path;
//    std::pair<int, int> myPair(depots[0],deliveries[0].pickUp);
//    path.subpath=findPathBetweenIntersections(myPair, turn_penalty);
//    path.start_intersection=depots[0];
//    path.end_intersection=deliveries[0].pickUp;
//    finalPaths.push_back(path);
//    
//    int totalDeliveries = deliveries.size();
//    
//    for(int i=0;i<totalDeliveries-1; i++){
//        path.start_intersection=deliveries[i].pickUp;
//        path.end_intersection=deliveries[i].dropOff;
//        std::pair<int, int> firstPair(deliveries[i].pickUp,deliveries[i].dropOff);
//        path.subpath=findPathBetweenIntersections(firstPair, turn_penalty);
//        finalPaths.push_back(path);
//        
//        path.start_intersection=deliveries[i].dropOff;
//        path.end_intersection=deliveries[i+1].pickUp;
//        std::pair<int, int> secondPair(deliveries[i].dropOff,deliveries[i+1].pickUp);
//        path.subpath=findPathBetweenIntersections(secondPair, turn_penalty);
//        finalPaths.push_back(path);       
//    }
//    
//    path.start_intersection=deliveries[totalDeliveries-1].pickUp;
//    path.end_intersection=deliveries[totalDeliveries-1].dropOff;
//    std::pair<int, int> thirdPair(deliveries[totalDeliveries-1].pickUp,deliveries[totalDeliveries-1].dropOff);
//    path.subpath=findPathBetweenIntersections(thirdPair, turn_penalty);
//    finalPaths.push_back(path);
//    
//    path.start_intersection=deliveries[totalDeliveries-1].dropOff;
//    path.end_intersection=depots[0];
//    std::pair<int, int> lastPair(deliveries[totalDeliveries-1].dropOff, depots[0]);
//    path.subpath=findPathBetweenIntersections(lastPair, turn_penalty);
//    finalPaths.push_back(path);
//    
//    return finalPaths;
//
//