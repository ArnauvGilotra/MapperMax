#include "feature.h"
#include "front_end.h"

std::vector<IntersectionIdx> streetIntersections;
std::vector<cityNamesLatLon> cityInfo;
GObject *searchFromBar;
GObject *searchToBar;
GObject *bottomLabel;
GObject *comboBox;
GObject *f;
GObject *f1;
GObject *f2;
GObject *f3;
GObject *f0;
GObject *t;
GObject *t1;
GObject *t2;
GObject *t3;
GObject *t0;
GObject *info;
GObject *food;
GObject *emer;
GObject *all;
GObject *edu;
GObject *ent;
GObject *listbox;
GObject *route;
GObject *dir;
GObject *tutbox;
GObject *tutview;
GObject *tutbtn;
GObject *help;
GObject *find;
GObject *hidepane;
GObject *exitdir;
GObject *showdirpane;


bool streetIntersectionsPresent;
bool highlight_present = false;
bool searchFirstIntersection = false;
bool searchSecondIntersection = false;

int firstSearchHighlight = 0;
int secondSearchHighlight = 0;
int id_last_highlight = 0;
gulong handlerFrom;
gulong handlerTo;


//function to populate a vector of structs called cityInfo
//used to store the name of the city and the location of the city
//in latlon and xy
void city_names(){
    cityInfo.resize(osm_city_names.size());
    for(int cityNum=0;cityNum<osm_city_names.size(); cityNum++){
        
        if(getOSMNodeTagValue(osm_city_names[cityNum]->id(), "name:en")!=""){
            cityInfo[cityNum].cityName= 
                getOSMNodeTagValue(osm_city_names[cityNum]->id(), "name:en");
        }
        else cityInfo[cityNum].cityName= 
                getOSMNodeTagValue(osm_city_names[cityNum]->id(), "name");
        
        
        cityInfo[cityNum].cityPosLL = osm_city_names[cityNum]->coords();
        
        cityInfo[cityNum].cityPosXY.y = 
                y_from_lat(osm_city_names[cityNum]->coords().latitude());
        
        cityInfo[cityNum].cityPosXY.x = 
                x_from_lon(osm_city_names[cityNum]->coords().longitude());
    }
    
}

//function to print the city names using the vector that was populated
//in void city_names
void printCityNames(ezgl::renderer *g){
    
    //to avoid overlapping
    //and to hide when too zoomed in as street names come into place
    // so city names not very relevant after that point
    //Observational constants used to improve usability
    int restrictingSize = 10;
    int zoomValueManyCitiesMin = 15;
    int zoomValueManyCitiesMax = 61;
    int zoomValueLessCities = 30;

    
    if (street_zoom_factor()<zoomValueLessCities && 
            cityInfo.size()<restrictingSize) return;
    
    if (street_zoom_factor()<zoomValueManyCitiesMin && 
            cityInfo.size()>restrictingSize) return;
    
    if(cityInfo.size()>restrictingSize && 
            street_zoom_factor()>zoomValueManyCitiesMax) return;
    
    
    for(int numCity=0; numCity<cityInfo.size(); numCity++){
        g->set_font_size(20);
        
        g->format_font("calibri", ezgl::font_slant::normal, 
                ezgl::font_weight::bold);
        
        g->set_color(ezgl::BLACK);
        
        g->draw_text(cityInfo[numCity].cityPosXY, cityInfo[numCity].cityName);
    }
    
}

//function to place/remove pin whenever we click om the screen
//places the pin on the closest intersection
void draw_intersections_as_boxes(ezgl::renderer *g){
    for (size_t inter_i = 0; inter_i < intersections.size(); ++inter_i) {

        if (intersections_xy[inter_i].highlight) {
        
            ezgl::surface *pin_surface = 
                    ezgl::renderer::load_png("libstreetmap/resources/pin2.png");

            ezgl::point2d inter_loc = intersections_xy[inter_i].xy_loc;
            if(intersections_xy[inter_i].markerType==INTER_MARKER){
                pin_surface = 
                        ezgl::renderer::load_png("libstreetmap/resources/pin2.png");               
            }
            else if(intersections_xy[inter_i].markerType==START_MARKER){
                pin_surface = 
                        ezgl::renderer::load_png("libstreetmap/resources/green_pin.png");              
            }
            else if(intersections_xy[inter_i].markerType==END_MARKER){
                pin_surface = 
                        ezgl::renderer::load_png("libstreetmap/resources/blue_pin.png");              
            }
            

            g->draw_surface(pin_surface, inter_loc);
            ezgl::renderer::free_surface(pin_surface);
                        
        } 
    }
}

