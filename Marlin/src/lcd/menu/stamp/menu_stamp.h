/*
    Diplomarbeit Automatisierte Stempelmaschine
    Marlin Firmware - Custom Stamp UI - Screen Declaration
*/

#pragma once

#include "../../../inc/MarlinConfigPre.h"


enum StampError : uint8_t {
  STAMP_ERROR_NONE = 0,
  STAMP_ERROR_HOMING,
  STAMP_ERROR_NO_DOCUMENTS,
  STAMP_ERROR_NO_DOCUMENTS_2,
  STAMP_ERROR_POSITION,
  STAMP_ERROR_MANUAL_STOP,
  STAMP_ERROR_LIMIT
};

void show_stamp_error(StampError error);
void screen_stamp_error();

void menu_stamp_start();
void menu_stamp_overview();
void wait_stamp_homing();
void menu_stamp_info();
void menu_stamp_settings();
void wait_stamp_documents();
void menu_stamp_positioning();
void wait_stamp_stamping();
void wait_stamp_stamping();
void wait_stamp_homing();
void wait_stamp_documents();

bool stamp_motion_done();
void init_stamp_system();
void stamp_start_move_xy(const float x, const float y, const float fr_mm_s);
void stamp_start_move_y(const float y, const float fr_mm_s);
void stamp_start_move_z(const float z, const float fr_mm_s);

extern float stamp_x_pos;
extern float stamp_y_pos;