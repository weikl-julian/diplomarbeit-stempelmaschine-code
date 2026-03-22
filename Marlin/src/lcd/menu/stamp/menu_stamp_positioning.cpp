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
#include "stamp_sensor.h"
#include "stamp_config.h"
#include "../../../module/motion.h"
#include "../../../gcode/parser.h"

float stamp_x_pos = 0.0f;              // Variables for saving chosen stamp position x and y
float stamp_y_pos = 0.0f;
static bool stamp_position_valid_x = false;   // Flag to indicate if the position has been set and is valid
static bool stamp_position_valid_y = false;
static bool pressed = false;                  // Control variable needed for fine movement feature
static AxisEnum stamp_selected_axis = X_AXIS;

// ---- Moving the Axis ---- //
static void lcd_stamp_move_axis () {
  
  // --- Click Handling --- //
  if (ui.use_click()) {
    // -- Fine Movement -- //
    if(pressed == false) {
      ui.manual_move.menu_scale = 1.0f;
      pressed = true;
      return;
    }

    // -- Save Position -- //
    // Position erfolgreich gespeichert   |-> menu_stamp_positioning()
    // Position nicht gespeichert         |-> error_screen_scr5e()
    if (pressed == true) {
      // save position of x axis  
      if (stamp_selected_axis == X_AXIS && current_position.x <= STAMP_X_MAX && current_position.x >= STAMP_X_MIN ) {
        stamp_x_pos = current_position.x;
        stamp_position_valid_x = true;
      } // save position of y axis
      else if (stamp_selected_axis == Y_AXIS && current_position.y <= STAMP_Y_MAX && current_position.y >= STAMP_Y_MIN) {
        stamp_y_pos = current_position.y;
        stamp_position_valid_y = true;
      }
      else{ // Position outside of defined stamp area 
        show_stamp_error(STAMP_ERROR_POSITION); 

        // set control variables to false depending on the axis
        if (stamp_selected_axis == X_AXIS){
          stamp_position_valid_x = false;
        }
        else if (stamp_selected_axis == Y_AXIS){
          stamp_position_valid_y = false;
        }
        return;
      }
      ui.goto_screen(menu_stamp_positioning);
      return;
    } // <<< End of Position Saving >>>
    return;
  } // <<< End of Click Handling >>>


  // --- Manual Move Handling --- //
  if (ui.encoderPosition && !ui.manual_move.processing) {

    // --- Safety Buffer --- //
      // safety for x axis
    const float safety_x = ui.manual_move.axis_value(X_AXIS);
    if(pressed == false && stamp_selected_axis == X_AXIS){
      if (float(int32_t(ui.encoderPosition)) > 0 && safety_x >= STAMP_X_MAX){
        ui.manual_move.menu_scale = 1.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) < 0 && safety_x == STAMP_X_MAX){
        ui.manual_move.menu_scale = 10.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) > 0 && safety_x == STAMP_X_MIN){
        ui.manual_move.menu_scale = 10.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) < 0 && safety_x <= STAMP_X_MIN){
        ui.manual_move.menu_scale = 1.0f;
      }
      else if (safety_x >= STAMP_X_MIN && safety_x <= STAMP_X_MAX){
        ui.manual_move.menu_scale = 10.0f;
      }
    }
      // safety for y axis
    const float safety_y = ui.manual_move.axis_value(Y_AXIS);
    if(pressed == false && stamp_selected_axis == Y_AXIS){
      if (float(int32_t(ui.encoderPosition)) > 0 && safety_y >= STAMP_Y_MAX){
        ui.manual_move.menu_scale = 1.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) < 0 && safety_y == STAMP_Y_MAX){
        ui.manual_move.menu_scale = 10.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) > 0 && safety_y == STAMP_Y_MIN){
        ui.manual_move.menu_scale = 10.0f;
      }
      else if (float(int32_t(ui.encoderPosition)) < 0 && safety_y <= STAMP_Y_MIN){
        ui.manual_move.menu_scale = 1.0f;
      }
      else if (safety_y >= STAMP_Y_MIN && safety_y <= STAMP_Y_MAX){
        ui.manual_move.menu_scale = 10.0f;
      }
    }

      // Get Endstops
    float min, max;
    soft_endstop.get_manual_axis_limits(stamp_selected_axis, min, max);

      // Send movement to planner and execute
    const float diff = float(int32_t(ui.encoderPosition)) * ui.manual_move.menu_scale;  // Calculate distance of movement based on encoder position and menu scale
                                                                                             
    (void)ui.manual_move.apply_diff(stamp_selected_axis, diff, min, max);               // Apply the calculated variable to the selected axis
    ui.manual_move.soon(stamp_selected_axis);                                           // Queue the action
    ui.refresh(LCDVIEW_REDRAW_NOW);                                                     // Redraw the LCD screen to visualize the change
  }

  ui.encoderPosition = 0; 

  // --- Display Position --- //
  if (ui.should_draw()) {
    MenuEditItemBase::itemIndex = stamp_selected_axis;
    const float pos = ui.manual_move.axis_value(stamp_selected_axis);             //save current coordinates to pos variable
    if (stamp_selected_axis == X_AXIS) {                                      //Drawing for X-Axis
      MenuEditItemBase::draw_edit_screen(F("X-Position"), ftostr41sign(pos));     //Output saved pos variable
    }
    else if (stamp_selected_axis == Y_AXIS) {                                 //Drawing for Y-Axis
      MenuEditItemBase::draw_edit_screen(F("Y-Position"), ftostr41sign(pos));     //Output saved pos variable
    }
  }
} // <<< End of lcd_stamp_move_axis() >>>


// ---- Choosing Axis ---- //
static void goto_stamp_move_x(){
  stamp_selected_axis = X_AXIS;
  ui.manual_move.menu_scale = 10.0f;
  ui.goto_screen(lcd_stamp_move_axis);
  pressed = false;
}
static void goto_stamp_move_y(){
  stamp_selected_axis = Y_AXIS;
  ui.manual_move.menu_scale = 10.0f;
  ui.goto_screen(lcd_stamp_move_axis);
  pressed = false;
} // <<< End of goto functions for axis selection >>>


// ---- Check before Stamping ---- //
  // Check if saved x and y positions are valid before moving to stamping
  // Also check if the sensor is triggered to prevent stamping without documents
static void goto_stamp_stamping(){
  if (stamp_position_valid_x && stamp_position_valid_y && stamp_sensor.triggered()) {
    ui.goto_screen(wait_stamp_stamping);
  }
  else if (!stamp_position_valid_x || !stamp_position_valid_y){
    show_stamp_error(STAMP_ERROR_POSITION);
    return;
  }
  else if (!stamp_sensor.triggered()){
    show_stamp_error(STAMP_ERROR_NO_DOCUMENTS_2);
    return;
  }
} // <<< End of goto_stamp_stamping() >>>

// --------- Menu --------- //

  //menu for saving coordinates of each axis
void menu_stamp_positioning() {
  START_MENU();
  STATIC_ITEM_F(F("Waehle Stempelposition"), SS_CENTER);
  SUBMENU_F(F("Waehle X-Position"), []{ goto_stamp_move_x(); });
  SUBMENU_F(F("Waehle Y-Position"), []{ goto_stamp_move_y(); });
  ACTION_ITEM_F(F("Fertig"),[]{ ui.goto_screen(goto_stamp_stamping); });
  END_MENU();
} // <<< End of Menu >>>
