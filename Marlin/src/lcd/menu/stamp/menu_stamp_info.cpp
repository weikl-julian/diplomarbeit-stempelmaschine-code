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



void menu_stamp_info(){
  START_MENU();

  BACK_ITEM_F(F("Uebersicht")); 

  STATIC_ITEM_F(F("Hier stehen Infos"));

  END_MENU();
}






#endif // HAS_MARLINUI_MENU