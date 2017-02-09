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

=======================================================
Modified by Robert Bolling
January 2017 
for the Mobile Life IoT project
  - refactored to add new sensor lists from 
    "config_me.h", and "sensors.cpp"
=======================================================

======================================================================== */

#include "mbed.h" 
#include <cctype>
#include <string>
#include "config_me.h"
#include "sensors.h"
#include "cell_modem.h"
#include "hardware.h"

I2C i2c(PTC11, PTC10);    //SDA, SCL -- define the I2C pins being used - I2C1
MODSERIAL pc(USBTX, USBRX, 256, 256); // tx, rx with default tx, rx buffer sizes
MODSERIAL mdm(PTD3, PTD2, 4096, 4096);
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);


//********************************************************************************************************************************************
//* Create string with sensor readings that can be sent to flow as an HTTP get
//********************************************************************************************************************************************
K64F_Sensors_t  SENSOR_DATA =
{
    .Temperature        = "0",
    .Humidity           = "0",
    .AccelX             = "0",
    .AccelY             = "0",
    .AccelZ             = "0",
    .MagnetometerX      = "0",
    .MagnetometerY      = "0",
    .MagnetometerZ      = "0",
    .Temperature_Si7020 = "0",
    .Humidity_Si7020    = "0",
    .GPS_Satellites     = "0",
    .GPS_Latitude       = "0",
    .GPS_Longitude      = "0",
    .GPS_Altitude       = "0",
    .GPS_Speed          = "0",
    .GPS_Course         = "0",
    .Battery_Voltage    = "0",
    .Intrusion_Detected = "0"
};

void display_app_firmware_version(void)
{
    PUTS("\r\n\r\nApp Firmware: Release 1.0 - built: "__DATE__" "__TIME__"\r\n\r\n");
}

void GenerateModemString(char * modem_string)
{
    switch(iSensorsToReport)
    {
        case SHIELDTEMP_ACCELEROMETER_BATTERY:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        case SHIELDTEMP_ACCELEROMETER_BATTERY_INTRUSION:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s&intrusion=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, SENSOR_DATA.Intrusion_Detected, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        case SHIELDTEMP_ACCELEROMETER_BATTERY_GPS:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s&gps_satellites=%s&latitude=%s&longitude=%s&altitude=%s&speed=%s&course=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, SENSOR_DATA.GPS_Satellites,SENSOR_DATA.GPS_Latitude,SENSOR_DATA.GPS_Longitude,SENSOR_DATA.GPS_Altitude,SENSOR_DATA.GPS_Speed,SENSOR_DATA.GPS_Course, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        case SHIELDTEMP_ACCELEROMETER_BATTERY_EXTERNALTEMP:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s&temp2=%s&humidity2=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, SENSOR_DATA.Temperature_Si7020, SENSOR_DATA.Humidity_Si7020, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        case SHIELDTEMP_ACCELEROMETER_BATTERY_EXTERNALTEMP_GPS:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s&temp2=%s&humidity2=%s&gps_satellites=%s&latitude=%s&longitude=%s&altitude=%s&speed=%s&course=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, SENSOR_DATA.Temperature_Si7020, SENSOR_DATA.Humidity_Si7020, SENSOR_DATA.GPS_Satellites,SENSOR_DATA.GPS_Latitude,SENSOR_DATA.GPS_Longitude,SENSOR_DATA.GPS_Altitude,SENSOR_DATA.GPS_Speed,SENSOR_DATA.GPS_Course, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        case SHIELDTEMP_ACCELEROMETER_BATTERY_EXTERNALTEMP_GPS_INTRUSION:
        {
            sprintf(modem_string, "GET %s%s?serial=%s&temp=%s&humidity=%s&accelX=%s&accelY=%s&accelZ=%s&batt_volt=%s&temp2=%s&humidity2=%s&gps_satellites=%s&latitude=%s&longitude=%s&altitude=%s&speed=%s&course=%s&intrusion=%s %s%s\r\n\r\n", FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ, SENSOR_DATA.Battery_Voltage, SENSOR_DATA.Temperature_Si7020, SENSOR_DATA.Humidity_Si7020, SENSOR_DATA.GPS_Satellites,SENSOR_DATA.GPS_Latitude,SENSOR_DATA.GPS_Longitude,SENSOR_DATA.GPS_Altitude,SENSOR_DATA.GPS_Speed,SENSOR_DATA.GPS_Course, SENSOR_DATA.Intrusion_Detected, FLOW_URL_TYPE, MY_SERVER_URL);
            break;
        }
        default:
        {
            sprintf(modem_string, "Invalid sensor selected\r\n\r\n");
            break;
        }
    } //switch(iSensorsToReport)
} //GenerateModemString        
            
            
//Periodic timer
Ticker OneMsTicker;
volatile bool bTimerExpiredFlag = false;
int OneMsTicks = 0;
int iTimer1Interval_ms = 1000;
//********************************************************************************************************************************************
//* Periodic 1ms timer tick
//********************************************************************************************************************************************
void OneMsFunction() 
{
    OneMsTicks++;
    if ((OneMsTicks % iTimer1Interval_ms) == 0)
    {
        bTimerExpiredFlag = true;
    }            
} //OneMsFunction()

