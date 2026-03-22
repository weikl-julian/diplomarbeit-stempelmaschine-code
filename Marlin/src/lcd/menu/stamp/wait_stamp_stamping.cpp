#include "../../../inc/MarlinConfigPre.h"
#include "../../dogm/marlinui_DOGM.h"
#include "../../dogm/u8g_fontutf8.h"
#include "../../lcdprint.h"
#include "../../marlinui.h"
#include "../menu_item.h"
#include "../../utf8.h"
#include "../../../libs/numtostr.h"
#include "../menu.h"
#include "../../dogm/fontdata/fontdata_ISO10646_1.h"
#include "menu_stamp.h"
#include "../menu_addon.h"
#include "../../../module/motion.h"
#include "../../../module/planner.h"
#include "../../../gcode/queue.h"
#include "stamp_sensor.h"
#include "stamp_config.h"
#include "../../../module/probe.h"

/*
  -----------------------------------------
  Non-blocking Stamp Process State Machine
  -----------------------------------------
*/

enum class StampState : uint8_t {
  INIT,
  RECHECK_DOCS,

  // Ink cycle
  INK_ZSAFE_START,
  INK_ZSAFE_WAIT,
  INK_XY_START,
  INK_XY_WAIT,
  INK_ZDOWN_START,
  INK_ZDOWN_WAIT,
  INK_ZUP_START,
  INK_ZUP_WAIT,

  // Move to stamp position
  MOVE_TO_STAMP_ZSAFE_START,
  MOVE_TO_STAMP_ZSAFE_WAIT,
  MOVE_TO_STAMP_XY_START,
  MOVE_TO_STAMP_XY_WAIT,
  MOVE_TO_STAMP_ZREADY_START,
  MOVE_TO_STAMP_ZREADY_WAIT,

  // Stamp movement
  STAMP_DOWN_START,
  STAMP_DOWN_WAIT,
  STAMPING_START,
  STAMPING_WAIT,
  STAMP_UP_START,
  STAMP_UP_WAIT,

  // Paper out
  PAPER_OUT_MOVE_START,
  PAPER_OUT_MOVE_WAIT,
  PAPER_OUT_POSITION,
  PAPER_OUT_POSITION_WAIT,
  PAPER_OUT,
  PAPER_OUT_WAIT,

  // Back to stamp Y
  MOVE_BACK_Y_START,
  MOVE_BACK_Y_WAIT,

  NEXT_OR_DONE,
  SUCCESS,
  ERROR
};

enum StampStateScreen : uint8_t {
  STAMP_PROCESS_SCREEN_NONE = 0,
  STAMP_PROCESS_SCREEN_MAIN,
  STAMP_PROCESS_SCREEN_MANUAL_STOP_INFO,
};

static StampState current_stamp_state = StampState::INIT;
static uint8_t stamped_documents = 0;
static uint8_t docs_since_last_ink = 0;
static enum StampError current_stamp_error = STAMP_ERROR_NONE;
static StampStateScreen current_stamp_process_screen = STAMP_PROCESS_SCREEN_MAIN;
static millis_t state_entry_ms = 0;

//static boolean has_entered_state = false;

/*
  Manual abort
*/
static millis_t manual_stop_first_click_ms = 0;
static uint8_t manual_stop_click_count = 0;

extern float stamp_x_pos;
extern float stamp_y_pos;

/*
  Z-Probe
*/

static float stamp_trigger_z = NAN;
static boolean has_already_stamped = false;


/*
  -----------------------
  Small Helper Functions
  -----------------------
*/

// Sets specified process screen
static void set_stamp_process_screen(const StampStateScreen screen) {
  current_stamp_process_screen = screen;
}

// Sets specified error code
static void set_stamp_error(const StampError error) {
  current_stamp_error = error;
  current_stamp_state = StampState::ERROR;
  state_entry_ms = millis();
}

// Enters specified state
static void enter_stamp_state(const StampState new_state) {
  current_stamp_state = new_state;
  state_entry_ms = millis();
}

// Checks sensor if there are still documents present.
static inline bool stamp_documents_present() {
  return stamp_sensor.triggered();
}

// Checks if the document limit has been reached. Returns true if it has.
static inline bool stamp_reached_doc_limit() {
  return stamped_documents >= STAMP_DOC_LIMIT;
}

// Checks if stamp is in motion. Returns false if in motion.
inline bool stamp_motion_done() {
  return !planner.busy() && !queue.has_commands_queued();
}

static inline bool stamp_probe_triggered() {
  return PROBE_TRIGGERED();
}


