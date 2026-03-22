#pragma once


// Sensor-Hardware aktiv
#define STAMP_DOCUMENT_SENSOR

// Gewählter Pin: Port A2 -> Marlin Pin 29
#define STAMP_SENSOR_PIN 29

// true = HIGH 
// false = LOW
#define STAMP_SENSOR_INVERTING false

// internen Pullup verwenden
#define STAMP_SENSOR_PULLUP true

// Simulationsmodus
#define STAMP_SENSOR_SIMULATION

// Stempelbreich Untergrenze X-Achse
#define STAMP_X_MIN 10.0f
// Stempelbreich Obergrenze X-Achse
#define STAMP_X_MAX 290.0f
// Stempelbreich Untergrenze Y-Achse
#define STAMP_Y_MIN 30.0f
// Stempelbreich Obergrenze Y-Achse
#define STAMP_Y_MAX 190.0f

// Limit for the amount of documents that can be stamped during one process
#define STAMP_DOC_LIMIT 200.0f


#define STAMP_POSITION_HOME_X     160.0f

#define STAMP_POSITION_HOME_Y     150.0f

#define STAMP_POSITION_HOME_Z     50.0f

#define STAMP_POSITION_SAFE_Z     20.0f
#define STAMP_POSITION_SREADY_Z   15.0f

#define STAMP_POSITION_INK_Z_SAFE 5.0f
#define STAMP_POSITION_INK_Z      4.0f
#define STAMP_POSITION_INK_X      163.0f
#define STAMP_POSITION_INK_Y      300.0f

#define STAMP_POSITION_PAPER_OUT_SAFE_STOP 295.0f
#define STAMP_POSITION_PAPER_OUT 300.0f

#define STAMP_FEEDRATE_TRAVEL_FAST 90.0f
#define STAMP_FEEDRATE_TRAVEL 70.0f             // mm/s
#define STAMP_FEEDRATE_TRAVEL_Z 25.0f           // mm/s
#define STAMP_FEEDRATE 2.5f                     // mm/s
#define STAMP_FEEDRATE_LIFT_Z 10.0f             // mm/s
#define STAMP_FEEDRATE_SAFE 10.0f               // mm/s


#define STAMP_PROBE_STEP_Z 0.15f                // Increments down
#define STAMP_PROBE_PRE_STAMP_OFFSET_Z 5.0f
#define STAMP_PROBE_PRE_STAMP__MOVE_OFFSET_Z 10.0f
#define STAMP_PROBE_SAFETY_OFFSET 1.0f
#define STAMP_PROBE_RELEASE_OFFSET_Z 0.30f      // kleiner Sicherheitsabstand nach Trigger
#define STAMP_PROBE_MIN_Z 0.0f                  // Untergrenze als Sicherheit

#define STAMP_STAMPING_SENSOR_OFFST 0.5f        // How far the stamping goes down past the sensor trigger hight