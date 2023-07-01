#include "search_feature.h"
#include "canvas.hpp"

std::vector<segmentData> curvexyStreetSegments;

//function calculates the distance traveled on each street along a given a path
//(vector of street segments)
//returns a vector of travel distances for each street
std::vector<double> streetDistances(std::vector<StreetSegmentIdx> pathSegments){
    int numSegs = pathSegments.size();
    std::vector<double> streetDistance;//local vector to store information
    double distance = 0;
    
    //if vector pathSegments only has a single element
    if(numSegs == 1){
        int seg = 0;//index for StreetSEgmentID
        distance +=  segmentLength[pathSegments[seg]][0];//preloaded global vector stores length of each segment
        streetDistance.push_back(distance);//calculated distance input in vector
    }
    
    //for loop runs when pathSegments has more than 1 element
    for(auto seg = 0; seg<(numSegs-1); seg++){
        StreetSegmentInfo currSeg = getStreetSegmentInfo(pathSegments[seg]);
        StreetSegmentInfo nextSeg = getStreetSegmentInfo(pathSegments[seg+1]);
        distance += segmentLength[pathSegments[seg]][0];//distance updated for each new iteration
        if(currSeg.streetID != nextSeg.streetID){//statement ensures that distance is pushed
            streetDistance.push_back(distance);   //only once new street reached
            distance=0;                          //reinitializes distance
        }
        if(seg==(numSegs-2)){//checks when loop runs for the last time
            distance += segmentLength[pathSegments[seg+1]][0];//adds lenth of last segment
            streetDistance.push_back(distance);
        }
    } return streetDistance;//returns locally populated vector
}

//helper function for directionsBetweenStreets, returns initial direction of travel
int initialDirectionHelper(double directionAngle){
    //given an angle (positive for left, negetive for right), checks 
    //bounds to return direction of travel
    if (directionAngle>=0){
        if(directionAngle<(M_PI/4)){
            return EAST;
        } 
        else if (directionAngle>=(M_PI/4)&&directionAngle<=(M_PI*(3/4))){
            return NORTH;
        } 
        else {
            return WEST;
        }
    } else {
        if(abs(directionAngle)<(M_PI/4)){
            return EAST;
        } 
        else if (abs(directionAngle)>=(M_PI/4) && abs(directionAngle)<=(M_PI*3/4)){
            return SOUTH;
        }
        else {
            return WEST;
        }
    }
    return -1;//if none of the above directions(should never happen)
}

//helper function for directionBetweenStreets to return direction of turn atfer the first street
int directionHelper(double directionAngle){
    //similar to initialDirectionHelper, checks angle to identify direction and degree of turn
    if (directionAngle>=0){
        if(directionAngle<(M_PI/3)){
            return SHARP_LEFT;
        } 
        else if (directionAngle>=(M_PI*2/3)&&directionAngle<=(M_PI*(11/12))){
            return SLIDE_LEFT;
        } 
        else if (directionAngle > (M_PI*11/12)){
            return STRAIGHT;
        } 
        else {
            return LEFT;
        }


    } else if (directionAngle<0){
        if(abs(directionAngle)<(M_PI/3)){
            return SHARP_RIGHT;
        } 
        else if (abs(directionAngle)>=(M_PI*2/3) && abs(directionAngle)<=(M_PI*(11/12))){
            return SLIDE_RIGHT;
        } 
        else if (abs(directionAngle) > (M_PI*11/12)){
            return STRAIGHT;
        }
        else{
            return RIGHT;
        }
    }
    return -1;//returns if angle NaN(should never happen)
}

//function finds angle between three points given in order (p1,p2,p3)
double pathAngle(ezgl::point2d p1, ezgl::point2d p2, ezgl::point2d p3){
    //computation always gives angle in a particular direction (clockwise)
    double dirP2ToP1 = atan2(p1.y - p2.y, p1.x - p2.x);
    double dirP2ToP3 = atan2(p3.y - p2.y, p3.x - p2.x);
    double angle = dirP2ToP1 - dirP2ToP3;
    
    //to ensure return value is between -pi, and pi
    if (angle > M_PI) angle -= 2*M_PI;
    else if (angle < -M_PI) angle += 2*M_PI;

    return angle;//positive when left
}

//prints the path found between 2 intersections
void printPath(ezgl::renderer *g){
    for(auto i:globalPath){
        draw_segment(g, motorAdjWidth, motorInitialWidth, motorZoomFactor, pathColor, i);
    }
}

