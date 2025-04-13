/*
 * AIClassifier.h
 * 
 * Handles obstacle classification using TensorFlow Lite model
 */

 #ifndef AI_CLASSIFIER_H
 #define AI_CLASSIFIER_H
 
 #include <Arduino.h>
 
 class AIClassifier {
   private:
     // Sensor data buffer for classification
     float sensorBuffer[10][8]; // 10 features across 8 time steps
     
     // Current obstacle classification
     String currentObstacleType;
     float confidenceScore;
     
     // Helper functions
     float calculateVariance(int sensorIndex);
     float calculatePeakFrequency(int sensorIndex);
     void extractFeatures(float distLower, float distUpper);
     
     // TFLite model variables
     bool modelInitialized;
     
   public:
     AIClassifier();
     
     // Initialize the classifier
     bool begin();
     
     // Update with new sensor readings
     void updateReadings(float distLower, float distUpper);
     
     // Get current obstacle classification
     String classifyObstacle();
     String getLastObstacleType() { return currentObstacleType; }
     float getConfidence() { return confidenceScore; }
 };
 
 #endif