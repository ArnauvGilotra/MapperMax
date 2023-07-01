#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m2.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "globals.h"



#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

//libraries being added
#include <cmath>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <iostream>
#include <sstream>



void draw_intersections_as_boxes(ezgl::renderer *g);
double x_from_lon(double longitude);
double y_from_lat(double latitude);
double lon_from_x(double x);
double lat_from_y(double y);
void clear_screen(ezgl::renderer *g);
void setMinMaxLatLon();
void initial_setup (ezgl::application* application, bool /*new_window */);
void toggle_clear(GtkWidget* /*widget*/, ezgl::application* application);
void act_on_mouse_click(ezgl::application* app, //function signature ezgl exists
GdkEventButton* event,
double x, double y);
void city_names();
//void draw_main_canvas(ezgl::renderer *g);
void printCityNames(ezgl::renderer *g);
void highlightCommonIntersections(StreetIdx street_id1, StreetIdx street_id2);
void clearMarkers();