//********************************************************************************************************************************************
//* Set the RGB LED's Color
//* LED Color 0=Off to 7=White.  3 bits represent BGR (bit0=Red, bit1=Green, bit2=Blue) 
//********************************************************************************************************************************************
void SetLedColor(unsigned char ucColor)
{
    //Note that when an LED is on, you write a 0 to it:
    led_red = !(ucColor & 0x1); //bit 0
    led_green = !(ucColor & 0x2); //bit 1
    led_blue = !(ucColor & 0x4); //bit 2
} //SetLedColor()

//********************************************************************************************************************************************
//* Process the JSON response.  In this example we are only extracting a LED color. 
//********************************************************************************************************************************************
bool parse_JSON(char* json_string)
{
    char* beginquote;
    char token[] = "\"LED\":\"";
    beginquote = strstr(json_string, token );
    if ((beginquote != 0))
    {
        char cLedColor = beginquote[strlen(token)];
        PRINTF(GRN "LED Found : %c" DEF "\r\n", cLedColor);
        switch(cLedColor)
        {
            case 'O':
            { //Off
                SetLedColor(0);
                break;
            }
            case 'R':
            { //Red
                SetLedColor(1);
                break;
            }
            case 'G':
            { //Green
                SetLedColor(2);
                break;
            }
            case 'Y':
            { //Yellow
                SetLedColor(3);
                break;
            }
            case 'B':
            { //Blue
                SetLedColor(4);
                break;
            }
            case 'M':
            { //Magenta
                SetLedColor(5);
                break;
            }
            case 'T':
            { //Turquoise
                SetLedColor(6);
                break;
            }
            case 'W':
            { //White
                SetLedColor(7);
                break;
            }
            default:
            {
                break;
            }
        } //switch(cLedColor)
        return true;
    }
    else
    {
        return false;
    }
} //parse_JSON

int main() {
    static unsigned ledOnce = 0;
    //delay so that the debug terminal can open after power-on reset:
    wait (5.0);
    pc.baud(115200);
    
    display_app_firmware_version();
    
    PRINTF(GRN "Hello World from the Cellular IoT Kit!\r\n\r\n");

    //Initialize the I2C sensors that are present
    sensors_init();
    read_sensors();

    // Set LED to RED until init finishes
    SetLedColor(0x1); //Red
    // Initialize the modem
    PRINTF("\r\n");
    
//TODO: comment out these next two lines for local testing (no modem send) to keep from initializing the modem
    cell_modem_init();
    display_wnc_firmware_rev();

    // Set LED BLUE for partial init
    SetLedColor(0x4); //Blue

    //Create a 1ms timer tick function:
    iTimer1Interval_ms = SENSOR_UPDATE_INTERVAL_MS;
    OneMsTicker.attach(OneMsFunction, 0.001f) ;

    // Send and receive data perpetually
    while(1) {
        if  (bTimerExpiredFlag)
        {
            bTimerExpiredFlag = false;
            PRINTF("Sensor readings... \n\r");
            read_sensors(); //read available external sensors from external and on-board temp and motion sensors
            PRINTF("...end sensor readings \n\r\n\r");

            
          
            char modem_string[512];
            GenerateModemString(&modem_string[0]);
            PRINTF(modem_string);   //TODO: check what is being sent
// TODO: comment out to the next "TODO" for local testing to keep from sendng to modem
            char myJsonResponse[512];
            if (cell_modem_Sendreceive(&modem_string[0], &myJsonResponse[0]))
            {
                if (!ledOnce)
                {
                    ledOnce = 1;
                    SetLedColor(0x2); //Green
                }
                parse_JSON(&myJsonResponse[0]);
            }
// TODO: end testing comment out section


        } //bTimerExpiredFlag
    } //forever loop
}
