/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   globals.h
 * Author: guptave2
 *
 * Created on March 2, 2023, 8:36 PM
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "m1.h"
#include "m4.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "feature.h"
#include "search_feature.h"


//libraries being added
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

//CONSTANTS
#define NO_EDGE -1
#define START_MARKER 2
#define END_MARKER 1
#define INTER_MARKER 0



struct Node{
    //intersection id is represented by index of vector of Node
    bool visited = false;
    StreetSegmentIdx reachingEdge = NO_EDGE;
    double bestTime = INFINITY;
};

struct waveElem{
    IntersectionIdx nodeId;
    StreetSegmentIdx edgeId;
    double travelTime; //TOTAL travel time FROM SOURCE to reach node
    waveElem(int n, int e, double tt) {nodeId=n; //constructor
                                        edgeId=e; 
                                            travelTime=tt;}  
};

// so that the top value is always the smallest travel time one
struct minHeap {
    bool operator()(const waveElem& lhs, const waveElem& rhs) const {
        return lhs.travelTime > rhs.travelTime;
    }
};

struct m4Node{
    //intersection id is represented by index of vector of Node
    bool dropoffAvailable = false;
    IntersectionIdx pick;
    IntersectionIdx drop;
    LatLon pickLL;
    LatLon dropLL;
    bool delCompleted;
    //constructor for 
    m4Node(bool DA, IntersectionIdx p, IntersectionIdx d) {dropoffAvailable=DA;
                                        pick=p; 
                                            drop=d;
                                                pickLL=getIntersectionPosition(p);
                                                dropLL=getIntersectionPosition(d);                               
                                                    delCompleted=false;
}
    
};


struct cityNamesLatLon{
    std:: string cityName;
    LatLon cityPosLL;
    ezgl::point2d cityPosXY;
}; 

struct intersectionInfo{
    LatLon position;
    std::string name;
};

struct Inter_data{
    ezgl::point2d xy_loc;
    std:: string name;
    bool highlight = false;
    int markerType = INTER_MARKER;
};

struct segmentData{
    std::vector<ezgl::point2d> xy_points;
    ezgl::point2d center;
    double angle;
    double distance;
    bool directionChanged;
};

struct IntersectionIndex{
    IntersectionIdx intersectionID;
    int index;
    IntersectionIndex(IntersectionIdx d, int i){
        intersectionID = d;
        index = i;
    }
};

extern std::vector<ezgl::point2d> subwayStops;
extern std::vector<std::vector<ezgl::point2d>> subwayWay;
extern std::unordered_map<OSMID, const OSMNode*> osmNodePointers;
extern std::unordered_map<OSMID, const OSMWay*> osmWayPointers;
extern std::unordered_map<OSMID, const OSMRelation*> osmRelPointers;
extern std::vector<std::string> POIType;
extern std::vector<std::string> POIName;
extern std::vector<ezgl::point2d> POILoc;       
extern std::vector<std::vector<std::vector<ezgl::point2d>>> featurePoints;
extern std::unordered_map<OSMID,std::vector<std::pair<std::string, std::string>>> osmwayidMap;
extern std::vector<const OSMNode*> osm_city_names;
extern std::vector<cityNamesLatLon> cityInfo;
extern ezgl::canvas* canv;
extern std::vector<segmentData> curvexyStreetSegments;
extern std::vector<std::vector<LatLon>> curveStreetSegments;
extern std::vector<std::vector<StreetSegmentIdx>> intersectionStreetSegments; 
extern std::vector<std::vector<IntersectionIdx>> adjacentIntersection;
extern std::vector<std::vector<IntersectionIdx>> streetIntersection;
extern std::vector<std::vector<std::vector<StreetIdx>>> sameFirstTwoLettersStreets; 
extern std::vector<std::vector<StreetIdx>> sameFirstLetterStreets;
extern std::vector<std::vector<std::vector<std::string>>> abc; 
extern std::vector<std::vector<std::vector<IntersectionIdx>>> sameFirstTwoLettersIntersections; 
extern std::vector<std::vector<IntersectionIdx>> sameFirstLetterIntersection;
extern std::vector<std::vector<std::vector<std::string>>> abcI; 
extern std::vector<std::vector<double>> segmentLength;
extern std::vector<double> streetLength; 
extern std::vector<double> segmentTime;
extern std::unordered_map<OSMID,std::vector<std::pair<std::string, std::string>>> osmidMap;
extern std::vector<intersectionInfo> intersections;
extern std::vector<Inter_data> intersections_xy;
extern std::vector<Node> nodes;
extern std::vector<StreetSegmentIdx> globalPath;
extern std::vector<m4Node> stopInfo;
extern GObject *searchFromBar;
extern GObject *searchToBar;
extern GObject *bottomLabel;
extern GObject *comboBox;
extern GObject *f;
extern GObject *f1;
extern GObject *f2;
extern GObject *f3;
extern GObject *f0;
extern GObject *t;
extern GObject *t1;
extern GObject *t2;
extern GObject *t3;
extern GObject *t0;
extern GObject *info;
extern double AVG_LAT_RAD;
extern double max_lat;
extern double min_lat;
extern double max_lon;
extern double min_lon;
extern bool CLEAR_SCREEN;
extern bool streetIntersectionsPresent;
extern int fromID;
extern int toID;
extern gulong handlerFrom;
extern gulong handlerTo;
extern std::vector<StreetIdx> from;
extern std::vector<StreetIdx> to;
extern std::vector<IntersectionIdx> streetIntersections;
extern bool isInitial;
extern GObject *info;
extern GObject *food;
extern GObject *emer;
extern GObject *all;
extern GObject *edu;
extern GObject *ent;
extern bool showPOIAll;
extern bool showPOIent;
extern bool showPOIedu;
extern bool showPOIemer;
extern bool showPOIfood;
extern GObject *listbox;
extern GObject *route;
extern GObject *dir;
extern GObject *tutbox;
extern GObject *tutview;
extern GObject *tutbtn;
extern GObject *help;
extern GObject *find;
extern GObject *hidepane;
extern GObject *exitdir;
extern GObject *showdirpane;

extern bool searchFirstIntersection;
extern bool searchSecondIntersection;

extern int firstSearchHighlight;
extern int secondSearchHighlight;

extern bool highlight_present;

extern int id_last_highlight;

extern bool streetIntersectionsPresent;

std::vector<double> multidjikstra(IntersectionIdx initial, const std::vector<DeliveryInf>& deliveries,
                            const std::vector<IntersectionIdx>& depots,
                            const double turn_penalty);

#endif /* GLOBALS_H */

