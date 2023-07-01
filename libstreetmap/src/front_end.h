#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m2.h"
#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "globals.h"
#include <string>
#include <sstream>
#include <iomanip>

void draw_features(ezgl::renderer *g);
void setCol(int i, ezgl::renderer *g);
void searchAuto();
void searchAutoTo();
void selectedFrom(int i);
void selectedTo(int i);
void click0();
void click1();
void click2();
void click3();
void clickt0();
void clickt1();
void clickt2();
void clickt3();
void printPOI (ezgl::renderer *g);
void printName(ezgl::renderer *g, ezgl::point2d point, std::string name);
void showAll();
void showFood();
void showEnt();
void showEmer();
void showEdu();
void setPOIWidgets(double zoom);
void addStyle(ezgl::application* application);
void addstylecontext(ezgl::application* application, const gchar * widgetID, GtkCssProvider* css);
void updateDirectionInfo(std::vector<int> turn, std::vector<StreetIdx> streetsIDs, std::vector<double> dist);
void destroy_callback(GtkWidget* widget, void*);
std::vector<IntersectionIdx> partialIntersections(std::string prefix);
int key(char input);
void directionMode();
void closeTut();
void showTut();
void hideDir();
void exitDir();
void reShowDir();
void setFromInfo(IntersectionIdx ID);
void setToInfo(IntersectionIdx ID);
void displayPathFind();
std::string toString(double distance, int precision = 6);
void addDistanceRow(std::string dist);
std::string returnTimeString(int travelTime);