//-------------------------------------------------

// FUNCTIONS FOR CONVERSION FROM/TO XY AND LATLON

double x_from_lon(double longitude){
    return kEarthRadiusInMeters*kDegreeToRadian*longitude*cos(AVG_LAT_RAD);
}


double y_from_lat(double latitude){
    return kEarthRadiusInMeters*latitude*kDegreeToRadian;
}
        

double lon_from_x(double x){
    return x/(kEarthRadiusInMeters*kDegreeToRadian*cos(AVG_LAT_RAD));
}


double lat_from_y(double y){
    return y/(kEarthRadiusInMeters*kDegreeToRadian);
}

//----------------------------------------------------------

void clear_screen(ezgl::renderer *g){
    ezgl::rectangle visible_world = g->get_visible_world();
    g->set_color(255, 255, 255);
    g->fill_rectangle(visible_world);
}

// FUNCTION TO GET THE BOUNDS OF THE MAP
//CALCULATES AVERAGE LAT

void setMinMaxLatLon(){
    max_lat = getIntersectionPosition(0).latitude();
    min_lat = max_lat;
    max_lon = getIntersectionPosition(0).longitude();
    min_lon = max_lon;
    
    intersections.resize(getNumIntersections());
    intersections_xy.resize(getNumIntersections());

    for (int id = 0; id < getNumIntersections(); ++id) {  
        intersections[id].position = getIntersectionPosition(id);
        intersections[id].name = getIntersectionName(id);
        intersections_xy[id].name = getIntersectionName(id);

        max_lat = std::max(max_lat, intersections[id].position.latitude());
        min_lat = std::min(min_lat, intersections[id].position.latitude());
        max_lon = std::max(max_lon, intersections[id].position.longitude());
        min_lon = std::min(min_lon, intersections[id].position.longitude());
    }
    
    AVG_LAT_RAD = (min_lat+max_lat)*.5*kDegreeToRadian;
    
        
    for(int id = 0; id < getNumIntersections(); ++id){
        intersections_xy[id].xy_loc.x = x_from_lon(getIntersectionPosition(id).longitude());
        intersections_xy[id].xy_loc.y = y_from_lat(getIntersectionPosition(id).latitude());
        
    }
}

// ADDING WIDGETS

