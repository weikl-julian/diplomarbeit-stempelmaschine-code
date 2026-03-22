#include "../../../inc/MarlinConfigPre.h"
#include "../../dogm/marlinui_DOGM.h"
#include "../../dogm/u8g_fontutf8.h"
#include "../../lcdprint.h"
#include "../../marlinui.h"
#include "../menu_item.h"
#include "../../utf8.h" 
#include "../../../libs/numtostr.h"
#include "../../dogm/fontdata/fontdata_ISO10646_1.h"
#include "menu_stamp.h"
#include "stamp_sensor.h"


void wait_stamp_documents(){
    static bool initialized = false;
    static millis_t entry_time = 0;

    START_MENU();
    STATIC_ITEM_F(F("Lege die Dokumente"));
    STATIC_ITEM_F(F("in die Ablage."));
    END_MENU();

    #ifdef STAMP_SENSOR_SIMULATION
        if (!initialized) {
            entry_time = millis();
            initialized = true;
            stamp_sensor.set_simulated_signal(false);
        }
    
        if (ELAPSED(millis(), entry_time + 2000)) {  //artificially trigger the sensor after 10 seconds for testing
            stamp_sensor.set_simulated_signal(true);
        }
    #endif

   if (ui.use_click()) {
        if (stamp_sensor.triggered()) {
            initialized = false;
            ui.goto_screen(menu_stamp_positioning);
            return;
        }
        else {
            initialized = false;
            show_stamp_error(STAMP_ERROR_NO_DOCUMENTS);
            return;
        }
    }
}
