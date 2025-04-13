/*
 * test_obstacle_detection.cpp
 * 
 * Unit tests for the obstacle detection system
 */

 #include <Arduino.h>
 #include <unity.h>
 #include "../src/main/ObstacleDetection.h"
 
 // Mock for Arduino's digitalWrite, digitalRead, etc.
 void digitalWrite(int pin, int value) { /* Mock implementation */ }
 void pinMode(int pin, int mode) { /* Mock implementation */ }
 long pulseIn(int pin, int value, unsigned long timeout) { 
   // Return different values for testing
   static int callCount = 0;
   callCount++;
   
   // Simulate distances of 50, 100, 200, and 400 cm
   switch(callCount % 4) {
     case 0: return 2941; // ~50cm (50 * 2 / 0.034)
     case 1: return 5882; // ~100cm
     case 2: return 11764; // ~200cm
     case 3: return 23529; // ~400cm
   }
   return 0;
 }
 
 // Test initialization
 void test_initialization() {
   ObstacleDetection detector;
   detector.begin(10, 11, 12, 13);
   
   TEST_ASSERT_EQUAL(150, detector.getWarningThreshold());
   TEST_ASSERT_EQUAL(50, detector.getDangerThreshold());
 }
 
 // Test distance calculation
 void test_distance_measurement() {
   ObstacleDetection detector;
   detector.begin(10, 11, 12, 13);
   
   float distance1 = detector.getLowerDistance();
   float distance2 = detector.getLowerDistance();
   float distance3 = detector.getLowerDistance();
   float distance4 = detector.getLowerDistance();
   
   // Test with 10% tolerance for floating point
   TEST_ASSERT_FLOAT_WITHIN(5, 50, distance1);
   TEST_ASSERT_FLOAT_WITHIN(10, 100, distance2);
   TEST_ASSERT_FLOAT_WITHIN(20, 200, distance3);
   TEST_ASSERT_FLOAT_WITHIN(40, 400, distance4);
 }
 
 // Test direction suggestion
 void test_direction_suggestion() {
   ObstacleDetection detector;
   detector.begin(10, 11, 12, 13);
   
   // Test with obstacle directly ahead (both sensors detecting)
   int direction1 = detector.suggestDirection(30, 30);
   
   // Test with obstacle only on lower sensor
   int direction2 = detector.suggestDirection(30, 200);
   
   // Test with obstacle only on upper sensor
   int direction3 = detector.suggestDirection(200, 30);
   
   // Test with no obstacles
   int direction4 = detector.suggestDirection(200, 200);
   
   // Direction should be -1 (left), 0 (stop), or 1 (right)
   TEST_ASSERT_TRUE(direction1 >= -1 && direction1 <= 1);
   TEST_ASSERT_TRUE(direction2 >= -1 && direction2 <= 1);
   TEST_ASSERT_TRUE(direction3 >= -1 && direction3 <= 1);
   TEST_ASSERT_EQUAL(0, direction4); // No obstacles should return 0
 }
 
 void setup() {
   delay(2000);  // Give serial port time to connect
   
   UNITY_BEGIN();
   
   RUN_TEST(test_initialization);
   RUN_TEST(test_distance_measurement);
   RUN_TEST(test_direction_suggestion);
   
   UNITY_END();
 }
 
 void loop() {
   // Nothing to do here
 }