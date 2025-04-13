/*
 * ObstacleDetection.h
 * 
 * Handles ultrasonic sensor operations for detecting obstacles
 */

 #ifndef OBSTACLE_DETECTION_H
 #define OBSTACLE_DETECTION_H
 
 class ObstacleDetection {
   private:
     int trigPinLower;
     int echoPinLower;
     int trigPinUpper;
     int echoPinUpper;
     
     // Distance thresholds (in cm)
     const int warningThreshold = 150;
     const int dangerThreshold = 50;
     
     // Last measured distances
     float lastLowerDistance;
     float lastUpperDistance;
     
     // Calculate distance from sensor readings
     float calculateDistance(int trigPin, int echoPin);
     
   public:
     ObstacleDetection();
     
     // Initialize with pin assignments
     void begin(int trigPinLower, int echoPinLower, int trigPinUpper, int echoPinUpper);
     
     // Get distance measurements
     float getLowerDistance();
     float getUpperDistance();
     
     // Get thresholds
     int getWarningThreshold() { return warningThreshold; }
     int getDangerThreshold() { return dangerThreshold; }
     
     // Suggest direction to move (-1 for left, 0 for stop, 1 for right)
     int suggestDirection(float distLower, float distUpper);
 };
 
 #endif