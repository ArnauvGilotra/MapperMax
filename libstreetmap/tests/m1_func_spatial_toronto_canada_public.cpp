
#include "m1.h"
#include "unit_test_util.h"

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include <UnitTest++/UnitTest++.h>

#include <random>
#include <algorithm>
#include <set>

using ece297test::relative_error;
using ece297test::sorted;

SUITE(spatial_queries_public_toronto_canada) {

    struct BaseMapFixture {
        BaseMapFixture() {
            //Load the map
            try {
                loadMap("/cad2/ece297s/public/maps/saint-helena.streets.bin");
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test setup: loadMap threw an exceptinon !!!!" << std::endl;
                throw; // re-throw exceptinon
            }
        }
    
        ~BaseMapFixture() {
            //Clean-up
            try {
                closeMap();
            } catch (const std::exception& e) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon. what(): " << e.what() << " !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exceptinon !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            }
        }
    };


    struct MapFixture : BaseMapFixture {};

    TEST_FIXTURE(MapFixture, closest_poi) {
        std::set<POIIdx> valid_results;

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.99930572509765625, -5.76324987411499023), "grave_yard"));

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.96873569488525391, -5.75373744964599609), "grave_yard"));

        ECE297_CHECK_EQUAL(1, findClosestPOI(LatLon(-15.92393589019775391, -5.74595260620117188), "grave_yard"));

        ECE297_CHECK_EQUAL(10, findClosestPOI(LatLon(-15.97830486297607422, -5.74973154067993164), "restaurant"));

        ECE297_CHECK_EQUAL(10, findClosestPOI(LatLon(-15.96715641021728516, -5.68612766265869141), "restaurant"));

        ECE297_CHECK_EQUAL(10, findClosestPOI(LatLon(-15.94857788085937500, -5.68364572525024414), "restaurant"));

        ECE297_CHECK_EQUAL(10, findClosestPOI(LatLon(-15.93003654479980469, -5.67341279983520508), "restaurant"));

        ECE297_CHECK_EQUAL(11, findClosestPOI(LatLon(-15.94714546203613281, -5.75587797164916992), "school"));

        ECE297_CHECK_EQUAL(11, findClosestPOI(LatLon(-15.92223834991455078, -5.74491453170776367), "school"));

    } //closest_poi

} //spatial_queries_public_toronto_canada

