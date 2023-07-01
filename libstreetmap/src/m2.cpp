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


#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m2.h"
#include "m3.h"
#include <cmath>
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "feature.h"
#include "search_feature.h"
#include "front_end.h"
#include <sstream>

#include "globals.h"
std::vector<intersectionInfo> intersections;
std::vector<Inter_data> intersections_xy;


//GLOBAL VARIABLES
double AVG_LAT_RAD;
double max_lat;
double min_lat;
double max_lon;
double min_lon;
bool CLEAR_SCREEN=FALSE;
void draw_main_canvas(ezgl::renderer *g);
void openCanvas();
ezgl::canvas* canv;

// Set up the ezgl graphics window and hand control to it, as shown in the 
// ezgl example program. 
// This function will be called by both the unit tests (ece297exercise) 
// and your main() function in main/src/main.cpp.
// The unit tests always call loadMap() before calling this function
// and call closeMap() after this function returns.
void drawMap() {
    openCanvas();
}

void draw_main_canvas(ezgl::renderer *g){
    
    if (CLEAR_SCREEN) {
        clear_screen(g);    
    }
    
    else{   
        draw_features(g);
        draw_street_segments_basic(g);
        printCityNames(g);
        draw_subway_lines(g);
        printPOI(g);
        printPath(g);
        print_street_names(g);
        draw_intersections_as_boxes(g);
    }

}

void openCanvas(){
    
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    ezgl::rectangle initial_world({x_from_lon(min_lon),y_from_lat(min_lat)}, 
    {x_from_lon(max_lon),y_from_lat(max_lat)});

    
    canv = application.add_canvas("MainCanvas", draw_main_canvas, initial_world, 
            ezgl:: color(225,225,225));

    application.run(initial_setup, act_on_mouse_click, nullptr, nullptr);    
    
}