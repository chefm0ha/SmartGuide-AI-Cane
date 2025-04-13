/*
 * Navigation.h
 * 
 * Handles GPS and navigation functionality
 */

 #ifndef NAVIGATION_H
 #define NAVIGATION_H
 
 #include <Arduino.h>
 #include <TinyGPS++.h>
 
 // Waypoint structure
 struct Waypoint {
   float lat;
   float lng;
   String name;
   String type;
   unsigned long timestamp;
 };
 
 class NavigationSystem {
   private:
     // GPS instance
     TinyGPSPlus gps;
     
     // Current location data
     float currentLat;
     float currentLng;
     float currentHeading;
     bool hasValidFix;
     
     // Destination data
     float destLat;
     float destLng;
     String destName;
     bool destinationSet;
     bool isActivelyNavigating;
     
     // Waypoints storage
     static const int MAX_WAYPOINTS = 30;
     Waypoint waypoints[MAX_WAYPOINTS];
     int waypointCount;
     
     // Navigation status
     int currentDirection; // -180 to +180 degrees
     float distanceToDestination;
     bool newInstructionAvailable;
     bool destinationReached;
     
     // Serial connection for GPS module
     Stream* gpsSerial;
     
     // Helper methods
     float calculateDistance(float lat1, float lon1, float lat2, float lon2);
     float calculateBearing(float lat1, float lon1, float lat2, float lon2);
     String bearingToDirection(float bearing);
     void updateNavigationStatus();
     
   public:
     NavigationSystem();
     
     // Initialization
     void begin();
     void setGpsSerial(Stream* serial) { gpsSerial = serial; }
     
     // GPS update
     bool updateGpsLocation();
     
     // Waypoint management
     bool setWaypoint(String name, String type);
     Waypoint* getWaypoint(String name);
     Waypoint* getWaypointAt(int index);
     int getWaypointCount() { return waypointCount; }
     bool deleteWaypoint(String name);
     void clearAllWaypoints();
     
     // Navigation control
     bool setDestination(String waypointName);
     bool startNavigation();
     void stopNavigation();
     String getNextInstruction();
     
     // Status getters
     float getCurrentLat() { return currentLat; }
     float getCurrentLng() { return currentLng; }
     float getDestLat() { return destLat; }
     float getDestLng() { return destLng; }
     String getDestName() { return destName; }
     bool hasValidGpsFix() { return hasValidFix; }
     bool isNavigating() { return isActivelyNavigating; }
     int getCurrentDirection() { return currentDirection; }
     float getDistanceToDestination() { return distanceToDestination; }
     bool isNewInstruction() { 
       bool result = newInstructionAvailable;
       newInstructionAvailable = false;
       return result;
     }
     bool isDestinationReached() { return destinationReached; }
 };
 
 #endif