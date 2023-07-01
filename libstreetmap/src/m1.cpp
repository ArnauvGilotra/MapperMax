/* 
 * Copyright 2023 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "front_end.h"

//libraries being added
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <thread>
#include <chrono>
#include "globals.h"

//global variables & Data Structures 
std::vector<std::string> POIType;
std::vector<std::string> POIName;
std::vector<ezgl::point2d> POILoc;
std::vector<std::vector<std::vector<ezgl::point2d>>> featurePoints;
std::vector<std::vector<LatLon>> curveStreetSegments;
std::vector<std::vector<StreetSegmentIdx>> intersectionStreetSegments; 
std::vector<std::vector<IntersectionIdx>> adjacentIntersection;
std::vector<std::vector<IntersectionIdx>> streetIntersection;
std::vector<std::vector<std::vector<StreetIdx>>> sameFirstTwoLettersStreets; 
std::vector<std::vector<StreetIdx>> sameFirstLetterStreets;
std::vector<std::vector<std::vector<std::string>>> abc; 
std::vector<std::vector<std::vector<IntersectionIdx>>> sameFirstTwoLettersIntersections; 
std::vector<std::vector<IntersectionIdx>> sameFirstLetterIntersection;
std::vector<std::vector<std::vector<std::string>>> abcI; 
std::vector<std::vector<double>> segmentLength;
std::vector<double> streetLength; 
std::vector<double> segmentTime;
std::unordered_map<OSMID,std::vector<std::pair<std::string, std::string>>> osmidMap;
//std::unordered_map<OSMID,std::vector<std::pair<std::string, std::string>>> osmwayidMap;
std::vector<const OSMNode*> osm_city_names;
std::unordered_map<OSMID, const OSMNode*> osmNodePointers;
std::unordered_map<OSMID, const OSMWay*> osmWayPointers;
std::unordered_map<OSMID, const OSMRelation*> osmRelPointers;
std::vector<ezgl::point2d> subwayStops;
std::vector<std::vector<ezgl::point2d>> subwayWay;
std::vector<Node> nodes;
bool loadOSM; //1152 8670

//bool isFirstLoad = TRUE;

//function declarations
void loadOSMdatabase(std::string str);
int keyFunction(char input);
int featKey(FeatureType type);
void preloadIntersections();
void preloadOSMNodes(int num);
void preloadOSMWays(int numWays);
void preloadOSMRels(int numRels);
void preloadSegmentLengthTime();
void preloadStreetLengthIntersection();
void preloadPartialNames();
void preloadFeaturesxy();
void preloadPOIxy();
void preLoadNodes();

// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.
void loadOSMdatabase(std::string str){
    loadOSM= loadOSMDatabaseBIN(str);
}

bool loadMap(std::string map_streets_database_filename) {
    
    bool load_successful = false; //Indicates whether the map has loaded
    fromID = -1;
    toID = -1;
    showPOIAll = FALSE;
    showPOIedu = FALSE;
    showPOIemer = FALSE;
    showPOIent = FALSE;
    showPOIfood = FALSE;
    //modifying argument(string) to replace "street.bin" with "osm.bin", to facilitate OSM map loading
    std::string str = map_streets_database_filename;
    std::string str_replacement = "osm.bin";
    str.replace(str.end()-11,str.end(),str_replacement);
    
    loadOSM = false;
    std::thread th1(loadOSMdatabase,str);//loads OSM map
       
    //function returns if OSM unsuccessful
    
    load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);
    
    if(!load_successful) return false;//returns function if load unsuccessful
    
    // to allocate only the amount of memory that is needed
    globalPath.clear();
    intersectionStreetSegments.resize(getNumIntersections());
    adjacentIntersection.resize(getNumIntersections()); 
    streetIntersection.resize(getNumStreets());
    streetLength.resize(getNumStreets());
    curveStreetSegments.resize(getNumStreetSegments());
    

    setMinMaxLatLon();
    // PRE LOADING DATA FOR STREET SEGMENTS OF AN INTERSECTION AND ADJACENT INTERSECTIONS OF AN INTERSECTION
    
    preloadIntersections();
    
    //-------PRELOADING FOR findStreetSegmentLength and findStreetSegmentTravelTime---------------------------------------------------------------
    
    preloadSegmentLengthTime();
    
    //-----------PRELOADING FOR findStreetLength and findIntersectionsOfStreet-----------------------------------    
    
    preloadStreetLengthIntersection();
    
    //----------------PRELOADING STARTING FOR findStreetIdsFromPartialStreetName----------------//

    preloadPartialNames();
    
    ///----------------PRELOADING STARTING FOR Each feature's x and y coordinates----------------//
    
    preloadFeaturesxy();
    
    //----------------PRELOADING STARTING FOR Each POI's x and y coordinates----------------//

    preloadPOIxy();
    
//    preLoadNodes();

    th1.join();
    if(!loadOSM) return false;
    
    //----------------------PRELOADING STARTING FOR getOSMNodeTagValue-------------------------//

    int num = getNumberOfNodes();
    preloadOSMNodes(num);
    
    int numWays = getNumberOfWays();
    preloadOSMWays(numWays);
    
    int numRelations = getNumberOfRelations();
    preloadOSMRels(numRelations);
    

    
    //preloading for functions in m2
    city_names();
    curvexyStreet_populate();
    

    //-----------END ALL PRELOADING------------//

    std::cout << "loadMap: " << map_streets_database_filename << std::endl;
    return true;
}
// Function to generate a key given a char input. 
// 'a' to 'z' char inputs are given 0-25 
// '0' to '9' are given 26 to 35
// rest of the possible chars are given -1 which means invalid
int keyFunction(char input) {
    if (isdigit(input)) {
        return input - 22;
    } else if (isalpha(input)) {
        return input - 'a';
    } else {
        return -1;
    }
}

int featKey(FeatureType type){
    if (type == UNKNOWN){
        return 0;
    } else if (type == LAKE){
        return 1;
    } else if (type == BEACH){
        return 2;
    } else if (type == RIVER){
        return 3;
    } else if (type == ISLAND){
        return 4;
    } else if (type == GLACIER){
        return 5;
    } else if (type == GREENSPACE){
        return 6;
    } else if (type == GOLFCOURSE){
        return 7;
    } else if (type == PARK){
        return 8;
    } else if (type == STREAM){
        return 9;
    } else if (type == BUILDING){
        return 10;
    }
    return 0;
}

void preloadIntersections(){
    for(int intersection = 0; intersection < getNumIntersections(); ++intersection) { //looping through every intersection
        for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) { //finding every intersection's street segments
            int ss_id = getIntersectionStreetSegment(intersection, i);
            intersectionStreetSegments[intersection].push_back(ss_id); //using vector of vector
        
            StreetSegmentInfo sstruct = getStreetSegmentInfo(ss_id);//using StreetsDatabaseAPI to retrieve information about street segment
            if (sstruct.oneWay){ 
                if(sstruct.from == intersection){//checks if other intersection is reachable from current intersection if segment one way
                    adjacentIntersection[intersection].push_back(sstruct.to);
                }
            } else {//if not one way, populates vector adjacentIntersection with index of intersection with the other intersection of given segment
                if(sstruct.from == intersection){
                    adjacentIntersection[intersection].push_back(sstruct.to);
                }
                else{
                    adjacentIntersection[intersection].push_back(sstruct.from);
                }
            }
        }
        //removes duplicates from vector pointed by adjacentIntersection for an intersection
        std::sort(adjacentIntersection[intersection].begin(),adjacentIntersection[intersection].end());
        std::vector<IntersectionIdx>::iterator a;
        a = std::unique(adjacentIntersection[intersection].begin(),adjacentIntersection[intersection].end());
        adjacentIntersection[intersection].resize(std::distance(adjacentIntersection[intersection].begin(),a));
    }
}

void preloadOSMNodes(int num){
    for(int i = 0; i<num;i++){ //loop through all node indexes, to find OSMNode pointer for each index 
        
        
        const  OSMNode* e = getNodeByIndex(i);
        osmNodePointers[e->id()] = e;
        int tagCount = getTagCount(e);
        for (int a = 0; a<tagCount; a++){//populate unordered map (osmidMap) with vector of tag pairs
            std::pair<std::string, std::string> tagPair = getTagPair(e, a);
            //std::cout<<"my name is dev\n";
            osmidMap[e->id()].push_back(tagPair);//each key stores all the attribute pairs
            
            if (tagPair.first == "place" && tagPair.second == "city") {
                //std::cout<<"my name is dev\n";
                osm_city_names.push_back(e);
            }
        }
        osmidMap[e->id()].resize(tagCount);//each vector is resized to total number of tag pairs
    }
}
void preloadOSMWays(int numWays){
    for(int currWay = 0; currWay<numWays;currWay++){ //loop through all node indexes, to find OSMNode pointer for each index 
        
        
        const  OSMWay* way_ptr = getWayByIndex(currWay);
        osmWayPointers[way_ptr->id()] = way_ptr;
        int tagCountWay = getTagCount(way_ptr);
        for (int a = 0; a<tagCountWay; a++){//populate unordered map (osmidMap) with vector of tag pairs
            std::pair<std::string, std::string> tagPairWay = getTagPair(way_ptr, a);
            osmidMap[way_ptr->id()].push_back(tagPairWay);//each key stores all the attribute pairs
        }
        osmidMap[way_ptr->id()].resize(tagCountWay);//each vector is resized to total number of tag pairs
    }
}
void preloadOSMRels(int numRels){
    for(int currRel = 0; currRel<numRels; currRel++){
        
        const OSMRelation* rel_ptr = getRelationByIndex(currRel);
        osmRelPointers[rel_ptr->id()] = rel_ptr;
        int tagCountRel = getTagCount(rel_ptr);
        for (int tag = 0; tag<tagCountRel; tag++){
            std::pair<std::string, std::string> tagPairRel = getTagPair(rel_ptr, tag);
            osmidMap[rel_ptr->id()].push_back(tagPairRel);

        }
        osmidMap[rel_ptr->id()].resize(tagCountRel);
        
        
        if(getOSMNodeTagValue(rel_ptr->id(), "route")=="subway"){
            //std::cout<<"name:"<<getOSMNodeTagValue(rel_ptr->id(), "name")<<"\n";
            std::vector<TypedOSMID>members = getRelationMembers(rel_ptr);
            std::vector<std::string>roles = getRelationMemberRoles(rel_ptr);
            int numMembers = members.size();
            for(int mem = 0; mem< numMembers; mem++){
                if (members[mem].type() == TypedOSMID::Way){
                    if(roles[mem]!="platform"){
                        const OSMWay* way = osmWayPointers[members[mem]];
                        std::vector<OSMID> membersWay = getWayMembers(way);
                        int numMembersWay = membersWay.size();
                        std::vector<ezgl::point2d> allNodes;
                        for(int memw = 0; memw<numMembersWay; memw++){

                                const OSMNode* node =  osmNodePointers[membersWay[memw]];
                                LatLon nodePos = getNodeCoords(node);
                                ezgl::point2d nodeXY;
                                nodeXY.x = x_from_lon(nodePos.longitude());
                                nodeXY.y = y_from_lat(nodePos.latitude());
                                allNodes.push_back(nodeXY);

                        }
                        subwayWay.push_back(allNodes); 
                    }
                   
                } 
                    else if (members[mem].type() == TypedOSMID::Node){
                    const OSMNode* node =  osmNodePointers[members[mem]];
                    LatLon nodePos = getNodeCoords(node);
                            ezgl::point2d nodeXY;
                            nodeXY.x = x_from_lon(nodePos.longitude());
                            nodeXY.y = y_from_lat(nodePos.latitude());
                            subwayStops.push_back(nodeXY);
                      } 
            
            }

        }
        
    }
}

void preloadSegmentLengthTime(){
    segmentLength.resize(getNumStreetSegments()); 
    
    for(int segment=0; segment<getNumStreetSegments();segment++){//looping through all street segment indexes
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segment);
        IntersectionIdx startingPoint = segmentInfo.from;
        IntersectionIdx endingPoint = segmentInfo.to;
        int numExtra = segmentInfo.numCurvePoints;

        //defining a vector which has LatLons of all points of a segment(to, from and curve points)
        curveStreetSegments[segment].push_back(getIntersectionPosition(startingPoint));
        //lat lon of curve points in segment stored in vector
        for(int i=0; i<numExtra; i++) curveStreetSegments[segment].push_back(getStreetSegmentCurvePoint(segment, i));

        curveStreetSegments[segment].push_back(getIntersectionPosition(endingPoint));
        double totalLength = 0;
        //loop to store and calculate the total length between all points in a segment
        for(int seg=0; seg<numExtra+1;seg++){
            totalLength += findDistanceBetweenTwoPoints(curveStreetSegments[segment][seg],curveStreetSegments[segment][seg+1]);
        }
        segmentLength[segment].push_back(totalLength);//populating segmentLength with value of total length
        
        float segmentSpeed = segmentInfo.speedLimit;
        segmentTime.push_back(totalLength/segmentSpeed);//populating vector segmentTime(stores time for each segment) in same loop
        
    }
}

void preloadStreetLengthIntersection(){
    std::fill(streetLength.begin(),streetLength.end(),0);//vector streetLength initialized to 0 for all indexes, so can be added to itself
    for (int segment=0; segment<getNumStreetSegments();++segment){//loop through all segment ids
       StreetSegmentInfo sstruct = getStreetSegmentInfo(segment);
       streetIntersection[sstruct.streetID].push_back(sstruct.to);//vector streetIntersection populated at each index (given by street id of each segment) 
       streetIntersection[sstruct.streetID].push_back(sstruct.from);//with to and from intersection ids of segment(vector of IntersectionIdx)
       streetLength[sstruct.streetID]=streetLength[sstruct.streetID]+segmentLength[segment][0];//same loop used to incrementally populate streetLength 
    }                                                                                          //by using street id from segment as index and adding each segment length(using global vector) belonging to the same street                                                
    
    for (int street = 0; street<getNumStreets();++street){//loop to check duplicates in  vector
       std::sort(streetIntersection[street].begin(),streetIntersection[street].end());//sorts vector
       std::vector<IntersectionIdx>::iterator b;
       b = std::unique(streetIntersection[street].begin(),streetIntersection[street].end());//removes duplicate value, returns iterator to last meaningful value
       streetIntersection[street].resize(std::distance(streetIntersection[street].begin(),b));//removes empty spaces from vector occupied by duplicate values
    }
    return;
}

void preloadPartialNames(){
    sameFirstTwoLettersStreets.clear();
    sameFirstTwoLettersStreets.resize(36);
    sameFirstLetterStreets.clear();
    sameFirstLetterStreets.resize(36);
    abc.clear();
    abc.resize(36);
    int numStreets = getNumStreets();

    for (int i = 0; i < numStreets; i++)
    {
        std::string name = getStreetName(i);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower); //make name lower case
        std::string noSpace;
        std::remove_copy(name.begin(), name.end(), std::back_inserter(noSpace), ' '); //remove all spaces

        int key1 = keyFunction(name[0]); //get key from function for the 1st character of the word
        int key2 = keyFunction(name[1]); //get key from function for the 2nd character of the word
        if(key1 == -1) continue; //Proceed in the loop ONLY if the 1st character is an alphabet or digit
        sameFirstLetterStreets[key1].push_back(i); //push all ids of the same first character
        if(key2 == -1) continue; //Proceed in the loop ONLY if the 2nd character is an alphabet or digit
        if (sameFirstTwoLettersStreets[key1].size() != 36) sameFirstTwoLettersStreets[key1].resize(36); //resize the vector is not already 36
        if (abc[key1].size() != 36) abc[key1].resize(36); //resize the vector is not already 36
        sameFirstTwoLettersStreets[key1][key2].push_back(i); //push all ids of the same first 2 character
        abc[key1][key2].push_back(noSpace); //push all names of the same first 2 character
    }

    sameFirstTwoLettersIntersections.clear();
    sameFirstTwoLettersIntersections.resize(36);
    sameFirstLetterIntersection.clear();
    sameFirstLetterIntersection.resize(36);
    abcI.clear();
    abcI.resize(36);
    int numIntersections = getNumIntersections();

    for (int i = 0; i < numIntersections; i++)
    {
        std::string name = getIntersectionName(i);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower); //make name lower case
        std::string noSpace;
        std::remove_copy(name.begin(), name.end(), std::back_inserter(noSpace), ' '); //remove all spaces

        int key1 = keyFunction(name[0]); //get key from function for the 1st character of the word
        int key2 = keyFunction(name[1]); //get key from function for the 2nd character of the word
        if(key1 == -1) continue; //Proceed in the loop ONLY if the 1st character is an alphabet or digit
        sameFirstLetterIntersection[key1].push_back(i); //push all ids of the same first character
        if(key2 == -1) continue; //Proceed in the loop ONLY if the 2nd character is an alphabet or digit
        if (sameFirstTwoLettersIntersections[key1].size() != 36) sameFirstTwoLettersIntersections[key1].resize(36); //resize the vector is not already 36
        if (abcI[key1].size() != 36) abcI[key1].resize(36); //resize the vector is not already 36
        sameFirstTwoLettersIntersections[key1][key2].push_back(i); //push all ids of the same first 2 character
        abcI[key1][key2].push_back(noSpace); //push all names of the same first 2 character
    }
    
}

void preloadFeaturesxy(){
    int numFeature = getNumFeatures();
    featurePoints.resize(11);

    for (int i = 0; i < numFeature; i++)
    {
        FeatureType type = getFeatureType(i);
        int key = featKey(type);
        std::vector<ezgl::point2d> points;
        for (int j = 0; j < getNumFeaturePoints(i); j++)
        {
            ezgl::point2d point;
            LatLon pt = getFeaturePoint(i, j);
            point.x = x_from_lon(pt.longitude());
            point.y = y_from_lat(pt.latitude());
            points.push_back(point);
        }
        featurePoints[key].push_back(points);
    }
}

//void preLoadNodes(){
//    int numIntersections = getNumIntersections();
//    nodes.resize(numIntersections);
//    for(int inter=0; inter<numIntersections; inter++){
//        //first loading the outgoing edge
//        std::vector<StreetSegmentIdx> outgoingSegments;
//        outgoingSegments.resize(findStreetSegmentsOfIntersection(inter).size());
//        outgoingSegments= findStreetSegmentsOfIntersection(inter);
//        for(int seg=0; seg<outgoingSegments.size();seg++){
//            StreetSegmentInfo segInfo = getStreetSegmentInfo(outgoingSegments[seg]);
//            if(segInfo.oneWay && segInfo.to==inter) outgoingSegments.erase(outgoingSegments.begin()+seg);
//            //removing segments that come into the intersection if one way
//        }
//        nodes[inter].outgoingEdges=outgoingSegments;
//        nodes[inter].visited=false;
//        nodes[inter].bestTime=INFINITY;
//    }
//}



void preloadPOIxy(){
    int numPOI = getNumPointsOfInterest();
    POIType.resize(numPOI);
    POIName.resize(numPOI);
    POILoc.resize(numPOI);

    for (int i = 0; i < numPOI; i++)
    {
        ezgl::point2d point;
        LatLon pt = getPOIPosition(i);
        point.x = x_from_lon(pt.longitude());
        point.y = y_from_lat(pt.latitude());
        POILoc[i] = point;
        POIType[i] = getPOIType(i);
        POIName[i] = getPOIName(i);
    }
}

void closeMap() {
//    exitDir();
    //Clean-up your map related data structures here
    closeStreetDatabase();
    closeOSMDatabase();
    
    //Clearing all the datastructures
    POIName.clear();
    POIType.clear();
    POILoc.clear();
    globalPath.clear();
    featurePoints.clear();
    osm_city_names.clear();
    cityInfo.clear();
    intersections.clear();
    intersections_xy.clear();
    curvexyStreetSegments.clear();
    curveStreetSegments.clear();
    osmNodePointers.clear();
    osmWayPointers.clear();
    osmRelPointers.clear();
    subwayStops.clear();
    subwayWay.clear();
    nodes.clear();
    intersectionStreetSegments.clear(); 
    adjacentIntersection.clear();
    streetIntersection.clear();
    sameFirstTwoLettersStreets.clear(); 
    sameFirstLetterStreets.clear();
    abc.clear(); 
    sameFirstTwoLettersIntersections.clear(); 
    sameFirstLetterIntersection.clear();
    abcI.clear();
    segmentLength.clear();
    streetLength.clear(); 
    segmentTime.clear();
    osmidMap.clear();
    from.clear();
    to.clear();
    streetIntersections.clear();
    stopInfo.clear();
}

//-------------------------FUNCTIONALITY FUNCTIONS START BELOW--------------------------------------------//

// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double finalDistance = 0;
    
    // getting lat lon and converting from radians to degrees
    double latP1 = point_1.latitude()*kDegreeToRadian;
    double lonP1 = point_1.longitude()*kDegreeToRadian;
    
    double latP2 = point_2.latitude()*kDegreeToRadian;
    double lonP2 = point_2.longitude()*kDegreeToRadian;
    
    double avgLat = (latP1+latP2)/2;
    
    // converting to x-y
    double xCoorP1 = kEarthRadiusInMeters*lonP1*cos(avgLat);
    double yCoorP1 = kEarthRadiusInMeters*latP1;
    
    double xCoorP2 = kEarthRadiusInMeters*lonP2*cos(avgLat);
    double yCoorP2 = kEarthRadiusInMeters*latP2;
    
    finalDistance = sqrt(pow(yCoorP2-yCoorP1,2)+pow(xCoorP2-xCoorP1,2));
    
    return finalDistance;
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    return segmentLength[street_segment_id][0]; //Preloaded in Load Map
}

// Returns the travel time to drive from one end of a street segment
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    return segmentTime[street_segment_id]; //Preloaded in Load Map
}

// Returns all intersections reachable by traveling down one street segment
// from the given intersection (hint: you can't travel the wrong way on a
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Corner case: cul-de-sacs can connect an intersection to itself
// (from and to intersection on  street segment are the same). In that case
// include the intersection in the returned vector (no special handling needed).
// Speed Requirement --> high
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){
    return adjacentIntersection[intersection_id]; //Preloaded in Load Map
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to
// the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    int numPoints = getNumIntersections(); //get all the possible intersections
    double currentDist = 0.0; 
    IntersectionIdx index = 0; 
    for (int i = 0; i < numPoints; i++){
            LatLon pos = getIntersectionPosition(i); //get the coord for the iterating i intersection
            double newDist = findDistanceBetweenTwoPoints(my_position, pos); //find the distance between the given and the intersection point
            if(currentDist == 0.0){ //when current distance is 0.0 (the first iteration)
                currentDist = newDist; 
            } 
            if(newDist < currentDist){ 
                //If the newly calculated distance in this iteration is lower than the one saved from the 
                //previous iteration then this is our newest low
                currentDist = newDist;
                index = i;
            }
    }
    return index;
}

// Returns the street segments that connect to the given intersection
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return intersectionStreetSegments[intersection_id]; //Preloaded in Load Map
}

// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    return streetIntersection[street_id]; //Preloaded in Load Map
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(StreetIdx street_id1, StreetIdx street_id2){
    std::vector<IntersectionIdx> intersectionsOfStreets;
    std::set_intersection(streetIntersection[street_id1].begin(), streetIntersection[street_id1].end(),//finds common values between streetIntersections of
                            streetIntersection[street_id2].begin(),streetIntersection[street_id2].end(),//both streetIds, pushes them into vector intersections
                            back_inserter(intersectionsOfStreets));
    return intersectionsOfStreets;//vector of common elements(Intersection ids) returned 
}

// Returns all street ids corresponding to street names that start with the
// given prefix
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty
// (length 0) string, but your program must not crash if street_prefix is a
// length 0 string.
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::string prefixNoSpaces; 
    std::vector<StreetIdx> toReturn; 
    if(street_prefix == ""){
        return toReturn; //Empty street prefix hence we return an empty vector
    }
    std::remove_copy(
        street_prefix.begin(), 
        street_prefix.end(), 
        std::back_inserter(prefixNoSpaces), 
        ' '
    );
    std::transform(prefixNoSpaces.begin(), prefixNoSpaces.end(), prefixNoSpaces.begin(), ::tolower);
    int numCheckChars = prefixNoSpaces.size();
    int key1 = keyFunction(prefixNoSpaces[0]); //Get the assigned key for the first character of the prefix
    int key2 = keyFunction(prefixNoSpaces[1]); //Get the assigned key for the second character of the prefix
    if (key1 != -1 && prefixNoSpaces[1] == '\0'){ //A single character input
        return sameFirstLetterStreets[key1]; //return the vector with the Idx of all street with the same initial
    }
    if (key1 != -1 && key2 != -1){ //first two characters are ONLY an alphabet or digit
        for (int i = 0; i < sameFirstTwoLettersStreets[key1][key2].size(); i++){ 
            //filter through the remaining St IDs by matching the prefix string with the street name
            //abc vector of vector of vector<strings> holds the name of street in the same index as where the
            //sameFirstTwoLettersStreet holds the matching ID. So the the ID and name of one street have the same Idx
            std::string stNameNoSpaces = abc[key1][key2][i]; 
            if(prefixNoSpaces.compare(0, numCheckChars, stNameNoSpaces, 0, numCheckChars) == 0){
                toReturn.push_back(sameFirstTwoLettersStreets[key1][key2][i]); //add to the retuning vector
            }
        }
    }   
    return toReturn;
}

// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id){
    return streetLength[street_id]; //Preloaded in Load Map
}

// Returns the nearest point of interest of the given type (e.g. "restaurant")
// to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    int numPoints = getNumPointsOfInterest(); //get the number of POI on the map
    double currentDist = 0.0;
    POIIdx index = 0;
    std::string type;
    for (int i = 0; i < numPoints; i++){
        type = getPOIType(i);
        if(type==POItype){
            LatLon pos = getPOIPosition(i); //get the coord for the iterating i intersection
            double newDist = findDistanceBetweenTwoPoints(my_position, pos); //find the distance between the given point and the POI point
            if(currentDist == 0.0){ //when current distance is 0.0 (the first iteration)
                currentDist = newDist;
            } 
            if(newDist < currentDist){
                //If the newly calculated distance in this iteration is lower than the one saved from the 
                //previous iteration then this is our newest low
                currentDist = newDist;
                index = i;
            }
        }
    }
    return index;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id){
    double xdiff, ydiff;
    double area = 0;
    int numPoints = getNumFeaturePoints(feature_id); // get the number of points which define the shape of the feature

    if ( getFeaturePoint(feature_id, 0).longitude() != getFeaturePoint(feature_id, numPoints - 1).longitude() || 
        getFeaturePoint(feature_id, 0).latitude() != getFeaturePoint(feature_id, numPoints - 1).latitude()
    ) return area; // if the feature is not a closed shape i.e the first and last aren't the same

    double factorEarthDegree = kEarthRadiusInMeters * kDegreeToRadian;

    for (int i = 1; i < numPoints; i++)
    {
        double x0 = getFeaturePoint(feature_id, i - 1 ).longitude(); // get x0 coords which is 1 behind the iteration
        double x1 = getFeaturePoint(feature_id, i).longitude(); // get x1 coords
        double y0 = getFeaturePoint(feature_id, i - 1 ).latitude(); // get y0 coords which is 1 behind the iteration
        double y1 = getFeaturePoint(feature_id, i).latitude(); // get y1 coords
        //below calculation are in line with the Trapezoidal method of Polygon area calculation method 
        double cosTheta = cos( kDegreeToRadian * (y0 + y1) /2);
        xdiff = factorEarthDegree * cosTheta *  (x1 + x0); 
        ydiff = factorEarthDegree * (y1 - y0);
        area += ydiff * xdiff /2.0;
    }
    return fabs(area);
}

// Return the value associated with this key on the specified OSMNode.
// If this OSMNode does not exist in the current map, or the specified key is
// not set on the specified OSMNode, return an empty string.
// Speed Requirement --> high
// uses global variable osmidMap (unordered map of vector of pairs) to access pre-loaded information
std::string getOSMNodeTagValue (OSMID OSMid, std::string key){
    int num = osmidMap[OSMid].size();
    for (int i = 0; i<num;i++){//searches vector of pairs for argument key (stored in first part of pair)
	    if(osmidMap[OSMid][i].first == key){//if key found, returns the value (stored in second part of pair)
		    return osmidMap[OSMid][i].second;
	    }
    }return "";//if not found, returns an empty string
}