//  Manual abort: 3 clicks within 5 seconds
static bool manual_stop_three_click_abort() {
  const millis_t now = millis();

  if (manual_stop_click_count && ELAPSED(now, manual_stop_first_click_ms + 5000UL))
    manual_stop_click_count = 0;

  if (ui.use_click()) {
    if (manual_stop_click_count == 0) {
      manual_stop_click_count = 1;
      manual_stop_first_click_ms = now;
    }
    else {
      manual_stop_click_count++;
      if (manual_stop_click_count >= 3) {
        manual_stop_click_count = 0;
        return true;
      }
    }
  }

  return false;
}

/*
  -------------------
  Final Success Menu
  -------------------
*/

void menu_stamp_done() {
  START_MENU();
  STATIC_ITEM_F(F("Stempelprozess"), SS_CENTER);
  STATIC_ITEM_F(F("abgeschlossen!"), SS_CENTER);
  ACTION_ITEM_F(F("Start"),[]{ ui.goto_screen(menu_stamp_start); });
  END_MENU();
}

/*
  -----------------------------------
  Non-blocking Move Start Functions
  -----------------------------------
*/

void stamp_start_move_xyz(const float x, const float y, const float z, const float fr_mm_s) {
  destination = current_position;
  destination.x = x;
  destination.y = y;
  destination.z = z;
  feedrate_mm_s = fr_mm_s;
  prepare_line_to_destination();
  current_position = destination;
}

void stamp_start_move_xy(const float x, const float y, const float fr_mm_s) {
  destination = current_position;
  destination.x = x;
  destination.y = y;
  feedrate_mm_s = fr_mm_s;
  prepare_line_to_destination();
  current_position = destination;
}

void stamp_start_move_y(const float y, const float fr_mm_s) {
  destination = current_position;
  destination.y = y;
  feedrate_mm_s = fr_mm_s;
  prepare_line_to_destination();
  current_position = destination;
}

void stamp_start_move_z(const float z, const float fr_mm_s) {
  destination = current_position;
  destination.z = z;
  feedrate_mm_s = fr_mm_s;
  prepare_line_to_destination();
  current_position = destination;
}

/*
  -------------------------
  Main Process State Logic
  -------------------------
*/

