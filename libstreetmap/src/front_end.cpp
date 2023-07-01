#include "front_end.h"
std::vector<StreetIdx> from;
std::vector<StreetIdx> to;
int fromID = -1;
int toID = -1;
#define RES 4
bool isInitial = FALSE;
bool showPOIAll = FALSE;
bool showPOIent = FALSE;
bool showPOIedu = FALSE;
bool showPOIemer = FALSE;
bool showPOIfood = FALSE;

std::vector<std::string> directions = {"North", "South", "East", "West"};

//This function draws features based on preset priority
void draw_features(ezgl::renderer* g) {
    double zoomFactor = street_zoom_factor();
    int drawTypes;
    if (zoomFactor < 10) drawTypes = 11; //which types of features to print
    else if (zoomFactor < 40) drawTypes = 10;
    else drawTypes = 9;

    for (int i = 0; i < drawTypes; i++)
    {
        int len = featurePoints[i].size();
        for (int j = 0; j < len; j++)
        {
            int numPoints = featurePoints[i][j].size();
            if (featurePoints[i][j][0].x == featurePoints[i][j][numPoints - 1].x &&
                featurePoints[i][j][0].y == featurePoints[i][j][numPoints - 1].y &&
                numPoints > 1) { //closed features as polygons
                setCol(i, g);
                g->fill_poly(featurePoints[i][j]);
            }
            else { //open features as lines
                setCol(i, g);
                double width = 2;
                g->set_line_width(width);
                for (int point = 0; point < (numPoints - 1); point++) {
                    g->draw_line(featurePoints[i][j][point], featurePoints[i][j][point + 1]);
                }
            }
        }
    }
    return;
}
//This function sets the color for the feature based on key input i
void setCol(int i, ezgl::renderer* g) {
    if (i == 0) {
        g->set_color(ezgl::color(0xEF, 0xEF, 0xEF));
    }
    else if (i == 4) {
        g->set_color(ezgl::color(0xEF, 0xEF, 0xEF));
    }
    else if (i == 8) {
        g->set_color(ezgl::color(170, 210, 156));
    }
    else if (i == 2) {
        g->set_color(ezgl::color(255, 242, 175));
    }
    else if (i == 1) {
        g->set_color(ezgl::color(170, 218, 255));
    }
    else if (i == 3) {
        g->set_color(ezgl::LIGHT_SKY_BLUE);
    }
    else if (i == 10) {
        g->set_color(ezgl::GREY_75);
    }
    else if (i == 6) {
        g->set_color(ezgl::color(195, 245, 178));
    }
    else if (i == 7) {
        g->set_color(ezgl::color(195, 236, 178));
    }
    else if (i == 9) {
        g->set_color(ezgl::color(170, 218, 255));
    }
    else if (i == 5) {
        g->set_color(ezgl::color(227, 244, 252));
    }
}
//This is the callback func for search From bar 
void searchAuto() {
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));

    const gchar* text = gtk_entry_get_text(GTK_ENTRY(searchFromBar));
    if (from.size() != 4) from.resize(4);
    
    std::vector<StreetIdx> resultFrom;

    if (!active) {
        resultFrom = findStreetIdsFromPartialStreetName(text);
    } else {
        resultFrom = partialIntersections(text);
    }
    
    std::vector<GObject*> btns = { f0,f1,f2,f3 };
    GList* children = gtk_container_get_children(GTK_CONTAINER(f));
    int size = resultFrom.size();
    int display = 0;
    if (size >= 4) display = 4;
    if (size < 4) display = size;
    // Iterate over each child
    for (int i = 0; i < display; i++) {
        GtkWidget* widget = GTK_WIDGET(g_list_nth_data(children, i));
        if (GTK_IS_BUTTON(widget)) {
            GtkButton* button = GTK_BUTTON(widget);

            if(!active){
                gtk_button_set_label(button, getStreetName(resultFrom[i]).c_str());
            } 
            else {
                gtk_button_set_label(button, getIntersectionName(resultFrom[i]).c_str());
            }

            //gtk_button_set_label(button, getStreetName(resultFrom[i]).c_str());
            gtk_widget_show(GTK_WIDGET(btns[i]));
            from[i] = resultFrom[i]; 
        }

    }
    
    if (display < 4)
    {
        for (int i = 3; i >= display; i--) gtk_widget_hide(GTK_WIDGET(btns[i]));
    }
    // Free the list of children
    g_list_free(children);
}


