/*
 * AIClassifier.cpp
 * 
 * Implementation of AI-based obstacle classification system
 */

 #include "AIClassifier.h"
 #include <Arduino.h>
 
 // In a real implementation, this would include TensorFlow Lite
 // #include <TensorFlowLite.h>
 // #include "tensorflow/lite/micro/all_ops_resolver.h"
 // #include "tensorflow/lite/micro/micro_error_reporter.h"
 // #include "tensorflow/lite/micro/micro_interpreter.h"
 // #include "tensorflow/lite/schema/schema_generated.h"
 // #include "model_data.h" // Contains the TFLite model
 
 // Obstacle type definitions
 const char* obstacle_types[] = {
   "wall", "person", "chair", "table", "stairs", 
   "door", "pole", "unknown"
 };
 
 AIClassifier::AIClassifier() {
   modelInitialized = false;
   currentObstacleType = "unknown";
   confidenceScore = 0.0;
   
   // Initialize sensor buffer
   for (int i = 0; i < 10; i++) {
     for (int j = 0; j < 8; j++) {
       sensorBuffer[i][j] = 0.0;
     }
   }
 }
 
 bool AIClassifier::begin() {
   // In a real implementation, this would initialize TensorFlow Lite
   // model = tflite::GetModel(g_model_data);
   // interpreter->AllocateTensors();
   // etc.
   
   // For this prototype, we'll simulate model initialization
   modelInitialized = true;
   Serial.println("AI Classifier initialized");
   return modelInitialized;
 }
 
 void AIClassifier::updateReadings(float distLower, float distUpper) {
   // Extract features and update buffer
   extractFeatures(distLower, distUpper);
 }
 
 String AIClassifier::classifyObstacle() {
   if (!modelInitialized) {
     return "unknown";
   }
   
   // In a real implementation, this would run inference using TensorFlow Lite
   // TfLiteStatus invoke_status = interpreter->Invoke();
   // Process outputs
   // etc.
   
   // For this prototype, we'll simulate classification based on patterns
   
   // Simple heuristic classification based on sensor readings
   float lowerAvg = 0, upperAvg = 0, variance = 0;
   
   // Calculate average distances
   for (int i = 0; i < 8; i++) {
     lowerAvg += sensorBuffer[0][i];
     upperAvg += sensorBuffer[1][i];
   }
   lowerAvg /= 8;
   upperAvg /= 8;
   
   // Calculate height difference
   float heightDiff = abs(upperAvg - lowerAvg);
   
   // Calculate variance
   for (int i = 0; i < 8; i++) {
     variance += pow(sensorBuffer[0][i] - lowerAvg, 2);
   }
   variance /= 8;
   
   // Simplified classification logic
   String detectedType = "unknown";
   float confidence = 0.6; // Default confidence
   
   // Very close readings at both sensors with low variance
   if (lowerAvg < 50 && upperAvg < 50 && variance < 10) {
     detectedType = "wall";
     confidence = 0.92;
   }
   // Close lower reading, far upper reading
   else if (lowerAvg < 70 && upperAvg > 150) {
     detectedType = "table";
     confidence = 0.81;
   }
   // Medium distance with high variance in lower sensor
   else if (lowerAvg < 100 && upperAvg < 100 && variance > 50) {
     detectedType = "stairs";
     confidence = 0.89;
   }
   // Medium-high variance with similar upper/lower readings
   else if (variance > 20 && variance < 50 && abs(lowerAvg - upperAvg) < 30) {
     detectedType = "person";
     confidence = 0.87;
   }
   // Close readings with medium height difference
   else if (lowerAvg < 80 && heightDiff > 30 && heightDiff < 80) {
     detectedType = "chair";
     confidence = 0.81;
   }
   // Very close lower, medium upper
   else if (lowerAvg < 30 && upperAvg > 80 && upperAvg < 150) {
     detectedType = "pole";
     confidence = 0.78;
   }
   // Medium readings with specific height difference
   else if (lowerAvg > 50 && lowerAvg < 120 && heightDiff < 20) {
     detectedType = "door";
     confidence = 0.72;
   }
   
   // Update current classification if confidence is high enough
   if (confidence > 0.6) {
     currentObstacleType = detectedType;
     confidenceScore = confidence;
   }
   
   return currentObstacleType;
 }
 
 void AIClassifier::extractFeatures(float distLower, float distUpper) {
   // Shift buffer to make room for new data
   for (int i = 0; i < 10; i++) {
     for (int j = 0; j < 7; j++) {
       sensorBuffer[i][j] = sensorBuffer[i][j+1];
     }
   }
   
   // Calculate features for this time step
   sensorBuffer[0][7] = distLower;                            // Lower sensor distance
   sensorBuffer[1][7] = distUpper;                            // Upper sensor distance
   sensorBuffer[2][7] = abs(distLower - distUpper);           // Height difference
   sensorBuffer[3][7] = (distLower + distUpper) / 2;          // Average distance
   
   // Rate of change (if we have previous readings)
   if (sensorBuffer[0][6] != 0) {
     sensorBuffer[4][7] = sensorBuffer[0][6] - distLower;     // Lower sensor rate of change
     sensorBuffer[5][7] = sensorBuffer[1][6] - distUpper;     // Upper sensor rate of change
   } else {
     sensorBuffer[4][7] = 0;
     sensorBuffer[5][7] = 0;
   }
   
   sensorBuffer[6][7] = calculateVariance(0);                 // Lower sensor variance
   sensorBuffer[7][7] = calculateVariance(1);                 // Upper sensor variance
   sensorBuffer[8][7] = calculatePeakFrequency(0);            // Lower sensor dominant frequency
   sensorBuffer[9][7] = calculatePeakFrequency(1);            // Upper sensor dominant frequency
 }
 
 float AIClassifier::calculateVariance(int sensorIndex) {
   float mean = 0;
   int validSamples = 0;
   
   // Calculate mean of valid samples
   for (int i = 0; i < 8; i++) {
     if (sensorBuffer[sensorIndex][i] > 0) {
       mean += sensorBuffer[sensorIndex][i];
       validSamples++;
     }
   }
   
   if (validSamples == 0) return 0;
   mean /= validSamples;
   
   // Calculate variance
   float variance = 0;
   for (int i = 0; i < 8; i++) {
     if (sensorBuffer[sensorIndex][i] > 0) {
       variance += pow(sensorBuffer[sensorIndex][i] - mean, 2);
     }
   }
   
   return variance / validSamples;
 }
 
 float AIClassifier::calculatePeakFrequency(int sensorIndex) {
   // Simple implementation of frequency detection
   // In a real implementation, this would use FFT
   int zero_crossings = 0;
   bool above = false;
   float mean = 0;
   
   // Calculate mean
   for (int i = 0; i < 8; i++) {
     if (sensorBuffer[sensorIndex][i] > 0) {
       mean += sensorBuffer[sensorIndex][i];
     }
   }
   mean /= 8;
   
   // Count zero crossings around mean
   above = sensorBuffer[sensorIndex][0] > mean;
   for (int i = 1; i < 8; i++) {
     bool now_above = sensorBuffer[sensorIndex][i] > mean;
     if (above != now_above && sensorBuffer[sensorIndex][i] > 0) {
       zero_crossings++;
       above = now_above;
     }
   }
   
   // Return estimated frequency based on zero crossings
   return zero_crossings / (8 * 0.1); // 0.1s is our sampling period (assuming 10Hz)
 }