/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */
#include "m3.h"
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

//-----------------------------------------------------------------------------

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.

std::vector<StreetSegmentIdx> globalPath;



double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, 
                             const double turn_penalty){
    
    //checking if source is destination
    
    if(path.size()==0) return 0;
    
    //if source not destination
    
    double totalTurnPenalty = 0; //keeps track of time spent turning
    double totalTravelTime = 0; //keeps track of time spent on each segment
    
    for(int segment=0; segment<path.size(); segment++){
        //checking for turns
        if(segment!=(path.size()-1)){
            StreetSegmentInfo currentSegment=getStreetSegmentInfo(path[segment]);
            StreetSegmentInfo nextSegment=getStreetSegmentInfo(path[segment+1]);
            if(currentSegment.streetID != nextSegment.streetID)
                totalTurnPenalty+=turn_penalty;
        }
        
        //adding time per segment
        totalTravelTime+=findStreetSegmentTravelTime(path[segment]);
        
    }
    
    return totalTurnPenalty+totalTravelTime; //returning total time
    
}

std::vector<double> multidjikstra(IntersectionIdx initial, const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const double turn_penalty){
    std::vector<Node> multiNodes;
    int totalIntersectionSize = 2*deliveries.size()+depots.size();
    multiNodes.resize(getNumIntersections());
    
    std::vector<double> nodeBestTimes(totalIntersectionSize, -1);
    //vector to return segments in path
    std::vector<StreetSegmentIdx> allSegments;
    allSegments.clear();
    //we have both the source and destination intersections 
    IntersectionIdx sourceId = initial;
//    IntersectionIdx destinationId = intersect_ids.second;
    std::vector<IntersectionIdx> allDeliveryIntersections;
    for(int i = 0; i<deliveries.size(); i++){
        allDeliveryIntersections.push_back(deliveries[i].pickUp);
        allDeliveryIntersections.push_back(deliveries[i].dropOff);
    }
    for(int i = 0; i<depots.size(); i++){
        allDeliveryIntersections.push_back(depots[i]);
    }
    
    //return if sourceId = destinationId
//    if(sourceId==destinationId) return allSegments;
    
    //using priority queue
    //sorting priority queue depending on lowest travel time in waveElem
    std::priority_queue<waveElem, std::vector<waveElem>, minHeap> waveFront;
    waveFront.push(waveElem(sourceId, NO_EDGE, 0));

    //if nodes left to explore, loop runs
    while(!waveFront.empty()){
        
        //based on the priorty sequence removes the top node
        waveElem wave = waveFront.top();
        waveFront.pop();
        
        int currNodeId = wave.nodeId;
        
        //if the time calculated to node is less than the given best time, best time updated
        //and edge used to reach node updated 
        if (wave.travelTime<multiNodes[currNodeId].bestTime){
            multiNodes[currNodeId].bestTime = wave.travelTime;
            multiNodes[currNodeId].reachingEdge = wave.edgeId;
            std::vector<StreetSegmentIdx> outEdges = findStreetSegmentsOfIntersection(currNodeId);
            for (auto i:outEdges){
                StreetSegmentInfo nextEdge = getStreetSegmentInfo(i);
                //pushes into priority queue all other nodes reached by given node
                if(!nextEdge.oneWay || nextEdge.from == currNodeId){
                    StreetSegmentInfo currEdge;
                    if(multiNodes[currNodeId].reachingEdge!= NO_EDGE) currEdge = 
                            getStreetSegmentInfo(multiNodes[currNodeId].reachingEdge);
                    StreetSegmentIdx nextNodeId;
                    if (nextEdge.to == currNodeId)  nextNodeId = nextEdge.from;
                    else nextNodeId = nextEdge.to;
                    double nodeTT = 0;
                    if(multiNodes[currNodeId].reachingEdge!= NO_EDGE){
                        if(currEdge.streetID!=nextEdge.streetID) nodeTT = turn_penalty;
                    }
                    nodeTT+= multiNodes[currNodeId].bestTime+findStreetSegmentTravelTime(i);
                    //for each outgoing node, best time, and reachingEdge found, input into priority queue 
                    waveFront.push(waveElem(nextNodeId, i, nodeTT));
                }
            }
            
        }
        //checks if destination node reached
        
        for(auto i =0; i<allDeliveryIntersections.size(); i++){
            allSegments.clear();
            if(currNodeId == allDeliveryIntersections[i]){
//                allDeliveryIntersections[i].visited = true;
//                while (currNodeId != sourceId) {
//                    StreetSegmentIdx reachingEdge = multiNodes[currNodeId].reachingEdge;//from current node finds edge used to reach
//                    allSegments.push_back(reachingEdge);//inputs each streetID used from destination to source in a vector
//                    //from edge, finds the other node(intersction)
//                    if (getStreetSegmentInfo(reachingEdge).from==currNodeId) 
//                        currNodeId=getStreetSegmentInfo(reachingEdge).to;
//                    else currNodeId = getStreetSegmentInfo(reachingEdge).from;
//                }
//                std::reverse(allSegments.begin(), allSegments.end());//vector reversed to correct order(source to destination)
                nodeBestTimes[i] = multiNodes[currNodeId].bestTime;
            }
        }
    }
    //backtraces the path to each node if path found in above loop
//    if (pathFound) {
//        int currNodeId = destinationId;
//        while (currNodeId != sourceId) {
//            StreetSegmentIdx reachingEdge = multiNodes[currNodeId].reachingEdge;//from current node finds edge used to reach
//            allSegments.push_back(reachingEdge);//inputs each streetID used from destination to source in a vector
//            //from edge, finds the other node(intersction)
//            if (getStreetSegmentInfo(reachingEdge).from==currNodeId) 
//                currNodeId=getStreetSegmentInfo(reachingEdge).to;
//            else currNodeId = getStreetSegmentInfo(reachingEdge).from;
//        }
//        std::reverse(allSegments.begin(), allSegments.end());//vector reversed to correct order(source to destination)
//        
//    }
    

    
    return nodeBestTimes;//returns the same vector of streetIDs
        
}
//-----------------------------------------------------------------------------

