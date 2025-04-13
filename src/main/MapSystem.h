/*
 * MapSystem.h
 * 
 * Handles mapping and spatial memory features
 */

 #ifndef MAP_SYSTEM_H
 #define MAP_SYSTEM_H
 
 #include <Arduino.h>
 #include <SD.h>
 #include <ArduinoJson.h>
 
 // Maximum number of map nodes and edges
 #define MAX_MAP_NODES 500
 #define MAX_MAP_EDGES 1000
 
 // Node structure for map
 struct MapNode {
   String id;
   float lat;
   float lng;
   String type;
   bool isObstacle;
   unsigned long lastSeen;
   int visitCount;
 };
 
 // Edge structure for map
 struct MapEdge {
   String id;
   String sourceId;
   String targetId;
   float weight;
   unsigned long lastTraversed;
   int traverseCount;
 };
 
 class MapSystem {
   private:
     // Map data storage
     MapNode nodes[MAX_MAP_NODES];
     MapEdge edges[MAX_MAP_EDGES];
     int nodeCount;
     int edgeCount;
     
     // Current location and movement tracking
     float prevLat;
     float prevLng;
     float currentLat;
     float currentLng;
     bool isFirstPosition;
     String currentNodeId;
     
     // SD card file handling
     File mapFile;
     bool sdAvailable;
     
     // Helper methods
     String generateNodeId();
     String generateEdgeId();
     int findNodeIndex(String nodeId);
     int findNearestNodeIndex(float lat, float lng, float maxDistance);
     float calculateDistance(float lat1, float lon1, float lat2, float lon2);
     void saveMapToSD();
     bool loadMapFromSD();
     
   public:
     MapSystem();
     
     // Initialization
     bool begin();
     
     // Map management
     void updateCurrentPosition(float lat, float lng);
     void addObstacle(float lat, float lng, String type);
     void addLandmark(float lat, float lng, String type, String name);
     
     // Path finding
     bool findPath(float startLat, float startLng, float endLat, float endLng);
     bool getNextPathNode(float &lat, float &lng);
     
     // Map information
     int getNodeCount() { return nodeCount; }
     int getEdgeCount() { return edgeCount; }
     bool isObstacleNearby(float lat, float lng, float radius);
     String getAreaType(float lat, float lng, float radius);
     
     // Map persistence
     bool saveMap();
     bool loadMap();
     void clearMap();
 };
 
 #endif