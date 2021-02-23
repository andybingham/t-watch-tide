/****************************************************************************
 *   Tu May 22 21:23:51 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <config.h>

#include "tide.h"
#include "tide_fetch.h"
#include "tide_setup.h"

#include "gui/mainbar/mainbar.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/statusbar.h"
#include "gui/keyboard.h"

#include "hardware/motor.h"
#include "hardware/json_psram_allocator.h"

lv_obj_t *tide_setup_tile = NULL;
lv_style_t tide_setup_style;
uint32_t tide_setup_tile_num;

lv_obj_t *tide_key_textfield = NULL;
lv_obj_t *tide_station_textfield = NULL;
lv_style_t tide_widget_setup_style;

LV_IMG_DECLARE(exit_32px);

static void tide_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void tide_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event );
static void exit_tide_widget_setup_event_cb( lv_obj_t * obj, lv_event_t event );

void tide_setup_tile_setup( uint32_t tile_num ) {

    tide_config_t *tide_config = tide_get_config();

    tide_setup_tile_num = tile_num;
    tide_setup_tile = mainbar_get_tile_obj( tide_setup_tile_num );

    lv_style_copy( &tide_setup_style, mainbar_get_style() );
    lv_style_set_bg_color( &tide_setup_style, LV_OBJ_PART_MAIN, LV_COLOR_GRAY);
    lv_style_set_bg_opa( &tide_setup_style, LV_OBJ_PART_MAIN, LV_OPA_100);
    lv_style_set_border_width( &tide_setup_style, LV_OBJ_PART_MAIN, 0);
    lv_obj_add_style( tide_setup_tile, LV_OBJ_PART_MAIN, &tide_setup_style );

    lv_obj_t *exit_btn = lv_imgbtn_create( tide_setup_tile, NULL);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src( exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style( exit_btn, LV_IMGBTN_PART_MAIN, &tide_setup_style );
    lv_obj_align( exit_btn, tide_setup_tile, LV_ALIGN_IN_TOP_LEFT, 10, 10 );
    lv_obj_set_event_cb( exit_btn, exit_tide_widget_setup_event_cb );
    
    lv_obj_t *exit_label = lv_label_create( tide_setup_tile, NULL);
    lv_obj_add_style( exit_label, LV_OBJ_PART_MAIN, &tide_setup_style  );
    lv_label_set_text( exit_label, "open tide setup");
    lv_obj_align( exit_label, exit_btn, LV_ALIGN_OUT_RIGHT_MID, 5, 0 );

    lv_obj_t *tide_key_cont = lv_obj_create( tide_setup_tile, NULL );
    lv_obj_set_size(tide_key_cont, lv_disp_get_hor_res( NULL ) , 35);
    lv_obj_add_style( tide_key_cont, LV_OBJ_PART_MAIN, &tide_setup_style  );
    lv_obj_align( tide_key_cont, tide_setup_tile, LV_ALIGN_IN_TOP_MID, 0, 45 );
    lv_obj_t *tide_key_label = lv_label_create( tide_key_cont, NULL);
    lv_obj_add_style( tide_key_label, LV_OBJ_PART_MAIN, &tide_setup_style  );
    lv_label_set_text( tide_key_label, "Key");
    lv_obj_align( tide_key_label, tide_key_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    tide_key_textfield = lv_textarea_create( tide_key_cont, NULL);
    lv_textarea_set_text( tide_key_textfield, tide_config->key );
    lv_textarea_set_pwd_mode( tide_key_textfield, false);
    lv_textarea_set_one_line( tide_key_textfield, true);
    lv_textarea_set_cursor_hidden( tide_key_textfield, true);
    lv_obj_set_width( tide_key_textfield, LV_HOR_RES /4 * 3 );
    lv_obj_align( tide_key_textfield, tide_key_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( tide_key_textfield, tide_textarea_event_cb );

    lv_obj_t *tide_station_cont = lv_obj_create( tide_setup_tile, NULL );
    lv_obj_set_size(tide_station_cont, lv_disp_get_hor_res( NULL ) , 35);
    lv_obj_add_style( tide_station_cont, LV_OBJ_PART_MAIN, &tide_setup_style  );
    lv_obj_align( tide_station_cont, tide_setup_tile, LV_ALIGN_IN_TOP_MID, 0, 90 );
    lv_obj_t *tide_station_label = lv_label_create( tide_station_cont, NULL);
    lv_obj_add_style( tide_station_label, LV_OBJ_PART_MAIN, &tide_setup_style  );
    lv_label_set_text( tide_station_label, "Station");
    lv_obj_align( tide_station_label, tide_station_cont, LV_ALIGN_IN_LEFT_MID, 5, 0 );
    tide_station_textfield = lv_textarea_create( tide_station_cont, NULL);
    lv_textarea_set_text( tide_station_textfield, tide_config->station );
    lv_textarea_set_pwd_mode( tide_station_textfield, false);
    lv_textarea_set_one_line( tide_station_textfield, true);
    lv_textarea_set_cursor_hidden( tide_station_textfield, true);
    lv_obj_set_width( tide_station_textfield, LV_HOR_RES /4 * 3 );
    lv_obj_align( tide_station_textfield, tide_station_cont, LV_ALIGN_IN_RIGHT_MID, -5, 0 );
    lv_obj_set_event_cb( tide_station_textfield, tide_num_textarea_event_cb );

}

static void tide_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        keyboard_set_textarea( obj );
    }
}

static void tide_num_textarea_event_cb( lv_obj_t * obj, lv_event_t event ) {
    if( event == LV_EVENT_CLICKED ) {
        num_keyboard_set_textarea( obj );
    }
}

static void exit_tide_widget_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):           keyboard_hide();
                                            tide_config_t *tide_config = tide_get_config();
                                            strlcpy( tide_config->key, lv_textarea_get_text( tide_key_textfield ), sizeof( tide_config->key ) );
                                            strlcpy( tide_config->station, lv_textarea_get_text( tide_station_textfield ), sizeof( tide_config->station ) );
                                            tide_save_config();
                                            mainbar_jump_to_maintile( LV_ANIM_OFF );
                                            break;
    }
}