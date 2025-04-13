 #include "ObstacleDetection.h"
 #include "Navigation.h"
 #include "AIClassifier.h"
 #include "MapSystem.h"
 
 // Pin Definitions
 const int TRIG_PIN_LOWER = 12;
 const int ECHO_PIN_LOWER = 13;
 const int TRIG_PIN_UPPER = 14;
 const int ECHO_PIN_UPPER = 15;
 const int VIBRATE_LEFT = 5;
 const int VIBRATE_RIGHT = 6;
 const int VIBRATE_WARNING = 7;
 const int BUTTON_SET_WAYPOINT = 2;
 const int BUTTON_NAVIGATION = 3;
 const int SPEAKER_PIN = 9;
 
 // Global Instances
 ObstacleDetection obstacleDetector;
 AIClassifier aiClassifier;
 NavigationSystem navSystem;
 MapSystem mapSystem;
 
 // Timing variables
 unsigned long lastSensorRead = 0;
 unsigned long lastGpsUpdate = 0;
 unsigned long lastFeedbackUpdate = 0;
 const int SENSOR_INTERVAL = 100;  // 10Hz
 const int GPS_INTERVAL = 1000;    // 1Hz
 const int FEEDBACK_INTERVAL = 250;  // 4Hz
 
 void setup() {
   Serial.begin(115200);
   Serial.println("SmartGuide Initializing...");
   
   // Initialize subsystems
   obstacleDetector.begin(TRIG_PIN_LOWER, ECHO_PIN_LOWER, TRIG_PIN_UPPER, ECHO_PIN_UPPER);
   
   // Initialize feedback pins
   pinMode(VIBRATE_LEFT, OUTPUT);
   pinMode(VIBRATE_RIGHT, OUTPUT);
   pinMode(VIBRATE_WARNING, OUTPUT);
   
   // Initialize buttons
   pinMode(BUTTON_SET_WAYPOINT, INPUT_PULLUP);
   pinMode(BUTTON_NAVIGATION, INPUT_PULLUP);
   
   // Initialize AI classifier
   if (!aiClassifier.begin()) {
     Serial.println("Failed to initialize AI classifier!");
   }
   
   // Initialize navigation system
   navSystem.begin();
   
   // Initialize map system
   if (!mapSystem.begin()) {
     Serial.println("Failed to initialize map system!");
   }
   
   // Speaker initialization
   speakMessage("SmartGuide ready");
   
   Serial.println("SmartGuide Initialization Complete");
 }
 
 void loop() {
   // Read sensors at specified interval
   if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
     // Get distance readings from ultrasonic sensors
     float distLower = obstacleDetector.getLowerDistance();
     float distUpper = obstacleDetector.getUpperDistance();
     
     // Update AI classifier with new readings
     aiClassifier.updateReadings(distLower, distUpper);
     
     // Process obstacle detection
     processObstacles(distLower, distUpper);
     
     lastSensorRead = millis();
   }
   
   // Update GPS and navigation at specified interval
   if (millis() - lastGpsUpdate >= GPS_INTERVAL) {
     // Update GPS location
     if (navSystem.updateGpsLocation()) {
       // Update map with new location
       mapSystem.updateCurrentPosition(navSystem.getCurrentLat(), navSystem.getCurrentLng());
     }
     
     lastGpsUpdate = millis();
   }
   
   // Update feedback system at specified interval
   if (millis() - lastFeedbackUpdate >= FEEDBACK_INTERVAL) {
     if (navSystem.isNavigating()) {
       // Provide navigation feedback
       processNavigationFeedback();
     }
     
     lastFeedbackUpdate = millis();
   }
   
   // Check user input
   checkButtons();
 }
 
 void processObstacles(float distLower, float distUpper) {
   bool obstacleDetected = false;
   
   // Process lower sensor data (for ground-level obstacles)
   if (distLower < obstacleDetector.getDangerThreshold()) {
     // Immediate danger - strong feedback
     digitalWrite(VIBRATE_WARNING, HIGH);
     
     // Get obstacle classification
     String obstacleType = aiClassifier.classifyObstacle();
     
     // Provide audio feedback with obstacle type
     if (obstacleType != "unknown") {
       speakMessage(obstacleType + " ahead");
     } else {
       speakMessage("Obstacle ahead");
     }
     
     obstacleDetected = true;
   } 
   else if (distLower < obstacleDetector.getWarningThreshold()) {
     // Warning - moderate feedback
     digitalWrite(VIBRATE_WARNING, HIGH);
     delay(100);
     digitalWrite(VIBRATE_WARNING, LOW);
     
     obstacleDetected = true;
   }
   
   // Process upper sensor data (for head-height obstacles)
   if (distUpper < obstacleDetector.getWarningThreshold()) {
     // Upper obstacle detected
     digitalWrite(VIBRATE_WARNING, HIGH);
     speakMessage("Head-height obstacle");
     
     obstacleDetected = true;
   }
   
   // If obstacle detected, suggest direction and update map
   if (obstacleDetected) {
     // Update map with obstacle
     String obstacleType = aiClassifier.getLastObstacleType();
     mapSystem.addObstacle(navSystem.getCurrentLat(), navSystem.getCurrentLng(), obstacleType);
     
     // Get suggested direction
     int suggestedDirection = obstacleDetector.suggestDirection(distLower, distUpper);
     
     // Provide haptic feedback for direction
     if (suggestedDirection < 0) {
       // Suggest left
       digitalWrite(VIBRATE_LEFT, HIGH);
       delay(200);
       digitalWrite(VIBRATE_LEFT, LOW);
     } else if (suggestedDirection > 0) {
       // Suggest right
       digitalWrite(VIBRATE_RIGHT, HIGH);
       delay(200);
       digitalWrite(VIBRATE_RIGHT, LOW);
     }
   } else {
     // No obstacles - turn off warnings
     digitalWrite(VIBRATE_WARNING, LOW);
   }
 }
 
 void processNavigationFeedback() {
   // Get next navigation instruction
   String instruction = navSystem.getNextInstruction();
   
   // Announce if it's a new instruction
   if (navSystem.isNewInstruction()) {
     speakMessage(instruction);
   }
   
   // Provide haptic feedback for direction
   int direction = navSystem.getCurrentDirection();
   if (direction < -30) {
     // Turn left
     digitalWrite(VIBRATE_LEFT, HIGH);
     delay(100);
     digitalWrite(VIBRATE_LEFT, LOW);
   } else if (direction > 30) {
     // Turn right
     digitalWrite(VIBRATE_RIGHT, HIGH);
     delay(100);
     digitalWrite(VIBRATE_RIGHT, LOW);
   }
   
   // Check if destination reached
   if (navSystem.isDestinationReached()) {
     speakMessage("Destination reached");
     navSystem.stopNavigation();
   }
 }
 
 void checkButtons() {
   // Set waypoint button
   if (digitalRead(BUTTON_SET_WAYPOINT) == LOW) {
     delay(50); // Debounce
     if (digitalRead(BUTTON_SET_WAYPOINT) == LOW) {
       // Button still pressed after debounce
       speakMessage("Setting waypoint");
       
       // Set waypoint at current location
       navSystem.setWaypoint("Waypoint " + String(millis()), "custom");
       
       // Wait for button release
       while (digitalRead(BUTTON_SET_WAYPOINT) == LOW) {
         delay(10);
       }
     }
   }
   
   // Navigation button
   if (digitalRead(BUTTON_NAVIGATION) == LOW) {
     delay(50); // Debounce
     if (digitalRead(BUTTON_NAVIGATION) == LOW) {
       // Button still pressed after debounce
       if (navSystem.isNavigating()) {
         // Stop navigation
         speakMessage("Navigation stopped");
         navSystem.stopNavigation();
       } else {
         // Start navigation to last set waypoint
         if (navSystem.startNavigation()) {
           speakMessage("Navigation started");
         } else {
           speakMessage("No destination set");
         }
       }
       
       // Wait for button release
       while (digitalRead(BUTTON_NAVIGATION) == LOW) {
         delay(10);
       }
     }
   }
 }
 
 // Function to output spoken messages
 void speakMessage(String message) {
   Serial.println("SPEECH: " + message);
   // In a real implementation, this would use text-to-speech
   // or pre-recorded audio messages
 }