void searchAutoTo() {
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));
    const gchar* text = gtk_entry_get_text(GTK_ENTRY(searchToBar));

    std::vector<StreetIdx> resultTo;

    if (!active) {
        resultTo = findStreetIdsFromPartialStreetName(text);
    } else {
        resultTo = partialIntersections(text);
    }
    std::vector<GObject*> btns = { t0,t1,t2,t3 };

    if (to.size() != RES) to.resize(RES);
    GList* children = gtk_container_get_children(GTK_CONTAINER(t));

    int size = resultTo.size();
    int display = 0;

    if (size >= RES) display = RES;
    if (size < RES) display = size;
    // Iterate over each child
    for (int i = 0; i < display; i++) {
        GtkWidget* widget = GTK_WIDGET(g_list_nth_data(children, i));
        if (GTK_IS_BUTTON(widget)) {
            GtkButton* button = GTK_BUTTON(widget);

            if (!active)
            {
                gtk_button_set_label(button, getStreetName(resultTo[i]).c_str());
            } else {
                gtk_button_set_label(button, getIntersectionName(resultTo[i]).c_str());
            }
            
            //gtk_button_set_label(button, getStreetName(resultTo[i]).c_str());
            gtk_widget_show(GTK_WIDGET(btns[i]));
            to[i] = resultTo[i];
        }

    }
    if (display < RES) //hide the rows
    {
        for (int i = (RES - 1); i >= display; i--) gtk_widget_hide(GTK_WIDGET(btns[i]));
    }
    // Free the list of children
    g_list_free(children);
}
//search from Row 0 clicked 
void click0() {
    selectedFrom(0);
}
//search from Row 1 clicked 
void click1() {
    selectedFrom(1);
}
//search from Row 2 clicked 
void click2() {
    selectedFrom(2);
}
//search from Row 3 clicked 
void click3() {
    selectedFrom(3);
}
//This function handles the selection of the search from bar and calls the intersection fucn
void selectedFrom(int i) {
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));

    if (active){
        if (searchFirstIntersection == TRUE){
            exitDir();
        }
        if(highlight_present){
            intersections_xy[id_last_highlight].highlight=false;
            highlight_present = false;
        }

        if (streetIntersectionsPresent){
            for(int inter_num=0; inter_num<streetIntersections.size(); inter_num++){
                intersections_xy[streetIntersections[inter_num]].highlight=false;
            }
            streetIntersectionsPresent = FALSE;
        }
    }
    

    std::vector<GObject*> btns = { f0,f1,f2,f3 };

    gtk_entry_set_text(GTK_ENTRY(searchFromBar), "");

    gtk_entry_set_placeholder_text(GTK_ENTRY(searchFromBar), gtk_button_get_label(GTK_BUTTON(btns[i])));
    fromID = i;

    if (active){
        intersections_xy[from[fromID]].highlight = true;
        intersections_xy[from[fromID]].markerType = START_MARKER;
        searchFirstIntersection = TRUE;
        firstSearchHighlight = from[fromID];
    }
    
    if (fromID != -1 && toID != -1) {
        if(!active){//call vedant's functions - if !direction mode:
            highlightCommonIntersections(from[fromID], to[toID]);
        } else {
            //if the direction mode then we do nothing and wait for the user to click on find directions btn
            gtk_widget_show(GTK_WIDGET(find));
        } 
    }
    canv->redraw();
}

//search from Row 0 clicked 
void clickt0() {
    selectedTo(0);
}
//search from Row 1 clicked 
void clickt1() {
    selectedTo(1);
}
//search from Row 2 clicked 
void clickt2() {
    selectedTo(2);
}
//search from Row 3 clicked 
void clickt3() {
    selectedTo(3);
}

