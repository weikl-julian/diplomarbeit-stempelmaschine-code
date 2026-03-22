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
#include "../../../module/motion.h"
#include "../../../module/planner.h"
#include "../../../gcode/parser.h"
#include "../../../gcode/queue.h"


static StampError current_stamp_error = STAMP_ERROR_NONE;

void show_stamp_error(const StampError error){
  current_stamp_error = error;
  ui.goto_screen(screen_stamp_error);
}

void screen_stamp_error(){
  START_MENU();

  STATIC_ITEM_F(F("Fehler"), SS_CENTER);

  switch (current_stamp_error) {
    case STAMP_ERROR_HOMING:
      STATIC_ITEM_F(F("Problem bei Vorbereitungen"));
      ACTION_ITEM_F(F("Start Screen"), []{ ui.goto_screen(menu_stamp_start); });
      break;
    case STAMP_ERROR_NO_DOCUMENTS:
      STATIC_ITEM_F(F("Keine Dokumente!"));
      STATIC_ITEM_F(F("Bitte einlegen"));
      ACTION_ITEM_F(F("Zurueck"), []{ ui.goto_screen(wait_stamp_documents); });
      break;

    case STAMP_ERROR_NO_DOCUMENTS_2:
      queue.clear();
      quickstop_stepper();
      STATIC_ITEM_F(F("Keine Dokumente"));
      STATIC_ITEM_F(F("Prozess neu starten."));
      ACTION_ITEM_F(F("Start Screen"), []{ ui.goto_screen(menu_stamp_start); });
      break;
    case STAMP_ERROR_POSITION:
      STATIC_ITEM_F(F("Position ausserhalb"));
      STATIC_ITEM_F(F("des Stempelbereichs!"));
      ACTION_ITEM_F(F("Zurueck"), []{ ui.goto_screen(menu_stamp_positioning); });
      break;

    case STAMP_ERROR_MANUAL_STOP:
      STATIC_ITEM_F(F("Prozess wurde manuell"));
      STATIC_ITEM_F(F("abgebrochen."));
      ACTION_ITEM_F(F("Start Screen"), []{ ui.goto_screen(menu_stamp_start); });
      break;
    case STAMP_ERROR_LIMIT:
      STATIC_ITEM_F(F("Prozess abgebrochen,"));
      STATIC_ITEM_F(F("Stempel-Limit erreicht."));
      ACTION_ITEM_F(F("Start Screen"), []{ ui.goto_screen(menu_stamp_start); });
      break;
    default:
      STATIC_ITEM_F(F("Unbekannter Fehler!"));
      ACTION_ITEM_F(F("Start Screen"), []{ ui.goto_screen(menu_stamp_start); });
      break;
  }

  END_MENU();

  ui.refresh(LCDVIEW_REDRAW_NOW);
}
#endif // HAS_MARLINUI_MENU

/*

if (ui.use_click()) {

    switch (current_stamp_error){
      case STAMP_ERROR_HOMING_S:
        ui.goto_screen(menu_stamp_start);
        break;
      case STAMP_ERROR_HOMING:
        ui.goto_screen(menu_stamp_start);
        break;
      case STAMP_ERROR_NO_DOCUMENTS:
        ui.goto_screen(wait_stamp_documents);
        break;
      case STAMP_ERROR_NO_DOCUMENTS_2:
        ui.goto_screen(menu_stamp_start);
        break;
      case STAMP_ERROR_POSITION:
        ui.goto_screen(menu_stamp_positioning);
        break;
      case STAMP_ERROR_MANUAL_STOP:
        ui.goto_screen(menu_stamp_start);
        break;
      case STAMP_ERROR_LIMIT:
        ui.goto_screen(menu_stamp_start);
        break;
      default:
        ui.goto_screen(menu_stamp_start);
        break;
    }
    return;
  }


  if (ui.should_draw()) {
    switch (current_stamp_error){
      case STAMP_ERROR_HOMING_S:
        MenuEditItemBase::draw_edit_screen(F("Es ist ein Problem bei den Vorbereitungen aufgetreten"), F("Start Screen"));
        break;
      case STAMP_ERROR_HOMING:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Homing fehlgeschlagen"), F("Klicken zum Neustart"));
        break;
      case STAMP_ERROR_NO_DOCUMENTS:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Keine Dokumente gefunden"), F("Klicken zum Neustart"));
        break;
      case STAMP_ERROR_NO_DOCUMENTS_2:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Dokumentenfehler"), F("Klicken zum Neustart"));
        break;
      case STAMP_ERROR_POSITION:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Ungueltige Position"), F("Klicken zur Korrektur"));
        break;
      case STAMP_ERROR_MANUAL_STOP:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Manuelle Bewegung gestoppt"), F("Klicken zum Neustart"));
        break;
      case STAMP_ERROR_LIMIT:
        MenuEditItemBase::draw_edit_screen(F("Fehler: Bewegungsgrenze erreicht"), F("Klicken zum Neustart"));
        break;
      default:
        MenuEditItemBase::draw_edit_screen(F("Unbekannter Fehler"), F("Klicken zum Neustart"));
        break;
    }
  }

*/