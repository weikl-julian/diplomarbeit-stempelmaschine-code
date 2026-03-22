#include "../../../inc/MarlinConfig.h"
#include "stamp_sensor.h"

StampSensor stamp_sensor;

#ifdef STAMP_SENSOR_SIMULATION
    bool StampSensor::simulated_state = false;
#endif

void StampSensor::init() {
    #if STAMP_SENSOR_PULLUP
        SET_INPUT_PULLUP(STAMP_SENSOR_PIN);
    #else
        SET_INPUT(STAMP_SENSOR_PIN);
    #endif
}

bool StampSensor::raw_read () {
    return READ(STAMP_SENSOR_PIN);
}

bool StampSensor::triggered () {
    bool state = raw_read();

    if (STAMP_SENSOR_INVERTING){
        state = !state;
    }
    
    #ifdef STAMP_SENSOR_SIMULATION
        state = simulated_state;
    #endif

    return state;
}

#ifdef STAMP_SENSOR_SIMULATION
    void StampSensor::set_simulated_signal(const bool state) {
        simulated_state = state;
    }
#endif