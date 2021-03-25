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
#include "config.h"
#include <TTGO.h>
#include "esp_task_wdt.h"

#include "tide.h"
#include "tide_fetch.h"
#include "tide_setup.h"
#include "images/resolve_icon.h"

#include "gui/app.h"
#include "gui/widget.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/keyboard.h"

#include "hardware/json_psram_allocator.h"
#include "hardware/wifictl.h"

EventGroupHandle_t tide_widget_event_handle = NULL;
TaskHandle_t _tide_widget_sync_Task;
void tide_widget_sync_Task(void *pvParameters);

tide_config_t tide_config;
tide_events_t tide_today;

uint32_t tide_app_setup_tile_num;

icon_t *tide_app = NULL;
icon_t *tide_widget = NULL;

static void enter_tide_widget_event_cb(lv_obj_t *obj, lv_event_t event);
bool tide_widget_wifictl_event_cb(EventBits_t event, void *arg);

LV_IMG_DECLARE(tide);
LV_IMG_DECLARE(high);
LV_IMG_DECLARE(low);

void tide_app_setup(void)
{
    tide_load_config();
    // init the event storage
    tide_today.valid = false;

    // get an app tile and copy mainstyle
    tide_app_setup_tile_num = mainbar_add_app_tile(1, 2, "Tides App");

    tide_setup_tile_setup(tide_app_setup_tile_num);
    tide_app = app_register("tide", &tide, enter_tide_widget_event_cb);

    // register app and widget icon
    tide_add_widget();

    tide_widget_event_handle = xEventGroupCreate();
    widget_set_label(tide_widget, "--:--");

    // wifictl_register_cb(WIFICTL_OFF | WIFICTL_CONNECT, tide_widget_wifictl_event_cb, "tide");
}

bool tide_widget_wifictl_event_cb(EventBits_t event, void *arg)
{
    switch (event)
    {
    case WIFICTL_CONNECT:
        tide_widget_sync_request();
        break;
    case WIFICTL_OFF:
        widget_hide_indicator(tide_widget);
        break;
    }
    return (true);
}

static void enter_tide_widget_event_cb(lv_obj_t *obj, lv_event_t event)
{
    tide_widget_sync_request();
}

void tide_add_widget(void)
{
    tide_widget = widget_register("tidea", &tide, enter_tide_widget_event_cb);
}

void tide_remove_widget(void)
{
    tide_widget = widget_remove(tide_widget);
}

void tide_jump_to_setup(void)
{
    statusbar_hide(true);
    mainbar_jump_to_tilenumber(tide_app_setup_tile_num, LV_ANIM_ON);
}

void tide_widget_sync_request(void)
{
    if (xEventGroupGetBits(tide_widget_event_handle) & TIDE_WIDGET_SYNC_REQUEST)
    {
        return;
    }
    else
    {
        xEventGroupSetBits(tide_widget_event_handle, TIDE_WIDGET_SYNC_REQUEST);
        widget_hide_indicator(tide_widget);
        xTaskCreate(tide_widget_sync_Task,    /* Function to implement the task */
                    "tide widget sync Task",  /* Name of the task */
                    12368,                     /* Stack size in words */
                    NULL,                     /* Task input parameter */
                    1,                        /* Priority of the task */
                    &_tide_widget_sync_Task); /* Task handle. */
    }
}

tide_config_t *tide_get_config(void)
{
    return (&tide_config);
}

void tide_widget_sync_Task(void *pvParameters)
{
    log_i("start tide widget task, heap: %d", ESP.getFreeHeap());
    if (tide_today.valid) {
        log_i("No need to fetch again: ");
        return;
    }

    vTaskDelay(250);

    if (xEventGroupGetBits(tide_widget_event_handle) & TIDE_WIDGET_SYNC_REQUEST)
    {
        uint32_t retval = tide_fetch(&tide_config, &tide_today);
        if (retval == 200)
        {
            log_i("Next Tide: %s %s", tide_today.first.EventType, tide_today.first.Time);
            widget_set_label(tide_widget, tide_today.first.Time);
            widget_set_icon(tide_widget, (lv_obj_t *)resolve_icon(tide_today.first.EventType));
            widget_set_indicator(tide_widget, ICON_INDICATOR_OK);
        }
        else
        {
            widget_set_indicator(tide_widget, ICON_INDICATOR_FAIL);
        }
        lv_obj_invalidate(lv_scr_act());
    }
    xEventGroupClearBits(tide_widget_event_handle, TIDE_WIDGET_SYNC_REQUEST);
    log_i("finish tide widget task, heap: %d", ESP.getFreeHeap());
    vTaskDelete(NULL);
}

void tide_save_config(void)
{
    fs::File file = SPIFFS.open(TIDE_JSON_CONFIG_FILE, FILE_WRITE);

    if (!file)
    {
        log_e("Can't open file: %s!", TIDE_JSON_CONFIG_FILE);
    }
    else
    {
        SpiRamJsonDocument doc(1000);

        doc["station"] = tide_config.station;
        doc["key"] = tide_config.key;

        if (serializeJsonPretty(doc, file) == 0)
        {
            log_e("Failed to write config file");
        }
        doc.clear();
    }
    file.close();
}

void tide_load_config(void)
{
    fs::File file = SPIFFS.open(TIDE_JSON_CONFIG_FILE, FILE_READ);
    if (!file)
    {
        log_e("Can't open file: %s!", TIDE_JSON_CONFIG_FILE);
    }
    else
    {
        int filesize = file.size();
        SpiRamJsonDocument doc(filesize * 4);

        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            log_e("update check deserializeJson() failed: %s", error.c_str());
        }
        else
        {
            strlcpy(tide_config.station, doc["station"], sizeof(tide_config.station));
            strlcpy(tide_config.key, doc["key"], sizeof(tide_config.key));
        }
        doc.clear();
    }
    file.close();
}