//returns a vector of Street IDs given a vector of segment IDs every time a street changes
std::vector<StreetIdx> streetIdPath(std::vector<StreetSegmentIdx> pathSegments){
    int numSegs = pathSegments.size();
    std::vector<StreetIdx> streetPath;//local vector of Street Ids
    
    if (numSegs==1){//when pathSegments has only one elements
        int seg = 0;
        streetPath.push_back(getStreetSegmentInfo(pathSegments[seg]).streetID);
    }
    for(auto seg = 0; seg<(numSegs-1); seg++){//loop when pathSegments has more than 1 element
        StreetSegmentInfo currSeg = getStreetSegmentInfo(pathSegments[seg]);
        StreetSegmentInfo nextSeg = getStreetSegmentInfo(pathSegments[seg+1]);
        if(seg==0){//inouts into vector the first StreetID of the first element in pathSegments
            streetPath.push_back(currSeg.streetID);
        }
        if(currSeg.streetID != nextSeg.streetID){//inputs StreetID only when change in Street
            streetPath.push_back(nextSeg.streetID);
        }
    } 
    return streetPath;//returns local vector
}

//returns a vector of turns/directions for a path, every time there is a change in Street
std::vector<int> directionsBetweenStreets(std::vector<StreetSegmentIdx> pathSegments, 
        IntersectionIdx toIntersection){
    int numSegs = pathSegments.size();
    std::vector<int> directions;//local vector
    directions.clear();
    if(numSegs ==1){//for when only 1 segment in pathSegments
        int seg = 0;
        double directionAngle;
        StreetSegmentInfo currSeg = getStreetSegmentInfo(pathSegments[seg]);
        ezgl::point2d point1 = *(curvexyStreetSegments[pathSegments[seg]].xy_points.end()-1);
        ezgl::point2d point2 = curvexyStreetSegments[pathSegments[seg]].xy_points[0];
        //calculates angle of the street segment with respect to horizontal 
        directionAngle = atan2((point1).y-(point2).y,(point1).x-(point2).x);
        //checks orientation of street segment against the travel direction, inverts angle if different
        if(currSeg.from == toIntersection){
            if(directionAngle > 0) directionAngle -= M_PI;
            else directionAngle+=M_PI;
        }
        //uses initialDirectionHelper to find direction of travel for segment, input into vector
        directions.push_back(initialDirectionHelper(directionAngle));    
    }
    
    for(auto seg = 0; seg<(numSegs-1); seg++){//for more than 1 element in pathSegments
        double directionAngle = 0;
        StreetSegmentInfo currSeg, nextSeg;
        currSeg = getStreetSegmentInfo(pathSegments[seg]);
        nextSeg = getStreetSegmentInfo(pathSegments[seg+1]);
        
        if(seg==0){//for the first intersection, initail direction found
            auto pointer1 = curvexyStreetSegments[pathSegments[seg]].xy_points.end()-1;
            auto pointer2 = curvexyStreetSegments[pathSegments[seg]].xy_points.begin();
            if(currSeg.from == nextSeg.to || currSeg.from == nextSeg.from){
                pointer1 = curvexyStreetSegments[pathSegments[seg]].xy_points.begin();
                pointer2 = curvexyStreetSegments[pathSegments[seg]].xy_points.end()-1;
            }

            directionAngle = atan2((*pointer1).y-(*pointer2).y,(*pointer1).x-(*pointer2).x);
            directions.push_back(initialDirectionHelper(directionAngle));    

        }
            
        //for a change in street, angle is found between the incoming and outgoing street segments
        //calculates and inputs direction of turn in order of the path
        if(currSeg.streetID != nextSeg.streetID){
            
            if (currSeg.to == nextSeg.from){
                
                ezgl::point2d p1 = curvexyStreetSegments[pathSegments[seg]].center;
                if(currSeg.numCurvePoints == 0){
                    p1 = curvexyStreetSegments[pathSegments[seg]].xy_points[0];
                }
                
                ezgl::point2d p2 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[0];
                
                ezgl::point2d p3 = curvexyStreetSegments[pathSegments[seg+1]].center;
                if(nextSeg.numCurvePoints == 0){
                    p3 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[1];
                }

                directionAngle = pathAngle(p1, p2, p3);
                
            } else if (currSeg.from == nextSeg.to){
                
                ezgl::point2d p1 = curvexyStreetSegments[pathSegments[seg]].center;
                if(currSeg.numCurvePoints == 0){
                    p1 = curvexyStreetSegments[pathSegments[seg]].xy_points[1];
                }
                
                ezgl::point2d p2 = curvexyStreetSegments[pathSegments[seg]].xy_points[0];
                
                ezgl::point2d p3 = curvexyStreetSegments[pathSegments[seg+1]].center;
                if(nextSeg.numCurvePoints == 0){
                    p3 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[0];
                }
                
                directionAngle = pathAngle(p1, p2, p3);
                
            } else if (currSeg.to == nextSeg.to){
                
                auto pointer1 = (curvexyStreetSegments[pathSegments[seg+1]].xy_points.end()-1);
                
                ezgl::point2d p1 = curvexyStreetSegments[pathSegments[seg]].center;
                if(currSeg.numCurvePoints == 0){
                    p1 = curvexyStreetSegments[pathSegments[seg]].xy_points[0];
                }
                
                ezgl::point2d p2 = *(pointer1);

                ezgl::point2d p3 = curvexyStreetSegments[pathSegments[seg+1]].center;
                if(nextSeg.numCurvePoints == 0){
                    p3 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[0];
                }
                
                directionAngle = pathAngle(p1, p2, p3);
                
            } else if (currSeg.from == nextSeg.from){

                ezgl::point2d p1 = curvexyStreetSegments[pathSegments[seg]].center;
                if(currSeg.numCurvePoints == 0){
                    p1 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[1];
                }
                
                ezgl::point2d p2 = curvexyStreetSegments[pathSegments[seg]].xy_points[0];
                
                ezgl::point2d p3 = curvexyStreetSegments[pathSegments[seg+1]].center;
                if(nextSeg.numCurvePoints == 0){
                    p3 = curvexyStreetSegments[pathSegments[seg+1]].xy_points[1];
                }

                directionAngle = pathAngle(p1, p2, p3);
            }
            
            directions.push_back(directionHelper(directionAngle));
        }
    } return directions;//returns local vector
}

