#pragma once

#include "../../../inc/MarlinConfigPre.h"
#include "stamp_config.h"

class StampSensor {
    public:
        static void init();
        static bool raw_read();
        static bool triggered();

        #ifdef STAMP_SENSOR_SIMULATION
            static void set_simulated_signal(const bool state);
        #endif

    private:
        #ifdef STAMP_SENSOR_SIMULATION
            static bool simulated_state;
        #endif
};

extern StampSensor stamp_sensor;