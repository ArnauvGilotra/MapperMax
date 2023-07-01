#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m2.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "globals.h"
#include "front_end.h"



//libraries being added
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <iostream>
#include <sstream>


const ezgl::color pathColor(30, 129, 176);
const ezgl::color motorwayColor(246, 207, 101);
const double motorAdjWidth = 30;
const double motorInitialWidth = 3;
const double motorZoomFactor = 1000;
const double primtAdjWidth = 25;
const double primtInitialWidth = 1;
const double primtZoomFactor = 70;
const double secAdjWidth = 20;
const double secInitialWidth = 1;
const double secZoomFactor = 45;
const double terAdjWidth = 15;
const double terInitialWidth = 1;
const double terZoomFactor = 20;
const double resAdjWidth = 15;
const double resInitialWidth = 1;
const double resZoomFactor = 15;
const double othAdjWidth = 12;
const double othInitialWidth = 1;
const double othZoomFactor = 10;
const int RIGHT = 0;
const int LEFT = 1;
const int SLIDE_RIGHT = 2;
const int SLIDE_LEFT = 3;
const int SHARP_RIGHT = 4;
const int SHARP_LEFT = 5;
const int STRAIGHT = 6;
const int NORTH = 0;
const int SOUTH = 1;
const int EAST = 2;
const int WEST = 3;



int directionHelper(double directionAngle);
void draw_street_segments_basic(ezgl::renderer *g);
void curvexyStreet_populate();
double street_zoom_factor();
void draw_segment(ezgl::renderer *g, double adjustedWidth, double initialWidth,
        double adjustedZoomFactor, ezgl::color color, 
        StreetSegmentIdx segment);
void print_street_names(ezgl::renderer * g);
void print_names(ezgl::renderer * g, StreetSegmentIdx segemnt);
void reloadMap();
ezgl::rectangle curr_world(ezgl::renderer * g);
void draw_subway_lines(ezgl::renderer *g);
double pathAngle(ezgl::point2d p1, ezgl::point2d p2, ezgl::point2d p3);
std::vector<int> directionsBetweenStreets(std::vector<StreetSegmentIdx> path, IntersectionIdx to);
void printPath(ezgl::renderer *g);
std::vector<StreetIdx> streetIdPath(std::vector<StreetSegmentIdx> pathSegments);
std::vector<double> streetDistances(std::vector<StreetSegmentIdx> pathSegments);
int initialDirectionHelper(double directionAngle);