//function to populate vector curveStreetSegments
void curvexyStreet_populate(){
    curvexyStreetSegments.resize(getNumStreetSegments());
    //loop through all segments
    for (StreetSegmentIdx segments = 0; segments< getNumStreetSegments(); segments++){
    
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segments);
        int totalPoints = segmentInfo.numCurvePoints+2;//total number of points for a segment including start, end and curve points
        for(int point = 0; point < totalPoints; point++){
            //converting from latlon to world coordinates
            curvexyStreetSegments[segments].xy_points.resize(totalPoints);
            curvexyStreetSegments[segments].xy_points[point].x =  
                    x_from_lon(curveStreetSegments[segments][point].longitude());
            curvexyStreetSegments[segments].xy_points[point].y = 
                    y_from_lat(curveStreetSegments[segments][point].latitude());
        }
        //gives index of where to place street name by taking the middle index
        int center_pos = totalPoints/2-1;
        //distance between center index and next index to check for enough space when placing names
        double distance = sqrt((curvexyStreetSegments[segments].xy_points[center_pos+1].y
                        -curvexyStreetSegments[segments].xy_points[center_pos].y)*
                        (curvexyStreetSegments[segments].xy_points[center_pos+1].y
                        -curvexyStreetSegments[segments].xy_points[center_pos].y)+
                           (curvexyStreetSegments[segments].xy_points[center_pos+1].x
                        -curvexyStreetSegments[segments].xy_points[center_pos].x)*
                        (curvexyStreetSegments[segments].xy_points[center_pos+1].x
                        -curvexyStreetSegments[segments].xy_points[center_pos].x));
        
        curvexyStreetSegments[segments].distance = distance;
        
        //midpoint(center of street segment) calculated based on the middle of the center_pos and next index
        curvexyStreetSegments[segments].center.x= (curvexyStreetSegments[segments].xy_points[center_pos].x + 
                curvexyStreetSegments[segments].xy_points[center_pos+1].x)/2 ;
        curvexyStreetSegments[segments].center.y= (curvexyStreetSegments[segments].xy_points[center_pos].y + 
                curvexyStreetSegments[segments].xy_points[center_pos+1].y)/2 ;
        
        //changeInX and chhangeinY calculated to find angle of rotation(align street name with street) 
        double changeInY =  curvexyStreetSegments[segments].xy_points[center_pos+1].y
                        -curvexyStreetSegments[segments].xy_points[center_pos].y;
        double changeInX = curvexyStreetSegments[segments].xy_points[center_pos+1].x
                        -curvexyStreetSegments[segments].xy_points[center_pos].x;
        //rotation angle bounded between 90 and -90, directionChanged tells if street name oriented in opposite
        //direction to street orientation
        curvexyStreetSegments[segments].directionChanged = false;
        double angle = atan2(changeInY, changeInX)/kDegreeToRadian;
        if(angle<=90 && angle >-90){
            curvexyStreetSegments[segments].angle = angle;
        } else if(angle>90){
            curvexyStreetSegments[segments].directionChanged = true;
            angle = angle -180;
            curvexyStreetSegments[segments].angle = angle;
        } else {
            curvexyStreetSegments[segments].directionChanged = true;
            angle = angle +180;
            curvexyStreetSegments[segments].angle = angle;
        }
        
    }
    
	
}