static void tick_stamp_process() {
  switch (current_stamp_state) {

    case StampState::INIT:
      stamped_documents = 0;
      docs_since_last_ink = 5;   // ink on before first document
      has_already_stamped = false;
      thermalManager.set_fan_speed(0, 0);
      set_stamp_process_screen(STAMP_PROCESS_SCREEN_MAIN);
      enter_stamp_state(StampState::RECHECK_DOCS);
      break;

    case StampState::RECHECK_DOCS:
      if (!stamp_documents_present()) {
        set_stamp_error(STAMP_ERROR_NO_DOCUMENTS);
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        return;
      }

      if (docs_since_last_ink >= 5){
        enter_stamp_state(StampState::INK_ZSAFE_START);
      }else{
        enter_stamp_state(StampState::MOVE_TO_STAMP_ZSAFE_START);
      }
      break;

    /*
      -----------
      INK CYCLE
      -----------
    */
    case StampState::INK_ZSAFE_START:
    //          ACTION:
      stamp_start_move_z(STAMP_POSITION_SAFE_Z, STAMP_FEEDRATE_TRAVEL_Z);
    //          ON EXIT:
      enter_stamp_state(StampState::INK_ZSAFE_WAIT);
      break;

    case StampState::INK_ZSAFE_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::INK_XY_START);
      break;


    // - Inking start move xy
    case StampState::INK_XY_START:
    //          ACTION:
        if (current_position.y != 300.0){ // If it inks while PAPER_OUT then move to y does nothing as it already is at 300 (Also works as safety, to ensure that ink only at y=300)
          stamp_start_move_xy(current_position.x, STAMP_POSITION_PAPER_OUT_SAFE_STOP, STAMP_FEEDRATE_TRAVEL_FAST);
          stamp_start_move_xy(current_position.x, STAMP_POSITION_PAPER_OUT, STAMP_FEEDRATE_SAFE);
        }
        stamp_start_move_xy(STAMP_POSITION_INK_X, current_position.y , STAMP_FEEDRATE_TRAVEL_FAST);  
    //          ON EXIT:
      enter_stamp_state(StampState::INK_XY_WAIT);
      break;
    
      // Inking end move xy
    case StampState::INK_XY_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::INK_ZDOWN_START);
      break;

    case StampState::INK_ZDOWN_START:
    //          ACTION
      stamp_start_move_z(STAMP_POSITION_INK_Z_SAFE, STAMP_FEEDRATE_TRAVEL_Z);
      stamp_start_move_z(STAMP_POSITION_INK_Z, STAMP_FEEDRATE);
    //          ON EXIT  
      enter_stamp_state(StampState::INK_ZDOWN_WAIT);
      break;

    case StampState::INK_ZDOWN_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::INK_ZUP_START);
      break;

    case StampState::INK_ZUP_START:
    //          ACTION
      stamp_start_move_z(STAMP_POSITION_SAFE_Z, STAMP_FEEDRATE_TRAVEL_Z);
    //          ON EXIT  
      enter_stamp_state(StampState::INK_ZUP_WAIT);
      break;

    case StampState::INK_ZUP_WAIT:
    //          ON ENTER
      if (stamp_motion_done()) {
      //          ON EXIT
        docs_since_last_ink = 0;
        enter_stamp_state(StampState::MOVE_TO_STAMP_ZSAFE_START);
      }
      break;

    /*
      ----------------------
      MOVE TO STAMP TARGET
      ----------------------
    */
    case StampState::MOVE_TO_STAMP_ZSAFE_START:
    //          ON ENTER
      if (!stamp_documents_present()) {
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_NO_DOCUMENTS_2);
        return;
      }
    //          ACTION
      if (has_already_stamped)
      {
        stamp_start_move_z(stamp_trigger_z + STAMP_PROBE_PRE_STAMP__MOVE_OFFSET_Z, STAMP_FEEDRATE_TRAVEL_Z);
        //          ON EXIT 1
        enter_stamp_state(StampState::MOVE_TO_STAMP_ZSAFE_WAIT);
      }
      else
      {
        stamp_start_move_z(STAMP_POSITION_SAFE_Z, STAMP_FEEDRATE_TRAVEL_Z);
        //          ON EXIT 2
        enter_stamp_state(StampState::MOVE_TO_STAMP_ZSAFE_WAIT);
      }
      break;


    case StampState::MOVE_TO_STAMP_ZSAFE_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::MOVE_TO_STAMP_XY_START);
      break;


    case StampState::MOVE_TO_STAMP_XY_START:
    //          ON ENTER
      if (!stamp_documents_present()) {
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_NO_DOCUMENTS_2);
        return;
      }
    //          ACTION
      stamp_start_move_xy(stamp_x_pos, stamp_y_pos, STAMP_FEEDRATE_TRAVEL_FAST);
    //          ON EXIT
      enter_stamp_state(StampState::MOVE_TO_STAMP_XY_WAIT);
      break;

    case StampState::MOVE_TO_STAMP_XY_WAIT:
      if (stamp_motion_done()){
          enter_stamp_state(StampState::MOVE_TO_STAMP_ZREADY_START);
        }
      break;

    case StampState::MOVE_TO_STAMP_ZREADY_START:
    //          ACTION
      if (has_already_stamped) {
        stamp_start_move_z(stamp_trigger_z + STAMP_PROBE_SAFETY_OFFSET, STAMP_FEEDRATE);  // Safe Z position based on the sensor data from the previous stamp
      } else {
        stamp_start_move_z(STAMP_POSITION_SREADY_Z, STAMP_FEEDRATE_TRAVEL_Z);             // Safe Z position above logical max paper hight for first stamp
      }
    //          ON EXIT  
      enter_stamp_state(StampState::MOVE_TO_STAMP_ZREADY_WAIT);
      break;

    case StampState::MOVE_TO_STAMP_ZREADY_WAIT:
    //          ON ENTER
      if (stamp_motion_done()) {
      //          ON EXIT
        stamp_trigger_z = NAN;
        enter_stamp_state(StampState::STAMP_DOWN_START);
        }
      break;

    /*
      -------------
      STAMP DOWN
      -------------
    */
    case StampState::STAMP_DOWN_START:
    //          ON ENTER: 
      if (!stamp_documents_present()) {
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_NO_DOCUMENTS_2);
        return;
      }

      // Sensor triggered => Save position.
      if (stamp_probe_triggered()) {
        stamp_trigger_z = current_position.z;
        has_already_stamped = true;
        enter_stamp_state(StampState::STAMPING_START);
        return;
      }

      // Safety stop if sensor doesnt trigger.
      if (current_position.z <= STAMP_PROBE_MIN_Z) {
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_HOMING);
        return;
      }
    
    //          ACTIONS
        stamp_start_move_z(current_position.z - STAMP_PROBE_STEP_Z, STAMP_FEEDRATE_TRAVEL_Z); // Small incremental steps down until sensor triggers; Only on first document
    //          ON EXIT
        enter_stamp_state(StampState::STAMP_DOWN_WAIT);
      break;

    case StampState::STAMP_DOWN_WAIT:
    //          ON ENTER
      if (!stamp_motion_done())
        break;

      // Check after every move
      if (stamp_probe_triggered()) {
        stamp_trigger_z = current_position.z;
        has_already_stamped = true;
        enter_stamp_state(StampState::STAMPING_START);                                  // STATE EXIT
        return;
      }

      // safety
      if (current_position.z <= STAMP_PROBE_MIN_Z) {
        enter_stamp_state(StampState::ERROR);                                           // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_HOMING);
        return;
      }
    //          ON EXIT
      has_already_stamped = false; // If the sensor was not triggered by moving to z = stamp_trigger_z + STAMP_PROBE_SAFETY_OFFSET_Z then go again but with incremental steps as if it never stamped.
      enter_stamp_state(StampState::STAMP_DOWN_START);
      break;


    /*
      ----------------------
      STAMPING - STAMP ON PAPER (FOR ACCURATE CALC. STAMP MOUNT PLANS NEEDED)
      ----------------------
    */
    case StampState::STAMPING_START:
    //          ON ENTER:
      if (isnan(stamp_trigger_z)) {
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_HOMING);
        return;
      }
    //          ACTION:
      safe_delay(1000);
      stamp_start_move_z((stamp_trigger_z - STAMP_STAMPING_SENSOR_OFFST), STAMP_FEEDRATE);
    //          ON EXIT:  
      enter_stamp_state(StampState::STAMPING_WAIT);
      break;

    case StampState::STAMPING_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::STAMP_UP_START);                                  // STATE EXIT
      break;

    /*
      ----------
      STAMP UP
      ----------
    */
    case StampState::STAMP_UP_START:
    //          ACTION:
      stamp_start_move_z(STAMP_POSITION_SAFE_Z, STAMP_FEEDRATE_TRAVEL_Z);
    //          ON EXIT:  
      enter_stamp_state(StampState::STAMP_UP_WAIT);
      break;

    case StampState::STAMP_UP_WAIT:
    //          ON ENTER:
      if (stamp_motion_done()) {
        if (docs_since_last_ink >= 5)
        //          ON EXIT 1
          enter_stamp_state(StampState::INK_ZSAFE_START);                                  // STATE EXIT 1
        else
        //          ON EXIT 2
          enter_stamp_state(StampState::PAPER_OUT_MOVE_START);                                  // STATE EXIT 2
      }
      break;

    /*
      ----------
      PAPER OUT
      ----------
    */
    case StampState::PAPER_OUT_MOVE_START:
    //          ON ENTER:
      set_stamp_process_screen(STAMP_PROCESS_SCREEN_MANUAL_STOP_INFO);
    //          ACTION:
      stamp_start_move_y(STAMP_POSITION_PAPER_OUT_SAFE_STOP, STAMP_FEEDRATE_TRAVEL);
    //          ON EXIT:
      enter_stamp_state(StampState::PAPER_OUT_MOVE_WAIT);
      break;

    case StampState::PAPER_OUT_MOVE_WAIT:
      if (stamp_motion_done())
        enter_stamp_state(StampState::PAPER_OUT_POSITION);
      break;

    case StampState::PAPER_OUT_POSITION:
    //          ACTION:
      stamp_start_move_y(STAMP_POSITION_PAPER_OUT, STAMP_FEEDRATE_SAFE);
    //          ON EXIT:
      enter_stamp_state(StampState::PAPER_OUT);
      break;

    case StampState::PAPER_OUT_POSITION_WAIT:
      if(stamp_motion_done())
        enter_stamp_state(StampState::PAPER_OUT);
      break;

    case StampState::PAPER_OUT:
    //          ACTION
      thermalManager.set_fan_speed(0, 255);
    //          ON EXIT
      enter_stamp_state(StampState::PAPER_OUT_WAIT);    // Motor fan control implement
      break;
    
    case StampState::PAPER_OUT_WAIT:
    //          ON ENTER
      if (ELAPSED(millis(), state_entry_ms + 3000)){
      //          ACTION
        thermalManager.set_fan_speed(0, 0);
      //          ON EXIT
        enter_stamp_state(StampState::MOVE_BACK_Y_START);                                // STATE EXIT
      }
      break;

    /*
      ------------
      MOVE BACK Y
      ------------
    */
    case StampState::MOVE_BACK_Y_START:
      stamp_start_move_z(STAMP_POSITION_SAFE_Z, STAMP_FEEDRATE_TRAVEL_Z);
      enter_stamp_state(StampState::MOVE_BACK_Y_WAIT);
      break;

    case StampState::MOVE_BACK_Y_WAIT:
    //          ON ENTER
      if (!stamp_motion_done())
        break;
    //          ACTION
      #ifdef STAMP_SENSOR_SIMULATION  // For testing -> sucesful finish after 10 stamp runs
        if(stamped_documents >= 10){
          stamp_sensor.set_simulated_signal(false);
        }
      #endif

      if (docs_since_last_ink >= 5 && current_position.y >= 200.0){
        enter_stamp_state(StampState::RECHECK_DOCS);
      } else {
        stamp_start_move_y(stamp_y_pos, STAMP_FEEDRATE_TRAVEL);
      }
    //          ON EXIT
      enter_stamp_state(StampState::NEXT_OR_DONE);
      break;

    /*
      --------------
      NEXT / FINISH
      --------------
    */
    case StampState::NEXT_OR_DONE:
    //          ON ENTER
      if (!stamp_motion_done())
        break;

      stamped_documents++;
      docs_since_last_ink++;
      set_stamp_process_screen(STAMP_PROCESS_SCREEN_MAIN);

      if (!stamp_documents_present()) {
      //          ON EXIT 1 - FINISHED
        current_stamp_state = StampState::SUCCESS;
        state_entry_ms = millis();
        return;
      }

      if (stamp_reached_doc_limit()) {
      //          ON EXIT 2 - ERROR_DOC_LIMIT
        enter_stamp_state(StampState::ERROR);                   // STATE EXIT ERROR
        set_stamp_error(STAMP_ERROR_LIMIT);
        return;
      }
      //          ON EXIT 3 - RESTART
      enter_stamp_state(StampState::RECHECK_DOCS);
      break;

    case StampState::SUCCESS:
    //          ON ENTER
      docs_since_last_ink = 0;
      stamped_documents = 0;
      has_already_stamped = 0;
    //          ACTION
      stamp_start_move_z(STAMP_POSITION_HOME_Z, STAMP_FEEDRATE_TRAVEL_Z);
      safe_delay(1000);
      stamp_start_move_xy(STAMP_POSITION_HOME_X, STAMP_POSITION_HOME_Y, STAMP_FEEDRATE_TRAVEL_FAST);
    //          ON EXIT
      ui.goto_screen(menu_stamp_done);
      return;

    case StampState::ERROR:
    //          ON ENTER
      docs_since_last_ink = 0;
      stamped_documents = 0;
      has_already_stamped = false;
    //          ACTION
      stamp_start_move_z((current_position.z + 10.0f), STAMP_FEEDRATE_SAFE);
    //          ON EXIT
      queue.clear();
      quickstop_stepper();
      show_stamp_error(current_stamp_error);
      ui.refresh(LCDVIEW_REDRAW_NOW);
      return;
  }
}


