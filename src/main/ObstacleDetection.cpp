/*
 * ObstacleDetection.cpp
 * 
 * Implementation of the obstacle detection system
 */

 #include "ObstacleDetection.h"
 #include <Arduino.h>
 
 ObstacleDetection::ObstacleDetection() {
   lastLowerDistance = 400.0; // Initialize with max range
   lastUpperDistance = 400.0;
 }
 
 void ObstacleDetection::begin(int trigLow, int echoLow, int trigUp, int echoUp) {
   trigPinLower = trigLow;
   echoPinLower = echoLow;
   trigPinUpper = trigUp;
   echoPinUpper = echoUp;
   
   // Set pin modes
   pinMode(trigPinLower, OUTPUT);
   pinMode(echoPinLower, INPUT);
   pinMode(trigPinUpper, OUTPUT);
   pinMode(echoPinUpper, INPUT);
 }
 
 float ObstacleDetection::calculateDistance(int trigPin, int echoPin) {
   // Clear the trigger pin
   digitalWrite(trigPin, LOW);
   delayMicroseconds(2);
   
   // Set the trigger pin HIGH for 10 microseconds
   digitalWrite(trigPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPin, LOW);
   
   // Read the echo pin, convert to distance in cm
   long duration = pulseIn(echoPin, HIGH, 23200); // Timeout for ~4m range
   
   // If timeout occurred, return max range
   if (duration == 0) {
     return 400.0;
   }
   
   float distance = duration * 0.034 / 2;
   
   // Limit to reasonable range
   if (distance > 400.0) {
     distance = 400.0;
   }
   
   return distance;
 }
 
 float ObstacleDetection::getLowerDistance() {
   lastLowerDistance = calculateDistance(trigPinLower, echoPinLower);
   return lastLowerDistance;
 }
 
 float ObstacleDetection::getUpperDistance() {
   lastUpperDistance = calculateDistance(trigPinUpper, echoPinUpper);
   return lastUpperDistance;
 }
 
 int ObstacleDetection::suggestDirection(float distLower, float distUpper) {
   // If both sensors detect nearby obstacles
   if (distLower < warningThreshold && distUpper < warningThreshold) {
     // Use historical data to determine best direction
     // This is a simplified algorithm - in a real implementation, 
     // this would use more sophisticated path planning
     
     // For now, just suggest the direction with more space
     
     // Take 3 readings on the left
     digitalWrite(trigPinLower, LOW);
     delay(50);
     float leftSum = 0;
     for (int i = 0; i < 3; i++) {
       // Turn slightly left (this would be handled by user in reality)
       // In a real implementation, this would use servo motors or instruct the user
       leftSum += calculateDistance(trigPinLower, echoPinLower);
       delay(10);
     }
     float leftAvg = leftSum / 3;
     
     // Take 3 readings on the right
     digitalWrite(trigPinLower, LOW);
     delay(50);
     float rightSum = 0;
     for (int i = 0; i < 3; i++) {
       // Turn slightly right (this would be handled by user in reality)
       rightSum += calculateDistance(trigPinLower, echoPinLower);
       delay(10);
     }
     float rightAvg = rightSum / 3;
     
     // Compare and suggest direction
     if (leftAvg > rightAvg * 1.25) {
       return -1; // Suggest left
     } else if (rightAvg > leftAvg * 1.25) {
       return 1;  // Suggest right
     } else {
       return 0;  // Suggest stop/wait
     }
   }
   
   // If just lower sensor detects an obstacle
   else if (distLower < warningThreshold) {
     // Simple left/right detection based on previous readings
     // In a real implementation, this would be more sophisticated
     return random(-1, 2); // Simplified random suggestion for prototype
   }
   
   // If just upper sensor detects an obstacle
   else if (distUpper < warningThreshold) {
     // For head-height obstacles, suggest ducking or moving around
     return random(-1, 2); // Simplified random suggestion for prototype
   }
   
   return 0; // No immediate obstacle, keep going
 }