// WE HAVE A VECTOR OF NODES FOR ALL INTERSECTIONS (nodes)

std::vector<StreetSegmentIdx> findPathBetweenIntersections(
                  const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids,
                  const double turn_penalty){
    globalPath.clear();
    nodes.clear();
    nodes.resize(getNumIntersections());
    //vector to return segments in path
    std::vector<StreetSegmentIdx> allSegments;
    allSegments.clear();
    //we have both the source and destination intersections 
    IntersectionIdx sourceId = intersect_ids.first;
    IntersectionIdx destinationId = intersect_ids.second;
    
    
    //return if sourceId = destinationId
    if(sourceId==destinationId) return allSegments;
    
    //using priority queue
    //sorting priority queue depending on lowest travel time in waveElem
    std::priority_queue<waveElem, std::vector<waveElem>, minHeap> waveFront;
    waveFront.push(waveElem(sourceId, NO_EDGE, 0));
    bool pathFound;
    pathFound= false;

    //if nodes left to explore, loop runs
    while(!waveFront.empty()){
        
        //based on the priorty sequence removes the top node
        waveElem wave = waveFront.top();
        waveFront.pop();
        
        int currNodeId = wave.nodeId;
        
        //if the time calculated to node is less than the given best time, best time updated
        //and edge used to reach node updated 
        if (wave.travelTime<nodes[currNodeId].bestTime){
            nodes[currNodeId].bestTime = wave.travelTime;
            nodes[currNodeId].reachingEdge = wave.edgeId;
            std::vector<StreetSegmentIdx> outEdges = findStreetSegmentsOfIntersection(currNodeId);
            for (auto i:outEdges){
                StreetSegmentInfo nextEdge = getStreetSegmentInfo(i);
                //pushes into priority queue all other nodes reached by given node
                if(!nextEdge.oneWay || nextEdge.from == currNodeId){
                    StreetSegmentInfo currEdge;
                    if(nodes[currNodeId].reachingEdge!= NO_EDGE) currEdge = getStreetSegmentInfo(nodes[currNodeId].reachingEdge);
                    StreetSegmentIdx nextNodeId;
                    if (nextEdge.to == currNodeId)  nextNodeId = nextEdge.from;
                    else nextNodeId = nextEdge.to;
                    double nodeTT = 0;
                    if(nodes[currNodeId].reachingEdge!= NO_EDGE){
                        if(currEdge.streetID!=nextEdge.streetID) nodeTT = turn_penalty;
                    }
                    nodeTT+= nodes[currNodeId].bestTime+findStreetSegmentTravelTime(i);
                    //for each outgoing node, best time, and reachingEdge found, input into priority queue 
                    waveFront.push(waveElem(nextNodeId, i, nodeTT));
                }
            }
            
        }
        //checks if destination node reached 
        if(currNodeId == destinationId){
            pathFound = true;
            break;
        }
    }
    //backtraces the path to each node if path found in above loop
    if (pathFound) {
        int currNodeId = destinationId;
        while (currNodeId != sourceId) {
            StreetSegmentIdx reachingEdge = nodes[currNodeId].reachingEdge;//from current node finds edge used to reach
            allSegments.push_back(reachingEdge);//inputs each streetID used from destination to source in a vector
            //from edge, finds the other node(intersction)
            if (getStreetSegmentInfo(reachingEdge).from==currNodeId) 
                currNodeId=getStreetSegmentInfo(reachingEdge).to;
            else currNodeId = getStreetSegmentInfo(reachingEdge).from;
        }
        std::reverse(allSegments.begin(), allSegments.end());//vector reversed to correct order(source to destination)
        
    }
    

    
    globalPath.resize(allSegments.size());//uses a global vector to store path
    globalPath = allSegments;
    return allSegments;//returns the same vector of streetIDs
        
}


//REMEMBER TO SET THE NODES VISITED TO FALSE AGAIN BEFORE EXITING FUNCTION