/*
  ----------------------------
  Main Wait / Process Screen
  ----------------------------
*/

void wait_stamp_stamping() {
  // Process tick
  tick_stamp_process();
  // Draw current process screen
  START_MENU();
  switch (current_stamp_process_screen) {
    case STAMP_PROCESS_SCREEN_MAIN:
      STATIC_ITEM_F(F("Warte auf Abschluss"), SS_LEFT);
      STATIC_ITEM_F(F("des Stempelprozesses"), SS_LEFT);
      if (ui.should_draw()) {
        MenuEditItemBase::draw_edit_screen(F("Dokumente: "), i8tostr3rj(stamped_documents));
      }
      break;

    case STAMP_PROCESS_SCREEN_MANUAL_STOP_INFO:
      STATIC_ITEM_F(F("------------"), SS_CENTER);
      STATIC_ITEM_F(F("Manueller Abbruch"), SS_CENTER);
      STATIC_ITEM_F(F("3x klicken"), SS_CENTER);
      STATIC_ITEM_F(F("------------"), SS_CENTER);
      break;

    default:
      STATIC_ITEM_F(F("Unbekannter Fehler!"));
      ACTION_ITEM_F(F("Start"),[]{ ui.goto_screen(menu_stamp_start); });
      break;
  }
  END_MENU();
  // Manual abort
  if (manual_stop_three_click_abort()) {
    set_stamp_error(STAMP_ERROR_MANUAL_STOP);
    enter_stamp_state(StampState::ERROR);
    return;
  }
  //refresh ui
  ui.refresh(LCDVIEW_CALL_REDRAW_NEXT);
}