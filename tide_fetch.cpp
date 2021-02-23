/****************************************************************************
 *   July 23 00:23:05 2020
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
#include "HTTPClient.h"

#include "tide.h"
#include "tide_fetch.h"
#include "rootca.h"
#include "ArduinoJson.h"

#include "hardware/powermgm.h"

void JSONtoEvent(JsonVariant doc, tide_event_t *o)
{
    strlcpy(o->DateTime, doc["DateTime"], 19);
    strlcpy(o->EventType, doc["EventType"], 9);
    double height = doc["Height"];
    sprintf(o->Height, "%1.2f m", height);
    // copy the time to its own string
    for (int i = 0; i < 5; i++)
    {
        o->Time[i] = o->DateTime[11 + i];
    }
    log_i("Event %s -> %s @ %s Height %s", o->DateTime, o->EventType, o->Time, o->Height);
}


int tide_fetch(tide_config_t *tide_config, tide_events_t *tide_events)
{
    char uri[100] = "";
    int httpcode = -1;
    tide_events->valid = false;

    snprintf(uri, sizeof(uri), "/uktidalapi/api/V1/Stations/%s/TidalEvents?duration=1", tide_config->station);
    log_i("%s", uri);
    HTTPClient client;
    // client.useHTTP10(true);
    client.begin("admiraltyapi.azure-api.net", 443, uri, root_ca);
    client.addHeader("Ocp-Apim-Subscription-Key", tide_config->key);
    httpcode = client.GET();
    log_i("Back from call");
    if (httpcode != 200)
    {
        log_e("HTTPClient error %d", httpcode);
        client.end();
        return (-1);
    }
    log_i("Decoding");
    StaticJsonDocument<JSON_DOC_SIZE> doc;
    DeserializationError error = deserializeJson(doc, client.getStream());
    client.end();
    log_i("Closed connections");

    if (error)
    {
        log_e("tide deserializeJson() failed: %s", error.c_str());
        doc.clear();
        return (-1);
    }

    // unpack results
    int8_t i = 0;
    for (JsonVariant item : doc.as<JsonArray>())
    {
        log_i("item: %d", i);
        if (tide_events->valid) {
            JSONtoEvent(item,&tide_events->events[i-1]);
        } else {
            JSONtoEvent(item,&tide_events->first);
            tide_events->valid = true;
        }
        i++;
    }

    tide_events->count = i;
    doc.clear();
    log_i("Count: %d", tide_events->count);
    return (httpcode);
}