//draws subway lines
void draw_subway_lines(ezgl::renderer * g){
    ezgl::surface* sub = ezgl::renderer::load_png("libstreetmap/resources/icons/subway.png");
    int numWays = subwayWay.size();//gets the total number of paths that make up the subway network
    ezgl::rectangle my_world = curr_world(g);
    for (int way = 0; way<numWays; way++){//loop through all paths
        double line_width = 2; //subway line width 
        for(auto stop = subwayWay[way].begin(); stop != (subwayWay[way].end()-1);stop++){//loops through all nodes in a path
            
            if(my_world.contains(*stop)){//condition only renders if subway line within visible scope
            
                g->set_color(ezgl::color(255,0,0,50));
                g->set_line_width(line_width);
                g->draw_line(*stop, *(stop+1));//draws line between each node
            }
        }
    }
    //prints all the subway stops
    //double stop_size = 5; 
    for(int stop = 0;stop < subwayStops.size(); stop = stop + 2){
        if(stop > subwayStops.size()) continue;
        if(my_world.contains(subwayStops[stop]) && street_zoom_factor() < 10){
                g->draw_surface(sub, subwayStops[stop]);
                //g->set_color(ezgl::RED);
                //g->fill_arc(*stop, stop_size, 0, 360);//draws a circle at each stop
            }
    }
    ezgl::renderer::free_surface(sub);
}

//changes map view when a diffrent map selected from drop down
void reloadMap(){
    //extracting text from drop-down selection
    gtk_widget_hide(GTK_WIDGET(bottomLabel));
    const gchar* active_option = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(comboBox));
    std::string city_name = active_option;
    //if else statements to specify loadmap path for each drop down text
    if(city_name == "Toronto"){
        city_name = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    } else if (city_name == "New Delhi"){
        city_name = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    } else if (city_name == "Tokyo"){
        city_name = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    } else if (city_name == "Tehran"){
        city_name = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    } else if (city_name == "Singapore"){
        city_name = "/cad2/ece297s/public/maps/singapore.streets.bin";
    } else if (city_name == "Saint-Helena"){
        city_name = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    } else if (city_name == "New York"){
        city_name = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    } else if (city_name == "Rio-De-Janeiro"){
        city_name = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    } else if (city_name == "Sydney"){
        city_name = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    } else if (city_name == "Hong Kong"){
        city_name = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    } else if (city_name == "Iceland"){
        city_name = "/cad2/ece297s/public/maps/iceland.streets.bin";
    } else if (city_name == "Interlaken"){
        city_name = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    } else if (city_name == "Kyiv"){
        city_name = "/cad2/ece297s/public/maps/kyiv_ukraine.streets.bin";
    } else if (city_name == "London"){
        city_name = "/cad2/ece297s/public/maps/london_england.streets.bin";
    } else if (city_name == "Hamilton"){
        city_name = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    } else if (city_name == "Golden Horseshoe"){
        city_name = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    } else if (city_name == "Cape Town"){
        city_name = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    } else if (city_name == "Cairo"){
        city_name = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    } else if (city_name == "Beijing"){
        city_name = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    }
    //when drop-down changed, closes current map, reopens new map based on current map selection
    closeMap();
    loadMap(city_name);
    exitDir();
    ezgl::rectangle updated_world({x_from_lon(min_lon),y_from_lat(min_lat)}, 
    {x_from_lon(max_lon),y_from_lat(max_lat)});
    ezgl::camera & cam = canv->get_camera();
    cam.reset_world(updated_world);//updates coordinates for map based on x,y of new location
    canv->redraw();//redraws the new map
}

//returns visible bounds of map in application window
ezgl::rectangle curr_world(ezgl::renderer * g){
    return g->get_visible_world();
}