//This function handles the selection of the search TO bar and calls the intersection func
void selectedTo(int i) {
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));

    if(active){
            if (searchSecondIntersection == TRUE){
            exitDir();
        }
        if(highlight_present){
            intersections_xy[id_last_highlight].highlight=false;
            highlight_present = false;
        }

        if (streetIntersectionsPresent){
            for(int inter_num=0; inter_num<streetIntersections.size(); inter_num++){
                intersections_xy[streetIntersections[inter_num]].highlight=false;
            }
            streetIntersectionsPresent = FALSE;
        }
    }
    
    std::vector<GObject*> btns = { t0,t1,t2,t3 };

    gtk_entry_set_text(GTK_ENTRY(searchToBar), "");

    gtk_entry_set_placeholder_text(GTK_ENTRY(searchToBar), gtk_button_get_label(GTK_BUTTON(btns[i])));
    toID = i;

    if (active){
        intersections_xy[to[toID]].highlight = true;
        intersections_xy[to[toID]].markerType = END_MARKER;
        searchSecondIntersection = TRUE;
        secondSearchHighlight = to[toID];
    }

    //saveTo = toIDs;
    if (fromID != -1 && toID != -1) {
        if(!active){//call vedant's functions - if !direction mode:
            highlightCommonIntersections(from[fromID], to[toID]);
        } else {
            //if the direction mode then we do nothing and wait for the user to click on find directions btn 
            gtk_widget_show(GTK_WIDGET(find));
        }
    }
    canv->redraw();
}