void initial_setup (ezgl::application* application, bool /*new_window */){
    isInitial = TRUE;
    addStyle(application);
    
    searchFromBar = application->get_object("searchBarFrom");
    searchToBar = application->get_object("searchBarTo");
    bottomLabel = application->get_object("bottomLabel");
    comboBox = application->get_object("cityDropDown");
    f = application->get_object("f");
    f0 = application->get_object("f0");
    f1 = application->get_object("f1");
    f2 = application->get_object("f2");
    f3 = application->get_object("f3");
    t = application->get_object("t");
    t0 = application->get_object("t0");
    t1 = application->get_object("t1");
    t2 = application->get_object("t2");
    t3 = application->get_object("t3");
    info = application->get_object("info");
    food = application->get_object("food");
    ent = application->get_object("ent");
    edu = application->get_object("edu");
    emer = application->get_object("emer");
    all = application->get_object("all");
    listbox = application->get_object("routerow");
    route = application->get_object("route");
    dir = application->get_object("dir");
    tutbox = application->get_object("tutbox");
    tutview = application->get_object("tutview");
    tutbtn = application->get_object("tutbtn");
    help = application->get_object("help");
    find = application->get_object("find");
    hidepane = application->get_object("hidepane");
    exitdir = application->get_object("exitdir");
    showdirpane = application->get_object("showdirpane");
    gtk_widget_hide(GTK_WIDGET(f1));
    gtk_widget_hide(GTK_WIDGET(f2));
    gtk_widget_hide(GTK_WIDGET(f3));
    gtk_widget_hide(GTK_WIDGET(f0));
    gtk_widget_hide(GTK_WIDGET(t1));
    gtk_widget_hide(GTK_WIDGET(t2));
    gtk_widget_hide(GTK_WIDGET(t3));
    gtk_widget_hide(GTK_WIDGET(t0));
    gtk_widget_hide(GTK_WIDGET(bottomLabel));
    gtk_widget_hide(GTK_WIDGET(food));
    gtk_widget_hide(GTK_WIDGET(ent));
    gtk_widget_hide(GTK_WIDGET(emer));
    gtk_widget_hide(GTK_WIDGET(edu));
    gtk_widget_hide(GTK_WIDGET(all));
    gtk_widget_hide(GTK_WIDGET(route));
    gtk_widget_hide(GTK_WIDGET(find));
    gtk_widget_hide(GTK_WIDGET(showdirpane));
    
    g_signal_connect(
        comboBox,
        "changed",
        G_CALLBACK(reloadMap),
        application);

    g_signal_connect( f0, "clicked", G_CALLBACK(click0), application);
    g_signal_connect( f1, "clicked", G_CALLBACK(click1), application);
    g_signal_connect( f2, "clicked", G_CALLBACK(click2), application);
    g_signal_connect( f3, "clicked", G_CALLBACK(click3), application);
    g_signal_connect( t0, "clicked", G_CALLBACK(clickt0), application);
    g_signal_connect( t1, "clicked", G_CALLBACK(clickt1), application);
    g_signal_connect( t2, "clicked", G_CALLBACK(clickt2), application);
    g_signal_connect( t3, "clicked", G_CALLBACK(clickt3), application);
    g_signal_connect( all, "clicked", G_CALLBACK(showAll), application);
    g_signal_connect( ent, "clicked", G_CALLBACK(showEnt), application);
    g_signal_connect( edu, "clicked", G_CALLBACK(showEdu), application);
    g_signal_connect( food, "clicked", G_CALLBACK(showFood), application);
    g_signal_connect( emer, "clicked", G_CALLBACK(showEmer), application);
    g_signal_connect( dir, "notify::active", G_CALLBACK(directionMode), application);
    g_signal_connect( tutbtn, "clicked", G_CALLBACK(closeTut), application);
    g_signal_connect( help, "clicked", G_CALLBACK(showTut), application);
    g_signal_connect( exitdir, "clicked", G_CALLBACK(exitDir), application);
    g_signal_connect( hidepane, "clicked", G_CALLBACK(hideDir), application);
    g_signal_connect( showdirpane, "clicked", G_CALLBACK(reShowDir), application);
    g_signal_connect( find, "clicked", G_CALLBACK(displayPathFind), application);

    handlerFrom = g_signal_connect(
        searchFromBar, // pointer to the UI widget
        "search-changed", // Signal state of switch being changed
        G_CALLBACK(searchAuto), // name of callback function (you write this function:
        // make sure its declaration is visible)
        application // passing an application ptr.
    );

    handlerTo = g_signal_connect(
        searchToBar, // pointer to the UI widget
        "search-changed", // Signal state of switch being changed
        G_CALLBACK(searchAutoTo), // name of callback function (you write this function:
        // make sure its declaration is visible)
        application // passing an application ptr.
    );
    
}

// FUNCTION THAT CALLS draw_intersections_as_boxes
// PLACES THE PIN ON CLOSEST INTERSECTION
// DISPLAYS NAME OF INTERSECTION ON A PANEL BELOW

