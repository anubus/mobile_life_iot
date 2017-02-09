/* ===================================================================
Copyright © 2016, AVNET Inc.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, 
software distributed under the License is distributed on an 
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
either express or implied. See the License for the specific 
language governing permissions and limitations under the License.

======================================================================== */

#ifndef __SENSORS_H_
#define __SENSORS_H_

void sensors_init(void);
void read_sensors(void);

#define CTOF(x)  ((x)*1.8+32)

#define SENSOR_FIELD_LEN_LIMIT  32
typedef struct
{
    char  Temperature[SENSOR_FIELD_LEN_LIMIT];
    char  Humidity[SENSOR_FIELD_LEN_LIMIT];
    char  AccelX[SENSOR_FIELD_LEN_LIMIT];
    char  AccelY[SENSOR_FIELD_LEN_LIMIT];
    char  AccelZ[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerX[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerY[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerZ[SENSOR_FIELD_LEN_LIMIT];
    char  Temperature_Si7020[SENSOR_FIELD_LEN_LIMIT];
    char  Humidity_Si7020[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Satellites[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Latitude[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Longitude[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Altitude[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Speed[SENSOR_FIELD_LEN_LIMIT];
    char  GPS_Course[SENSOR_FIELD_LEN_LIMIT];
    char  Battery_Voltage [SENSOR_FIELD_LEN_LIMIT];
    char  Intrusion_Detected [SENSOR_FIELD_LEN_LIMIT];
} K64F_Sensors_t ;

extern K64F_Sensors_t  SENSOR_DATA;

#endif