//This func prints the POIs on the screen below a certain zoom level and its names
void printPOI(ezgl::renderer* g) {
    double zoomFactor = street_zoom_factor();

    ezgl::rectangle my_world = curr_world(g);

    //get the icons
    ezgl::surface* general = ezgl::renderer::load_png("libstreetmap/resources/icons/general.png");
    ezgl::surface* airprot = ezgl::renderer::load_png("libstreetmap/resources/icons/airport.png");
    ezgl::surface* atm = ezgl::renderer::load_png("libstreetmap/resources/icons/atm.png");
    ezgl::surface* cafe = ezgl::renderer::load_png("libstreetmap/resources/icons/cafe.png");
    ezgl::surface* church = ezgl::renderer::load_png("libstreetmap/resources/icons/church.png");
    ezgl::surface* fastfood = ezgl::renderer::load_png("libstreetmap/resources/icons/fastfood.png");
    ezgl::surface* fire = ezgl::renderer::load_png("libstreetmap/resources/icons/firestation.png");
    ezgl::surface* fuel = ezgl::renderer::load_png("libstreetmap/resources/icons/fuel.png");
    ezgl::surface* hospital = ezgl::renderer::load_png("libstreetmap/resources/icons/hospital.png");
    ezgl::surface* lib = ezgl::renderer::load_png("libstreetmap/resources/icons/library.png");
    ezgl::surface* movie = ezgl::renderer::load_png("libstreetmap/resources/icons/movie.png");
    ezgl::surface* parking = ezgl::renderer::load_png("libstreetmap/resources/icons/parking.png");
    ezgl::surface* resturant = ezgl::renderer::load_png("libstreetmap/resources/icons/resturant.png");
    ezgl::surface* school = ezgl::renderer::load_png("libstreetmap/resources/icons/school.png");
    ezgl::surface* uni = ezgl::renderer::load_png("libstreetmap/resources/icons/uni.png");
    ezgl::surface* bus = ezgl::renderer::load_png("libstreetmap/resources/icons/bus.png");
    ezgl::surface* bank = ezgl::renderer::load_png("libstreetmap/resources/icons/bank.png");
    ezgl::surface* bar = ezgl::renderer::load_png("libstreetmap/resources/icons/bar.png");
    ezgl::surface* pub = ezgl::renderer::load_png("libstreetmap/resources/icons/pub.png");
    ezgl::surface* strip = ezgl::renderer::load_png("libstreetmap/resources/icons/strip.png");
    ezgl::surface* pill = ezgl::renderer::load_png("libstreetmap/resources/icons/pill.png");
    ezgl::surface* police = ezgl::renderer::load_png("libstreetmap/resources/icons/police.png");
    

    //small rule engine to choose the icons and place the POI based on POIType
    if (zoomFactor < 0.4) {

        //hide the zoom in nudge
        setPOIWidgets(zoomFactor);
        //gtk_widget_hide(GTK_WIDGET(info));

        for (int i = 0; i < getNumPointsOfInterest(); i++)
        {
            ezgl::point2d point = POILoc[i];
            if (my_world.contains(point)) {
                std::string type = POIType[i];
                std::string name = POIName[i];
                if (type == "terminal" || type == "aerodome") {
                    if (showPOIAll)
                    {
                        g->draw_surface(airprot, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "atm") {
                    if (showPOIAll)
                    {
                        g->draw_surface(atm, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "bank") {
                    if (showPOIAll)
                    {
                        g->draw_surface(bank, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "bus_station") {
                    if (showPOIAll)
                    {
                        g->draw_surface(bus, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "cafe") {
                    if (showPOIAll || showPOIfood)
                    {
                        g->draw_surface(cafe, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "church") {
                    if (showPOIAll)
                    {
                        g->draw_surface(church, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "fast_food") {
                    if (showPOIAll || showPOIfood)
                    {
                        g->draw_surface(fastfood, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "fire_station") {
                    if (showPOIAll || showPOIemer)
                    {
                        g->draw_surface(fire, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "fuel") {
                    if (showPOIAll)
                    {
                        g->draw_surface(fuel, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "doctors" || type == "hospital" || type == "clinic") {
                    if (showPOIAll || showPOIemer)
                    {
                        g->draw_surface(hospital, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "library" || type == "books") {
                    if (showPOIAll || showPOIedu)
                    {
                        g->draw_surface(lib, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }if (type == "parking") {
                    if (showPOIAll)
                    {
                        g->draw_surface(parking, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }if (type == "restaurant" || type == "food_court") {
                    if (showPOIAll || showPOIfood)
                    {
                        g->draw_surface(resturant, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }if (type == "theatre" || type == "cinema") {
                    if (showPOIAll || showPOIent)
                    {
                        g->draw_surface(movie, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }if (type == "uni" || type == "college") {
                    if (showPOIAll || showPOIedu)
                    {
                        g->draw_surface(uni, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "school" || type == "kindergarten") {
                    if (showPOIAll || showPOIedu)
                    {
                        g->draw_surface(school, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "stripclub") {
                    if (showPOIAll || showPOIent)
                    {
                        g->draw_surface(strip, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "bar" || type == "nighclub") {
                    if (showPOIAll || showPOIent)
                    {
                        g->draw_surface(bar, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "pub") {
                    if (showPOIAll || showPOIent)
                    {
                        g->draw_surface(pub, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "pharmacy") {
                    if (showPOIAll || showPOIemer)
                    {
                        g->draw_surface(pill, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
                if (type == "police") {
                    if (showPOIAll || showPOIemer)
                    {
                        g->draw_surface(police, POILoc[i]);
                        if (zoomFactor < 0.3) {
                        printName(g, point, name);
                        }
                    }
                }
            }
        } 
    } else {
            //show the zoom in nudge
            if(isInitial){
                //gtk_widget_show(GTK_WIDGET(info));
                setPOIWidgets(zoomFactor);
            }
                
    }

    //free the icons
    ezgl::renderer::free_surface(general);
    ezgl::renderer::free_surface(airprot);
    ezgl::renderer::free_surface(atm);
    ezgl::renderer::free_surface(cafe);
    ezgl::renderer::free_surface(church);
    ezgl::renderer::free_surface(fastfood);
    ezgl::renderer::free_surface(fire);
    ezgl::renderer::free_surface(fuel);
    ezgl::renderer::free_surface(hospital);
    ezgl::renderer::free_surface(lib);
    ezgl::renderer::free_surface(movie);
    ezgl::renderer::free_surface(parking);
    ezgl::renderer::free_surface(resturant);
    ezgl::renderer::free_surface(school);
    ezgl::renderer::free_surface(uni);
    ezgl::renderer::free_surface(bus);
    ezgl::renderer::free_surface(bank);
    ezgl::renderer::free_surface(bar);
    ezgl::renderer::free_surface(pub);
    ezgl::renderer::free_surface(strip);
    ezgl::renderer::free_surface(pill);
    ezgl::renderer::free_surface(police);

}

//This is the helper function that prints POI names
void printName(ezgl::renderer* g, ezgl::point2d point, std::string name) {
    g->set_color(ezgl::BLACK);
    g->set_text_rotation(0);
    g->set_font_size(10);
    double zoomFactor = street_zoom_factor();
    int adj = 0;
    if(zoomFactor < 0.15 ) {
        adj = 2;
    } else if (zoomFactor < 0.2){
        adj = 3;
    } else if (zoomFactor < 0.3){
        adj = 5;
    }
    g->draw_text({ (point.x),(point.y - adj) }, name);
}

void showAll(){
    if(showPOIAll == TRUE) {
        showPOIAll = FALSE;
        showPOIedu = FALSE;
        showPOIemer = FALSE;
        showPOIent = FALSE;
        showPOIfood = FALSE;
    }
    else showPOIAll = TRUE;
    canv->redraw();
}

void showFood(){
    if(showPOIfood == TRUE) showPOIfood = FALSE;
    else showPOIfood = TRUE;
    canv->redraw();
}

void showEnt(){
    if(showPOIent == TRUE) showPOIent = FALSE;
    else showPOIent = TRUE;
    canv->redraw();
}

void showEmer(){
    if(showPOIemer == TRUE) showPOIemer = FALSE;
    else showPOIemer = TRUE;
    canv->redraw();
}

void showEdu(){
    if(showPOIedu == TRUE) showPOIedu = FALSE;
    else showPOIedu = TRUE;
    canv->redraw();
}

void setPOIWidgets(double zoom){
    if (zoom < 0.4)
    {
        gtk_label_set_text(GTK_LABEL(info), "Toggle For POIs");
        gtk_widget_show(GTK_WIDGET(food));
        gtk_widget_show(GTK_WIDGET(all));
        gtk_widget_show(GTK_WIDGET(ent));
        gtk_widget_show(GTK_WIDGET(emer));
        gtk_widget_show(GTK_WIDGET(edu));
    } else {
        gtk_label_set_text(GTK_LABEL(info), "Zoom to see POIs");
        gtk_widget_hide(GTK_WIDGET(food));
        gtk_widget_hide(GTK_WIDGET(all));
        gtk_widget_hide(GTK_WIDGET(ent));
        gtk_widget_hide(GTK_WIDGET(emer));
        gtk_widget_hide(GTK_WIDGET(edu));
    }
    
}

//This PROPREITRY fucntions adds the CSS styling
//Note to TA: This is a SUPER feat dont you think?

void addStyle(ezgl::application* application) {
    // Load the CSS provider and stylesheet
    GtkCssProvider* css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "libstreetmap/src/style.css", NULL);

    //Add the context
    addstylecontext(application, "ZoomInButton", css_provider);
    addstylecontext(application, "ZoomOutButton", css_provider);
    addstylecontext(application, "ZoomFitButton", css_provider);
    addstylecontext(application, "bottomLabel", css_provider);
    addstylecontext(application, "dirpane", css_provider);
    addstylecontext(application, "f", css_provider);
    addstylecontext(application, "f0", css_provider);
    addstylecontext(application, "f1", css_provider);
    addstylecontext(application, "f2", css_provider);
    addstylecontext(application, "f3", css_provider);
    addstylecontext(application, "t", css_provider);
    addstylecontext(application, "t0", css_provider);
    addstylecontext(application, "t1", css_provider);
    addstylecontext(application, "t2", css_provider);
    addstylecontext(application, "t3", css_provider);
    addstylecontext(application, "searchBarFrom", css_provider);
    addstylecontext(application, "cityDropDown", css_provider);
    addstylecontext(application, "searchBarTo", css_provider);
    addstylecontext(application, "info", css_provider);
    addstylecontext(application, "all", css_provider);
    addstylecontext(application, "food", css_provider);
    addstylecontext(application, "ent", css_provider);
    addstylecontext(application, "emer", css_provider);
    addstylecontext(application, "edu", css_provider);
    addstylecontext(application, "dirlabel", css_provider);
    addstylecontext(application, "tutbox", css_provider);
    addstylecontext(application, "tutbtn", css_provider);
    addstylecontext(application, "help", css_provider);
    addstylecontext(application, "find", css_provider);
    addstylecontext(application, "hidepane", css_provider);
    addstylecontext(application, "exitdir", css_provider);
    addstylecontext(application, "routerow", css_provider);
    addstylecontext(application, "showdirpane", css_provider);
    addstylecontext(application, "dirpanebox", css_provider);
    addstylecontext(application, "iconbox", css_provider);
}

//This helper functions adds the css provider for any widget
void addstylecontext(ezgl::application* application, const gchar* widgetID, GtkCssProvider* css) {
    GObject* widget = application->get_object(widgetID);
    //get style context
    GtkStyleContext* newContext = gtk_widget_get_style_context(GTK_WIDGET(widget));
    gtk_style_context_add_provider(newContext, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

//Adds rows for the directions pane and updates all the required information
void updateDirectionInfo(std::vector<int> turn, std::vector<StreetIdx> streetsIDs, std::vector<double> dist) {
    // Clear the list box
    gtk_container_foreach(GTK_CONTAINER(GTK_WIDGET(listbox)), (GtkCallback) destroy_callback, NULL);

    //make the first gap
    GtkWidget *row_box_gap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box_gap, -1);

    GtkWidget *row_box_first = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    //handle the first node
    const std::string label_text_first = "Head " 
    + directions[turn[0]] + " onto " + getStreetName(streetsIDs[0]);

    GtkWidget *label_first = gtk_label_new(label_text_first.c_str());

    gtk_box_pack_end(GTK_BOX(row_box_first), label_first, TRUE, TRUE, 0);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box_first, -1);
    
    std::string str2 = toString(dist[0], 2);
    addDistanceRow(str2);
    // Iterate over the indices and add each row to the list box start from the second position
    for (std::size_t i = 1; i < (turn.size()); ++i) {
        // Create a box to hold the icon and label
        GtkWidget *row_box;
        GtkWidget *icon;
        GtkWidget *label;
        //need to update with direction types
        if (turn[i] == 0) //turn right
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/r.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Turn right onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());
            

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);


        } else if (turn[i] == 1) //turn left
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/l.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Turn left onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);
            gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);

        } else if (turn[i] == 2) //slide right
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/str.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Slide right onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);

        } else if (turn[i] == 3) //slide left
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/stl.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Slide left onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);
            
        } else if (turn[i] == 4) //sharp right
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/sr.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Take a sharp right onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);

        } else if (turn[i] == 5) //sharp left
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/sl.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Take a sharp left onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);

        } else if (turn[i] == 6) //straignt
        {
            row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            // Get the icon for this row
            icon = gtk_image_new_from_file("libstreetmap/resources/icons/ss.png");
            // Get the label text for this row
            std::string str = toString(dist[i], 2);
            const std::string label_text = "Continue Straight onto " + getStreetName(streetsIDs[i]);
            label = gtk_label_new(label_text.c_str());

            //set the margins
            gtk_widget_set_margin_start(GTK_WIDGET(icon), 5);
            gtk_widget_set_margin_end(GTK_WIDGET(label), 5);

            // Pack the icon and label into the row box
            gtk_box_pack_start(GTK_BOX(row_box), icon, FALSE, FALSE, 0);
            gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
            // Add the row box to the list box
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);

            addDistanceRow(str);
        }
        
    }

    //Add a gap row empty
    GtkWidget *row_box_gap_last = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box_gap_last, -1);

    GtkWidget *row_box_last = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

    //handle the last node
    const std::string label_text_last = "You have arrived! Yay!";

    GtkWidget *label_last = gtk_label_new(label_text_last.c_str());

    gtk_box_pack_end(GTK_BOX(row_box_last), label_last, TRUE, TRUE, 0);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box_last, -1);

    // Show the list box
    gtk_widget_show_all(GTK_WIDGET(listbox));
    gtk_widget_show(GTK_WIDGET(route));
}

//destroys a widget
void destroy_callback(GtkWidget* widget, void*) {
    gtk_widget_destroy(widget);
}

//Intersections name for partial search in the exact same wqay as the partial function for streets in M1
std::vector<IntersectionIdx> partialIntersections(std::string prefix){
    std::string prefixNoSpaces; 
    std::vector<IntersectionIdx> toReturn; 
    if(prefix == ""){
        return toReturn; //Empty intersection prefix hence we return an empty vector
    }
    std::remove_copy(
        prefix.begin(), 
        prefix.end(), 
        std::back_inserter(prefixNoSpaces), 
        ' '
    );
    std::transform(prefixNoSpaces.begin(), prefixNoSpaces.end(), prefixNoSpaces.begin(), ::tolower);
    int numCheckChars = prefixNoSpaces.size();
    int key1 = key(prefixNoSpaces[0]); //Get the assigned key for the first character of the prefix
    int key2 = key(prefixNoSpaces[1]); //Get the assigned key for the second character of the prefix
    if (key1 != -1 && prefixNoSpaces[1] == '\0'){ //A single character input
        return sameFirstLetterIntersection[key1]; //return the vector with the Idx of all intersection with the same initial
    }
    if (key1 != -1 && key2 != -1){ //first two characters are ONLY an alphabet or digit
        for (int i = 0; i < sameFirstTwoLettersIntersections[key1][key2].size(); i++){ 
            //filter through the remaining St IDs by matching the prefix string with the intersection name
            //abc vector of vector of vector<strings> holds the name of intersection in the same index as where the
            //sameFirstTwoLettersStreet holds the matching ID. So the the ID and name of one intersection have the same Idx
            std::string stNameNoSpaces = abcI[key1][key2][i]; 
            if(prefixNoSpaces.compare(0, numCheckChars, stNameNoSpaces, 0, numCheckChars) == 0){
                toReturn.push_back(sameFirstTwoLettersIntersections[key1][key2][i]); //add to the retuning vector
            }
        }
    }   
    return toReturn;
}

//Key func for the intersection preloader
int key(char input) {
    if (isdigit(input)) {
        return input - 22;
    } else if (isalpha(input)) {
        return input - 'a';
    } else {
        return -1;
    }
}

//Callback function for the direction mode toggle btn and resets the search bars for a new search
void directionMode(){
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));
    if (!active)
    {
        gtk_entry_set_placeholder_text(GTK_ENTRY(searchFromBar), "From Street");
        gtk_entry_set_placeholder_text(GTK_ENTRY(searchToBar), "To Street");
        fromID = -1;
        toID = -1;
        from.clear();
        to.clear();
        from.resize(4);
        to.resize(4);
        gtk_widget_hide(GTK_WIDGET(find));
        gtk_widget_hide(GTK_WIDGET(bottomLabel));
    } else
    {
        gtk_entry_set_placeholder_text(GTK_ENTRY(searchFromBar), "From Intersection");
        gtk_entry_set_placeholder_text(GTK_ENTRY(searchToBar), "To Intersection");
        fromID = -1;
        toID = -1;
        from.clear();
        to.clear();
        from.resize(4);
        to.resize(4);
        gtk_widget_hide(GTK_WIDGET(find));
        gtk_widget_hide(GTK_WIDGET(bottomLabel));
        
    }
    
    
}

//callback function for closing tutorial window
void closeTut(){
    gtk_widget_hide(GTK_WIDGET(tutview));
}

//callback function for showing tutorial window
void showTut(){
    gtk_widget_show(GTK_WIDGET(tutview));
}

//callback function for hiding direction window
void hideDir(){
    gtk_widget_hide(GTK_WIDGET(route));
    gtk_widget_show(GTK_WIDGET(showdirpane));
}

//callback function for exiting direction window and reseting for a new search
void exitDir(){
    gtk_widget_hide(GTK_WIDGET(route));
    gtk_widget_hide(GTK_WIDGET(showdirpane));
    gtk_widget_hide(GTK_WIDGET(find));
    directionMode();
    clearMarkers();

    //Vedant and Dev please use this function to clear out any paths that
    //were drawn on the map and redraw the map. 
    globalPath.clear();
    canv->redraw();
}

//callback function for showing direction window 
void reShowDir(){
    gtk_widget_show(GTK_WIDGET(route));
    gtk_widget_hide(GTK_WIDGET(showdirpane));   
}

//resets from search bar backend
void setFromInfo(IntersectionIdx ID){
    from.clear();
    from.resize(4);
    from[0] = ID;
    fromID = 0;
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));
    if (fromID != -1 && toID != -1 && active){
        gtk_widget_show(GTK_WIDGET(find));
    }
}

//resets to search bar backend
void setToInfo(IntersectionIdx ID){
    to.clear();
    to.resize(4);
    to[0] = ID;
    toID = 0;
    bool active = gtk_switch_get_state(GTK_SWITCH(dir));
    if (fromID != -1 && toID != -1 && active){
        gtk_widget_show(GTK_WIDGET(find));
    }
}

//This function calls all the other gunctions required to show the 
//directions on the screen
void displayPathFind(){
    std::vector<int> turn;
    std::vector<double> dist;
    std::vector<StreetIdx> street;
    std::vector<StreetSegmentIdx> segs;
    segs = findPathBetweenIntersections({from[fromID], to[toID]}, 15.0);
    if (segs.size() >0){
        
        double travelTime = computePathTravelTime(segs, 15.0); //in seconds 

        std::string intersectionString;
        int travelTimeInt = static_cast<int>(travelTime);
        intersectionString = returnTimeString(travelTimeInt);

        turn = directionsBetweenStreets(segs,to[toID]);
        street = streetIdPath(segs);
        dist = streetDistances(segs);
        updateDirectionInfo(turn, street, dist);

        gtk_label_set_text(GTK_LABEL(bottomLabel), intersectionString.c_str());
        gtk_widget_show(GTK_WIDGET(bottomLabel));

    } else {

        std::string intersectionString = "Retry - No Valid Path Found";
        //maybe clear any markers? 
        exitDir();
        gtk_label_set_text(GTK_LABEL(bottomLabel), intersectionString.c_str());
        gtk_widget_show(GTK_WIDGET(bottomLabel));
    }
    canv->redraw();
}

//Utility function to reduce a double to a str with 2dp
std::string toString(double distance, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision);
    ss << distance;
    return ss.str();
}

//Adds the row in the directions details pane with details about travel distance as a row_box with a label
void addDistanceRow(std::string dist){
    GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    const std::string label_text = "Travel for " + dist + "m";

    GtkWidget *label = gtk_label_new(label_text.c_str());

    gtk_box_pack_end(GTK_BOX(row_box), label, TRUE, TRUE, 0);
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);
}

//returns the string to display about how much time a journey wold take
std::string returnTimeString(int travelTime){
        std::string initialString = "Path displayed! Travel time is ";
        std::string minuteString = " minute";
        std::string secondString = " seconds";
        std::string hourString = " hour";
        std::string intersectionString;
        if(travelTime<60){
            std::string timeSeconds = std::to_string(travelTime);
            intersectionString = initialString+timeSeconds+secondString;

        }
        else if(travelTime>=60 && travelTime<3600){
            std::string timeMinutes = std::to_string(travelTime/60);
            intersectionString = initialString+timeMinutes+minuteString;
            if((travelTime/60)>1) intersectionString +="s";

        }
        else{
            std::string numHours = std::to_string(travelTime/3600);
            std::string numMinutes = std::to_string((travelTime%3600)/60);
            if((travelTime/3600)==1) hourString+=" ";
            if((travelTime/3600)>1) hourString+="s ";
            if(((travelTime%3600)/60)>1) minuteString+="s ";

            intersectionString = initialString+numHours+hourString
                    +numMinutes+minuteString;
        }

        return intersectionString;
}