void act_on_mouse_click(ezgl::application* app, //function signature ezgl exists
GdkEventButton* /*event*/,
double x, double y) {
    
    //checking which state we are in
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));
    
    //removing any previous markers
    if(highlight_present){
        intersections_xy[id_last_highlight].highlight=false;
    }

    
    //if finding the closest intersection
    if(!active){
        
        gtk_widget_hide(GTK_WIDGET(find));

        //resetting if coming back from direction mode
        exitDir();

        highlight_present = true; //only to avoid seg fault the first time we place

        LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
        int id = findClosestIntersection(pos);
        id_last_highlight = id;

        intersections_xy[id].highlight = true;
        
        gtk_widget_show(GTK_WIDGET(bottomLabel));

        std::string intersectionString = "Closest intersection: " + (intersections[id].name);

        gtk_label_set_text(GTK_LABEL(bottomLabel), intersectionString.c_str());

        app->refresh_drawing(); // TO FORCE REDRAW
        //causes draw_intersection_as_boxes to be called
        return;
    }
    
    //now in directions finding mode
    else{
        
        // to clear the previous marker first if any
        //this highlight_present only corresponds to intersection highlight
        if(highlight_present){
            highlight_present = false; //removing the highlight in
            app->refresh_drawing();  
        }
        
        LatLon pos = LatLon(lat_from_y(y),lon_from_x(x));
        int id = findClosestIntersection(pos);
        
        //receive the first click
        
        if(!searchFirstIntersection){

            searchFirstIntersection = true; //first intersection received
            intersections_xy[id].highlight = true;
            intersections_xy[id].markerType = START_MARKER;
            firstSearchHighlight=id;
            gtk_widget_show(GTK_WIDGET(bottomLabel));

            std::string intersectionString = "Select second intersection to find path!";

            gtk_label_set_text(GTK_LABEL(bottomLabel), intersectionString.c_str());
            
            intersectionString = intersections_xy[id].name;
            
            gtk_entry_set_placeholder_text(GTK_ENTRY(searchFromBar), intersectionString.c_str());

            setFromInfo(id);

            app->refresh_drawing();
            
            return;
        }
        
        //second click
        
        if(searchFirstIntersection && !searchSecondIntersection){
            
            searchSecondIntersection=true;
            intersections_xy[id].highlight = true;
            intersections_xy[id].markerType = END_MARKER;
            secondSearchHighlight=id;
            
            // CALL FUNCTION TO DISPLAY THE PATH

            std::string intersectionString = "Both intersections pins placed! Click on Find Route!";

            gtk_label_set_text(GTK_LABEL(bottomLabel), intersectionString.c_str());
            gtk_widget_show(GTK_WIDGET(bottomLabel));
            
         
            intersectionString = intersections_xy[id].name;
            
            gtk_entry_set_placeholder_text(GTK_ENTRY(searchToBar), intersectionString.c_str());
            
            setToInfo(id);

            app->refresh_drawing();
            gtk_widget_show(GTK_WIDGET(find));

            
            return;
        }
        
        //third click
        
        if(searchFirstIntersection && searchSecondIntersection){
            
            return;
        }

    }
    
}

//FUNCTION TO PLACE PIN ON INTERSECTIONS FOUND COMMON B/W 2 STREETS

void highlightCommonIntersections(StreetIdx street_id1, StreetIdx street_id2){
    
    // checking if 0 intersections 
    std::vector<IntersectionIdx> commonIntersections;
    
    gtk_widget_show(GTK_WIDGET(bottomLabel));

    commonIntersections = findIntersectionsOfTwoStreets(street_id1, street_id2);
    //clearing previous intersections

    if (streetIntersectionsPresent){
        for(int inter_num=0; inter_num<streetIntersections.size(); inter_num++){
            intersections_xy[streetIntersections[inter_num]].highlight=false;
        }
    }
    
    if(commonIntersections.size()==0){
        //for now
        std::cout<<"No intersections found!"<<std::endl;       
        
        gtk_label_set_text(GTK_LABEL(bottomLabel), "No intersection found!");
        
        return;
    }
    // highlighting the intersections
    
    streetIntersectionsPresent = true; 

    streetIntersections.resize(commonIntersections.size());
    
    if(commonIntersections.size()==1) 
        gtk_label_set_text(GTK_LABEL(bottomLabel), "1 intersection found!");
    if(commonIntersections.size()>1){
        
        gtk_label_set_text(GTK_LABEL(bottomLabel),
                "Multiple intersections found!");

    }

    for(int inter_num=0; inter_num<commonIntersections.size(); inter_num++){
        intersections_xy[commonIntersections[inter_num]].highlight=true;
        
        //storing in a global variable to delete in the next iteration
        streetIntersections[inter_num]= commonIntersections[inter_num]; 
    }
    canv->redraw();
    
}

void clearMarkers(){
    searchFirstIntersection=false;
    searchSecondIntersection=false;
    intersections_xy[firstSearchHighlight].highlight=false;
    intersections_xy[secondSearchHighlight].highlight=false; 
    intersections_xy[firstSearchHighlight].markerType = INTER_MARKER;
    intersections_xy[secondSearchHighlight].markerType = INTER_MARKER;
}