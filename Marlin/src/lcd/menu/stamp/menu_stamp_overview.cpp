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


void menu_stamp_overview() {
  START_MENU();

  BACK_ITEM_F(F("Start Screen"));

  SUBMENU_F(F("Starte Stempelprozess"), wait_stamp_homing);
  
  SUBMENU_F(F("Infos"), menu_stamp_info);
  SUBMENU_F(F("Einstellungen"), menu_stamp_settings);


  END_MENU();
}




#endif // HAS_MARLINUI_MENU