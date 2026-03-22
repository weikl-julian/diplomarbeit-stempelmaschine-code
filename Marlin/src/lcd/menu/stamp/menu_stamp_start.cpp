/*
    Diplomarbeit Automatisierte Stempelmaschine
    Marlin Firmware - Custom Stamp UI - Start Screen
*/
#include "../../../inc/MarlinConfigPre.h"

#if HAS_MARLINUI_MENU

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
#include "stamp_config.h"

void menu_stamp_overview();
void init_stamp_system() {
  stamp_sensor.init();
}

void menu_stamp_start() {
  START_MENU();


  STATIC_ITEM_F(F("Automatisierte"), SS_CENTER);
  STATIC_ITEM_F(F("Stempelmaschine"), SS_CENTER);

  // “Button”-Look: invertierte Zeile
  //STATIC_ITEM_F(F("START"), SS_CENTER | SS_INVERT);

  // Klick irgendwo im Menü macht hier NICHT automatisch was.
  // Wenn du willst, dass Encoder-Klick auf START geht, nimm ACTION_ITEM:
  SUBMENU_F(F("START"), menu_stamp_overview);

  END_MENU();
}

#endif // HAS_MARLINUI_MENU

/*
void lcd_move_axis(const AxisEnum axis) {
  if (ui.use_click()) return ui.goto_previous_screen_no_defer();
  if (ui.encoderPosition && !ui.manual_move.processing) {
    // Get motion limit from software endstops, if any
    float min, max;
    soft_endstop.get_manual_axis_limits(axis, min, max); //wichtig

    // Delta limits XY based on the current offset from center
    // This assumes the center is 0,0
    #if ENABLED(DELTA)
      if (axis != Z_AXIS) {
        max = SQRT(sq((float)(DELTA_PRINTABLE_RADIUS)) - sq(current_position[Y_AXIS - axis])); // (Y_AXIS - axis) == the other axis
        min = -max;
      }
    #endif

    // Get the new position
    const float diff = float(int32_t(ui.encoderPosition)) * ui.manual_move.menu_scale;
    (void)ui.manual_move.apply_diff(axis, diff, min, max);
    ui.manual_move.soon(axis);
    ui.refresh(LCDVIEW_REDRAW_NOW);
  }
  ui.encoderPosition = 0;
  if (ui.should_draw()) {
    MenuEditItemBase::itemIndex = axis;
    const float pos = ui.manual_move.axis_value(axis);
    if (parser.using_inch_units()) {
      const float imp_pos = LINEAR_UNIT(pos);
      MenuEditItemBase::draw_edit_screen(GET_TEXT_F(MSG_MOVE_N), ftostr63(imp_pos));
    }
    else
      MenuEditItemBase::draw_edit_screen(GET_TEXT_F(MSG_MOVE_N), ui.manual_move.menu_scale >= 0.1f ? (LARGE_AREA_TEST ? ftostr51sign(pos) : ftostr41sign(pos)) : ftostr63(pos));
  }
}
*/



/*
// Minimaler Startscreen: zeigt Text, Klick toggelt eine zweite Zeile
void menu_stamp_start() {
  static bool clicked_once = false;

  // Klick abfangen (einmaliges Event)
  if (ui.use_click())
    clicked_once = !clicked_once;

  // Für grafische Displays (ST7920) zeichnet Marlin seitenweise (Paged Drawing).
  // PAGE_CONTAINS schützt davor, außerhalb der aktuellen Page zu zeichnen.
  if (PAGE_CONTAINS(0, LCD_PIXEL_HEIGHT - 1)) {
    u8g.setColorIndex(1); // Farbe auf "weiß" setzen (1 = Pixel an, 0 = Pixel aus)
  }

    //  --  TITEL  START SCREEN  --  //
    u8g.setFont(MENU_FONT_NAME); // In u8g die Menu Font speichern
    //constexpr u8g_uint_t width = LCD_PIXEL_WIDTH; // u8g_uint_t da es speicher spart -> nur 8 bit, reicht für 128 pixel breite


    // Pixelbreite der verwendeten Texte abspeichern:
    const u8g_uint_t pixel_width_title_p1 = u8g_uint_t((sizeof("Automatisierte") - 1) * (MENU_FONT_WIDTH)), 
                     pixel_width_title_p2 = u8g_uint_t((sizeof("Stempelmaschine") - 1) * (MENU_FONT_WIDTH)),
                     pixel_width_start    = u8g_uint_t((sizeof("Start") - 1) * (MENU_FONT_WIDTH));
    
    // X offset berechnen - Notion noch ein bild für erklärung.
    const u8g_uint_t title_p1_x = (LCD_PIXEL_WIDTH - pixel_width_title_p1) / 2,
                     title_p2_x = (LCD_PIXEL_WIDTH - pixel_width_title_p2) / 2,
                     start_x    = (LCD_PIXEL_WIDTH - pixel_width_start)    / 2;

    lcd_put_u8str(title_p1_x, 14, F("Automatisierte"));
    lcd_put_u8str(title_p2_x, 30, F("Stempelmaschine"));
    lcd_put_u8str(start_x, 50, F("Start"));
    

  // Regelmäßig neu zeichnen lassen
  ui.refresh(LCDVIEW_CALL_REDRAW_NEXT);
}
*/


//Old Code:
/*
if (clicked_once)
      lcd_put_u8str(5, 46, F("CLICK OK"));
    else
      lcd_put_u8str(5, 46, F("CLICK TO TOGGLE"));
*/
