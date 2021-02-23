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
#ifndef _TIDE_FETCH_H
    #define _TIDE_FETCH_H
    // https://admiraltyapi.azure-api.net/uktidalapi/api/V1/Stations/0490/TidalEvents?duration=2
    #define ADM_HOST    "admiraltyapi.azure-api.net"
    #define ADM_PORT    443
    #define JSON_DOC_SIZE 2048

    int tide_fetch( tide_config_t * tide_config, tide_events_t * tide_events );

#endif // _TIDE_FETCH_H