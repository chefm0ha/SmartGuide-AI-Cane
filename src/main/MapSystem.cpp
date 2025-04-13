/*
 * MapSystem.cpp
 * 
 * Implementation of the mapping and spatial memory system
 */

 #include "MapSystem.h"
 #include <Arduino.h>
 #include <SD.h>
 #include <ArduinoJson.h>
 
 // Constants
 #define EARTH_RADIUS_M 6371000.0
 #define DEG_TO_RAD 0.017453292519943295 // PI/180
 #define MAP_FILENAME "/map_data.json"
 #define NODE_PROXIMITY_THRESHOLD 5.0  // meters
 #define PATH_NODE_DISTANCE 10.0       // meters
 
 MapSystem::MapSystem() {
   nodeCount = 0;
   edgeCount = 0;
   isFirstPosition = true;
   currentNodeId = "";
   sdAvailable = false;
 }
 
 bool MapSystem::begin() {
   // Initialize SD card
   if (!SD.begin(4)) { // Assuming SD card CS pin is 4
     Serial.println("SD card initialization failed.");
     sdAvailable = false;
     // We'll continue without persistence
   } else {
     sdAvailable = true;
     // Try to load existing map
     loadMap();
   }
   
   Serial.println("Map System initialized with " + String(nodeCount) + " nodes and " + 
                 String(edgeCount) + " edges");
   return true;
 }
 
 void MapSystem::updateCurrentPosition(float lat, float lng) {
   // Store previous position
   if (!isFirstPosition) {
     prevLat = currentLat;
     prevLng = currentLng;
   }
   
   // Update current position
   currentLat = lat;
   currentLng = lng;
   
   // If first position, just record it
   if (isFirstPosition) {
     isFirstPosition = false;
     
     // Create initial node
     MapNode node;
     node.id = generateNodeId();
     node.lat = lat;
     node.lng = lng;
     node.type = "path";
     node.isObstacle = false;
     node.lastSeen = millis();
     node.visitCount = 1;
     
     if (nodeCount < MAX_MAP_NODES) {
       nodes[nodeCount] = node;
       currentNodeId = node.id;
       nodeCount++;
     }
     
     return;
   }
   
   // Check if we're still near the current node
   int currentNodeIndex = findNodeIndex(currentNodeId);
   if (currentNodeIndex >= 0) {
     float distToCurrent = calculateDistance(lat, lng, 
                                           nodes[currentNodeIndex].lat, 
                                           nodes[currentNodeIndex].lng);
     
     if (distToCurrent <= NODE_PROXIMITY_THRESHOLD) {
       // Still at the same node, just update last seen and visit count
       nodes[currentNodeIndex].lastSeen = millis();
       nodes[currentNodeIndex].visitCount++;
       return;
     }
   }
   
   // We've moved away from the current node
   // Check if we're near an existing node
   int nearestNodeIndex = findNearestNodeIndex(lat, lng, NODE_PROXIMITY_THRESHOLD);
   
   if (nearestNodeIndex >= 0) {
     // We're near an existing node
     // Update node
     nodes[nearestNodeIndex].lastSeen = millis();
     nodes[nearestNodeIndex].visitCount++;
     
     // Create edge between current node and this node if it doesn't exist
     if (currentNodeIndex >= 0) {
       // Check if edge already exists
       bool edgeExists = false;
       for (int i = 0; i < edgeCount; i++) {
         if ((edges[i].sourceId == currentNodeId && 
              edges[i].targetId == nodes[nearestNodeIndex].id) ||
             (edges[i].sourceId == nodes[nearestNodeIndex].id && 
              edges[i].targetId == currentNodeId)) {
           // Edge exists, update it
           edges[i].lastTraversed = millis();
           edges[i].traverseCount++;
           edgeExists = true;
           break;
         }
       }
       
       // Create new edge if it doesn't exist
       if (!edgeExists && edgeCount < MAX_MAP_EDGES) {
         MapEdge edge;
         edge.id = generateEdgeId();
         edge.sourceId = currentNodeId;
         edge.targetId = nodes[nearestNodeIndex].id;
         edge.weight = calculateDistance(nodes[currentNodeIndex].lat, nodes[currentNodeIndex].lng,
                                        nodes[nearestNodeIndex].lat, nodes[nearestNodeIndex].lng);
         edge.lastTraversed = millis();
         edge.traverseCount = 1;
         
         edges[edgeCount] = edge;
         edgeCount++;
       }
     }
     
     currentNodeId = nodes[nearestNodeIndex].id;
   } else {
     // Create new node
     MapNode node;
     node.id = generateNodeId();
     node.lat = lat;
     node.lng = lng;
     node.type = "path";
     node.isObstacle = false;
     node.lastSeen = millis();
     node.visitCount = 1;
     
     if (nodeCount < MAX_MAP_NODES) {
       nodes[nodeCount] = node;
       
       // Create edge between current node and new node
       if (currentNodeIndex >= 0 && edgeCount < MAX_MAP_EDGES) {
         MapEdge edge;
         edge.id = generateEdgeId();
         edge.sourceId = currentNodeId;
         edge.targetId = node.id;
         edge.weight = calculateDistance(nodes[currentNodeIndex].lat, nodes[currentNodeIndex].lng,
                                       node.lat, node.lng);
         edge.lastTraversed = millis();
         edge.traverseCount = 1;
         
         edges[edgeCount] = edge;
         edgeCount++;
       }
       
       currentNodeId = node.id;
       nodeCount++;
     }
   }
   
   // Save map occasionally (every 10 minutes)
   static unsigned long lastSaveTime = 0;
   if (millis() - lastSaveTime > 600000) {
     saveMap();
     lastSaveTime = millis();
   }
 }
 
 void MapSystem::addObstacle(float lat, float lng, String type) {
   // Check if there's already an obstacle node nearby
   int nearestNodeIndex = findNearestNodeIndex(lat, lng, NODE_PROXIMITY_THRESHOLD);
   
   if (nearestNodeIndex >= 0 && nodes[nearestNodeIndex].isObstacle) {
     // Update existing obstacle
     nodes[nearestNodeIndex].lastSeen = millis();
     nodes[nearestNodeIndex].visitCount++;
     nodes[nearestNodeIndex].type = type; // Update type in case it changed
     return;
   }
   
   // Create new obstacle node
   MapNode node;
   node.id = generateNodeId();
   node.lat = lat;
   node.lng = lng;
   node.type = type;
   node.isObstacle = true;
   node.lastSeen = millis();
   node.visitCount = 1;
   
   if (nodeCount < MAX_MAP_NODES) {
     nodes[nodeCount] = node;
     nodeCount++;
   }
 }
 
 void MapSystem::addLandmark(float lat, float lng, String type, String name) {
   // Create new landmark node (basically a special type of node)
   MapNode node;
   node.id = name + "_" + generateNodeId(); // Use name as part of ID
   node.lat = lat;
   node.lng = lng;
   node.type = type;
   node.isObstacle = false;
   node.lastSeen = millis();
   node.visitCount = 1;
   
   if (nodeCount < MAX_MAP_NODES) {
     nodes[nodeCount] = node;
     nodeCount++;
   }
 }
 
 bool MapSystem::isObstacleNearby(float lat, float lng, float radius) {
   for (int i = 0; i < nodeCount; i++) {
     if (nodes[i].isObstacle) {
       float dist = calculateDistance(lat, lng, nodes[i].lat, nodes[i].lng);
       if (dist <= radius) {
         return true;
       }
     }
   }
   return false;
 }
 
 String MapSystem::getAreaType(float lat, float lng, float radius) {
   // Count node types in the area to determine predominant type
   int typeCount[5] = {0, 0, 0, 0, 0}; // path, door, room, street, other
   
   for (int i = 0; i < nodeCount; i++) {
     float dist = calculateDistance(lat, lng, nodes[i].lat, nodes[i].lng);
     if (dist <= radius) {
       if (nodes[i].type == "path") typeCount[0]++;
       else if (nodes[i].type == "door") typeCount[1]++;
       else if (nodes[i].type == "room") typeCount[2]++;
       else if (nodes[i].type == "street") typeCount[3]++;
       else typeCount[4]++;
     }
   }
   
   // Find max
   int maxIndex = 0;
   for (int i = 1; i < 5; i++) {
     if (typeCount[i] > typeCount[maxIndex]) {
       maxIndex = i;
     }
   }
   
   // Return predominant type
   switch (maxIndex) {
     case 0: return "path";
     case 1: return "door";
     case 2: return "room";
     case 3: return "street";
     default: return "unknown";
   }
 }
 
 bool MapSystem::saveMap() {
   if (!sdAvailable) {
     return false;
   }
   
   // Create JSON document
   // Note: in a real implementation, this would need to be more memory efficient
   // using streaming or chunked writes for large maps
   DynamicJsonDocument doc(50000); // Adjust size as needed
   
   // Add nodes
   JsonArray nodesArray = doc.createNestedArray("nodes");
   for (int i = 0; i < nodeCount; i++) {
     JsonObject nodeObj = nodesArray.createNestedObject();
     nodeObj["id"] = nodes[i].id;
     nodeObj["lat"] = nodes[i].lat;
     nodeObj["lng"] = nodes[i].lng;
     nodeObj["type"] = nodes[i].type;
     nodeObj["isObstacle"] = nodes[i].isObstacle;
     nodeObj["visitCount"] = nodes[i].visitCount;
   }
   
   // Add edges
   JsonArray edgesArray = doc.createNestedArray("edges");
   for (int i = 0; i < edgeCount; i++) {
     JsonObject edgeObj = edgesArray.createNestedObject();
     edgeObj["id"] = edges[i].id;
     edgeObj["sourceId"] = edges[i].sourceId;
     edgeObj["targetId"] = edges[i].targetId;
     edgeObj["weight"] = edges[i].weight;
     edgeObj["traverseCount"] = edges[i].traverseCount;
   }
   
   // Open file for writing
   File mapFile = SD.open(MAP_FILENAME, FILE_WRITE);
   if (!mapFile) {
     return false;
   }
   
   // Write JSON to file
   serializeJson(doc, mapFile);
   mapFile.close();
   
   return true;
 }
 
 bool MapSystem::loadMap() {
   if (!sdAvailable) {
     return false;
   }
   
   // Check if file exists
   if (!SD.exists(MAP_FILENAME)) {
     return false;
   }
   
   // Open file for reading
   File mapFile = SD.open(MAP_FILENAME, FILE_READ);
   if (!mapFile) {
     return false;
   }
   
   // Parse JSON
   DynamicJsonDocument doc(50000); // Adjust size as needed
   DeserializationError error = deserializeJson(doc, mapFile);
   mapFile.close();
   
   if (error) {
     Serial.println("Failed to parse map file");
     return false;
   }
   
   // Clear existing data
   nodeCount = 0;
   edgeCount = 0;
   
   // Load nodes
   JsonArray nodesArray = doc["nodes"];
   for (JsonObject nodeObj : nodesArray) {
     if (nodeCount < MAX_MAP_NODES) {
       MapNode node;
       node.id = nodeObj["id"].as<String>();
       node.lat = nodeObj["lat"];
       node.lng = nodeObj["lng"];
       node.type = nodeObj["type"].as<String>();
       node.isObstacle = nodeObj["isObstacle"];
       node.visitCount = nodeObj["visitCount"];
       node.lastSeen = millis(); // Reset last seen to now
       
       nodes[nodeCount] = node;
       nodeCount++;
     }
   }
   
   // Load edges
   JsonArray edgesArray = doc["edges"];
   for (JsonObject edgeObj : edgesArray) {
     if (edgeCount < MAX_MAP_EDGES) {
       MapEdge edge;
       edge.id = edgeObj["id"].as<String>();
       edge.sourceId = edgeObj["sourceId"].as<String>();
       edge.targetId = edgeObj["targetId"].as<String>();
       edge.weight = edgeObj["weight"];
       edge.traverseCount = edgeObj["traverseCount"];
       edge.lastTraversed = millis(); // Reset last traversed to now
       
       edges[edgeCount] = edge;
       edgeCount++;
     }
   }
   
   return true;
 }
 
 void MapSystem::clearMap() {
   nodeCount = 0;
   edgeCount = 0;
   isFirstPosition = true;
   currentNodeId = "";
   
   // Delete map file if it exists
   if (sdAvailable && SD.exists(MAP_FILENAME)) {
     SD.remove(MAP_FILENAME);
   }
 }
 
 String MapSystem::generateNodeId() {
   // Generate a simple unique ID based on timestamp and random number
   return "n_" + String(millis()) + "_" + String(random(1000, 9999));
 }
 
 String MapSystem::generateEdgeId() {
   // Generate a simple unique ID based on timestamp and random number
   return "e_" + String(millis()) + "_" + String(random(1000, 9999));
 }
 
 int MapSystem::findNodeIndex(String nodeId) {
   for (int i = 0; i < nodeCount; i++) {
     if (nodes[i].id == nodeId) {
       return i;
     }
   }
   return -1; // Not found
 }
 
 int MapSystem::findNearestNodeIndex(float lat, float lng, float maxDistance) {
   int nearestIndex = -1;
   float minDistance = maxDistance + 1.0; // Initialize above threshold
   
   for (int i = 0; i < nodeCount; i++) {
     float dist = calculateDistance(lat, lng, nodes[i].lat, nodes[i].lng);
     if (dist < minDistance && dist <= maxDistance) {
       minDistance = dist;
       nearestIndex = i;
     }
   }
   
   return nearestIndex;
 }