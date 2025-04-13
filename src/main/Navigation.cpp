/*
 * Navigation.cpp
 * 
 * Implementation of GPS-based navigation system
 */

 #include "Navigation.h"
 #include <Arduino.h>
 
 // Constants
 #define EARTH_RADIUS_M 6371000.0
 #define DEG_TO_RAD 0.017453292519943295 // PI/180
 #define RAD_TO_DEG 57.29577951308232    // 180/PI
 #define DESTINATION_REACHED_THRESHOLD 10.0 // meters
 
 NavigationSystem::NavigationSystem() {
   // Initialize location variables
   currentLat = 0.0;
   currentLng = 0.0;
   currentHeading = 0.0;
   hasValidFix = false;
   
   // Initialize navigation variables
   destLat = 0.0;
   destLng = 0.0;
   destName = "";
   destinationSet = false;
   isActivelyNavigating = false;
   
   // Initialize waypoints
   waypointCount = 0;
   
   // Initialize navigation status
   currentDirection = 0;
   distanceToDestination = 0.0;
   newInstructionAvailable = false;
   destinationReached = false;
 }
 
 void NavigationSystem::begin() {
   // Initialize GPS (assuming Serial1 for GPS module)
   gpsSerial = &Serial1;
   Serial1.begin(9600);
   
   Serial.println("GPS Navigation System initialized");
 }
 
 bool NavigationSystem::updateGpsLocation() {
   // Read data from GPS
   while (gpsSerial->available() > 0) {
     gps.encode(gpsSerial->read());
   }
   
   // Check if we have new position data
   if (gps.location.isUpdated()) {
     currentLat = gps.location.lat();
     currentLng = gps.location.lng();
     
     // Get heading if moving (speed > 1 km/h)
     if (gps.speed.kmph() > 1.0) {
       currentHeading = gps.course.deg();
     }
     
     hasValidFix = true;
     
     // If actively navigating, update navigation status
     if (isActivelyNavigating) {
       updateNavigationStatus();
     }
     
     return true;
   }
   
   // If no data for 5 seconds, consider fix lost
   if (millis() > 5000 && gps.charsProcessed() < 10) {
     hasValidFix = false;
   }
   
   return false;
 }
 
 bool NavigationSystem::setWaypoint(String name, String type) {
   // Check if we have a valid GPS fix
   if (!hasValidFix) {
     return false;
   }
   
   // Check if we reached max waypoints
   if (waypointCount >= MAX_WAYPOINTS) {
     return false;
   }
   
   // Check if waypoint name already exists
   for (int i = 0; i < waypointCount; i++) {
     if (waypoints[i].name.equalsIgnoreCase(name)) {
       // Update existing waypoint
       waypoints[i].lat = currentLat;
       waypoints[i].lng = currentLng;
       waypoints[i].type = type;
       waypoints[i].timestamp = millis();
       return true;
     }
   }
   
   // Create new waypoint
   Waypoint newPoint;
   newPoint.lat = currentLat;
   newPoint.lng = currentLng;
   newPoint.name = name;
   newPoint.type = type;
   newPoint.timestamp = millis();
   
   // Add to array
   waypoints[waypointCount] = newPoint;
   waypointCount++;
   
   return true;
 }
 
 Waypoint* NavigationSystem::getWaypoint(String name) {
   for (int i = 0; i < waypointCount; i++) {
     if (waypoints[i].name.equalsIgnoreCase(name)) {
       return &waypoints[i];
     }
   }
   return NULL;
 }
 
 Waypoint* NavigationSystem::getWaypointAt(int index) {
   if (index >= 0 && index < waypointCount) {
     return &waypoints[index];
   }
   return NULL;
 }
 
 bool NavigationSystem::deleteWaypoint(String name) {
   for (int i = 0; i < waypointCount; i++) {
     if (waypoints[i].name.equalsIgnoreCase(name)) {
       // Remove by shifting all elements
       for (int j = i; j < waypointCount - 1; j++) {
         waypoints[j] = waypoints[j + 1];
       }
       waypointCount--;
       return true;
     }
   }
   return false;
 }
 
 void NavigationSystem::clearAllWaypoints() {
   waypointCount = 0;
 }
 
 bool NavigationSystem::setDestination(String waypointName) {
   Waypoint* wp = getWaypoint(waypointName);
   if (wp == NULL) {
     return false;
   }
   
   destLat = wp->lat;
   destLng = wp->lng;
   destName = wp->name;
   destinationSet = true;
   
   return true;
 }
 
 bool NavigationSystem::startNavigation() {
   if (!destinationSet || !hasValidFix) {
     return false;
   }
   
   isActivelyNavigating = true;
   newInstructionAvailable = true;
   destinationReached = false;
   
   // Initial navigation calculation
   updateNavigationStatus();
   
   return true;
 }
 
 void NavigationSystem::stopNavigation() {
   isActivelyNavigating = false;
 }
 
 void NavigationSystem::updateNavigationStatus() {
   // Calculate distance to destination
   distanceToDestination = calculateDistance(currentLat, currentLng, destLat, destLng);
   
   // Check if destination reached
   if (distanceToDestination < DESTINATION_REACHED_THRESHOLD) {
     destinationReached = true;
     isActivelyNavigating = false;
     return;
   }
   
   // Calculate bearing to destination
   float bearing = calculateBearing(currentLat, currentLng, destLat, destLng);
   
   // Calculate relative direction (-180 to +180 degrees)
   // Positive: destination is to the right
   // Negative: destination is to the left
   float relativeDirection = bearing - currentHeading;
   
   // Normalize to -180 to +180
   while (relativeDirection > 180) relativeDirection -= 360;
   while (relativeDirection <= -180) relativeDirection += 360;
   
   // Update current direction
   currentDirection = round(relativeDirection);
   
   // Determine if we need a new instruction
   static int lastDirection = 999; // Invalid value to force first instruction
   
   if (abs(currentDirection - lastDirection) > 30 || 
       (lastDirection == 999)) {
     newInstructionAvailable = true;
     lastDirection = currentDirection;
   }
 }
 
 String NavigationSystem::getNextInstruction() {
   if (!isActivelyNavigating) {
     return "Navigation not active";
   }
   
   // Create instruction based on current direction and distance
   String instruction = "";
   
   // Direction guidance
   if (abs(currentDirection) <= 15) {
     instruction = "Continue straight";
   } else if (currentDirection > 15 && currentDirection <= 45) {
     instruction = "Slight right";
   } else if (currentDirection > 45 && currentDirection <= 135) {
     instruction = "Turn right";
   } else if (currentDirection > 135) {
     instruction = "Turn around";
   } else if (currentDirection < -15 && currentDirection >= -45) {
     instruction = "Slight left";
   } else if (currentDirection < -45 && currentDirection >= -135) {
     instruction = "Turn left";
   } else if (currentDirection < -135) {
     instruction = "Turn around";
   }
   
   // Add distance information
   if (distanceToDestination < 100) {
     // For distances under 100m, specify in meters
     instruction += String(", ") + String((int)distanceToDestination) + " meters to " + destName;
   } else {
     // For larger distances, use kilometers with one decimal place
     float distKm = distanceToDestination / 1000.0;
     instruction += String(", ") + String(distKm, 1) + " kilometers to " + destName;
   }
   
   return instruction;
 }
 
 float NavigationSystem::calculateDistance(float lat1, float lon1, float lat2, float lon2) {
   // Implementation of haversine formula
   float latRad1 = lat1 * DEG_TO_RAD;
   float latRad2 = lat2 * DEG_TO_RAD;
   float lonRad1 = lon1 * DEG_TO_RAD;
   float lonRad2 = lon2 * DEG_TO_RAD;
   
   float deltaLat = latRad2 - latRad1;
   float deltaLon = lonRad2 - lonRad1;
   
   float a = sin(deltaLat/2) * sin(deltaLat/2) +
             cos(latRad1) * cos(latRad2) * 
             sin(deltaLon/2) * sin(deltaLon/2);
   float c = 2 * atan2(sqrt(a), sqrt(1-a));
   
   return EARTH_RADIUS_M * c;
 }
 
 float NavigationSystem::calculateBearing(float lat1, float lon1, float lat2, float lon2) {
   float latRad1 = lat1 * DEG_TO_RAD;
   float latRad2 = lat2 * DEG_TO_RAD;
   float lonRad1 = lon1 * DEG_TO_RAD;
   float lonRad2 = lon2 * DEG_TO_RAD;
   
   float y = sin(lonRad2 - lonRad1) * cos(latRad2);
   float x = cos(latRad1) * sin(latRad2) -
             sin(latRad1) * cos(latRad2) * cos(lonRad2 - lonRad1);
   
   float bearing = atan2(y, x) * RAD_TO_DEG;
   
   // Normalize to 0-360
   return fmod((bearing + 360.0), 360.0);
 }
 
 String NavigationSystem::bearingToDirection(float bearing) {
   const char* directions[] = {"North", "Northeast", "East", "Southeast", 
                               "South", "Southwest", "West", "Northwest"};
   int index = round(bearing / 45.0) % 8;
   return directions[index];
 }