//returns the metres/pixel ratio of the x coordinates
double street_zoom_factor(){
    ezgl::camera & cam = canv->get_camera();
    double zoom_factor = cam.get_world_scale_factor().x;
    return zoom_factor;
}

//prints each individual name of a segment
void print_names(ezgl::renderer *g, StreetSegmentIdx segment){
    StreetSegmentInfo segInfo = getStreetSegmentInfo(segment);
    std::string streetName = getStreetName(segInfo.streetID);//street name extracted
    if(segInfo.oneWay){//appends direction of a oneway street based on street orientation
        if(curvexyStreetSegments[segment].directionChanged){
            streetName.append(" <<--");
        }else  streetName.append(" -->>");
    }
    ezgl::rectangle my_world = curr_world(g);
    double fontSize = 11; //constant street name font size
    if(curvexyStreetSegments[segment].distance/street_zoom_factor()>(streetName.length()*5) && streetName!= "<unknown>"
            && my_world.contains(curvexyStreetSegments[segment].center)
            ){//conditional statement to print only known street names for segments in view, only if 
        //they have enough space to display the name

    g->set_color(ezgl::BLACK);//set text color to black
    g->set_text_rotation(curvexyStreetSegments[segment].angle);
    g->set_font_size(fontSize);
    g->draw_text(curvexyStreetSegments[segment].center, streetName);}//draws name based on center calculated earlier
}

//loops through all street segments to print names
void print_street_names(ezgl::renderer * g){
    for(int i= 0; i<getNumStreetSegments(); i++){
        print_names(g, i);
    }
}

void draw_segment(ezgl::renderer *g, double adjustedWidth, double initialWidth,
        double adjustedZoomFactor, ezgl::color color, 
        StreetSegmentIdx segment){
        ezgl::rectangle my_world = curr_world(g);//get bounds of visible world
            g->set_color(color);
            double zoom = street_zoom_factor();
            if(zoom<adjustedZoomFactor){
                double width = initialWidth;//draws line with minimum width, at a certain zoom level
                if (adjustedWidth/zoom>= initialWidth){
                   width = adjustedWidth/zoom; //changes line width when zoom big enough
                }
                g->set_line_width(width);
                //loops through all points in segment to draw lines
                for (auto point = curvexyStreetSegments[segment].xy_points.begin(); point!=curvexyStreetSegments[segment].xy_points.end()-1; point++){
                    if(my_world.contains(*point)||my_world.contains(*(point+1))){//conditional statement to only map if street segment in view 
                    g->draw_line(*point,*(point+1));    
                    g->fill_arc(*point, (width*zoom)/2, 0, 360);
                        if(point == curvexyStreetSegments[segment].xy_points.end()-2){
                            g->fill_arc(*(point+1), (width*zoom)/2, 0, 360);
                        }
                    }
                }
            }
}

//decides parameters for all different types of street segments, and call draw_segment
void draw_street_segments_basic(ezgl::renderer *g){
    g->set_coordinate_system(ezgl::SCREEN);
    g->set_color(ezgl::BLACK);
    g->set_coordinate_system(ezgl::WORLD);
    for (StreetSegmentIdx segments=0; segments< getNumStreetSegments(); segments++){
        
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segments);
        std::string roadType = getOSMNodeTagValue(segmentInfo.wayOSMID, "highway");
        if (roadType == "motorway" ||roadType == "motorway_link"){
            draw_segment(g, motorAdjWidth, motorInitialWidth, motorZoomFactor, motorwayColor, segments);
        }else if (roadType == "trunk" ||roadType == "trunk_link"){
            draw_segment(g, primtAdjWidth, primtInitialWidth, primtZoomFactor, ezgl::WHITE, segments);
        }else if (roadType == "primary" || roadType == "primary_link"){
            draw_segment(g, primtAdjWidth, primtInitialWidth, primtZoomFactor, ezgl::WHITE, segments);
        }else if (roadType == "secondary" || roadType == "secondary-link"){
            draw_segment(g, secAdjWidth, secInitialWidth, secZoomFactor, ezgl::WHITE, segments);
        } else if (roadType == "tertiary" || roadType == "tertiary_link"){
            draw_segment(g, terAdjWidth, terInitialWidth, terZoomFactor, ezgl::WHITE, segments);
        }else if (roadType == "residential"){
            draw_segment(g, resAdjWidth, resInitialWidth, resZoomFactor, ezgl::WHITE, segments);
        }else draw_segment(g, othAdjWidth, othInitialWidth, othZoomFactor, ezgl::WHITE, segments);
       
    }
}