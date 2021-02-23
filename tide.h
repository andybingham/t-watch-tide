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
#ifndef _TIDE_H
    #define _TIDE_H

    #include <TTGO.h>

    #define TIDE_CONFIG_FILE             "/tide.cfg"
    #define TIDE_JSON_CONFIG_FILE        "/tide.json"

    #define TIDE_WIDGET_SYNC_REQUEST    _BV(0)

    typedef struct {
        char station[5] = "0490";
        char key[34] = "5d82d06a824547cd88ea8dd747daf578";        
    } tide_config_t;

    typedef struct {
        char EventType[10] = "";
        char DateTime[20] = "";
        char Height[7] = "";
        char Time[6] = "";
    } tide_event_t;

    typedef struct {
        bool valid = false;
        int8_t count = 0;
        tide_event_t first;
        tide_event_t events[4];
    } tide_events_t;

    void tide_app_setup( void );

    tide_config_t *tide_get_config( void );

    void tide_jump_to_setup( void );

    void tide_widget_sync_request( void );

    void tide_save_config( void );

    void tide_load_config( void );

    void tide_add_widget( void );
    
    void tide_remove_widget( void );
    
#endif
