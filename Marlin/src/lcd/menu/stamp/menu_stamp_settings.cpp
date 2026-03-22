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

static void open_original_marlin_ui() {
  ui.push_current_screen();
  ui.goto_screen(menu_main);
}

void menu_stamp_settings() {
  START_MENU();

  BACK_ITEM_F(F("Uebersicht"));
  ACTION_ITEM_F(F("Original Marlin UI"), open_original_marlin_ui);

  END_MENU();
}




#endif // HAS_MARLINUI_MENU