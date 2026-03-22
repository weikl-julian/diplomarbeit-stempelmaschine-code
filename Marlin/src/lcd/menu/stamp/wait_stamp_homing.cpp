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
#include "stamp_config.h"

static bool stamp_homing_started = false;
static millis_t stamp_homing_timeout = 0;

void wait_stamp_homing() {
    START_MENU();
    STATIC_ITEM_F(F("Warte Auf Abschluss"));
    STATIC_ITEM_F(F("der Vorbereitungen ..."));
    END_MENU();

    if (!stamp_homing_started) {
        queue.inject_P(PSTR("G28"));
        stamp_homing_started = true;
        stamp_homing_timeout = millis() + 120000UL; // 2 Minuten
    }

    if (all_axes_homed()) {
        stamp_start_move_z(STAMP_POSITION_HOME_Z, STAMP_FEEDRATE_TRAVEL_Z);
        stamp_start_move_xy(STAMP_POSITION_HOME_X, STAMP_POSITION_HOME_Y, STAMP_FEEDRATE_TRAVEL_FAST);
        stamp_homing_started = false;
        stamp_homing_timeout = 0;
        ui.goto_screen(wait_stamp_documents);
        return;
    }

    if (stamp_homing_started && ELAPSED(millis(), stamp_homing_timeout)) {
        stamp_homing_started = false;
        stamp_homing_timeout = 0;
        show_stamp_error(STAMP_ERROR_HOMING);
        return;
    }
}