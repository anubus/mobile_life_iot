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
  - refactored Si7020 to Si7021 sensor
  - added voltage and intrusion sensor
  - some other clean up including removing unused code
    for virtual sensors and the Si1145
=======================================================

======================================================================== */

#include "mbed.h"
#include "sensors.h"
#include "hardware.h"
#include "config_me.h"
#include "FXOS8700CQ.h"
#include "HTS221.h"
#include "xadow_gps.h"
#include <string>

#define Si7020_PMOD_I2C_ADDR   0x80 //this is for 7-bit I2C addr 0x4 for the Si7021 external sensor

// Storage for the data from the motion sensor
SRAWDATA accel_data;
SRAWDATA magn_data;
//InterruptIn fxos_int1(PTC6); // unused, common with SW2 on FRDM-K64F
InterruptIn fxos_int2(PTC13); // should just be the Data-Ready interrupt

bool fxos_int2_triggered = false;
void trigger_fxos_int2(void)
{
    fxos_int2_triggered = true;

}

#define VOLTS_SCALE 18.6 //scale battery voltage measurement based on R1 and R2
AnalogIn Analog0(PTC0); //  Had to use J1-11 for PTC0 to deconflict from wnc shield pins


/*------------------------------------------------------------------------------
 * Perform I2C single read.
 *------------------------------------------------------------------------------*/
unsigned char I2C_ReadSingleByte(unsigned char ucDeviceAddress)
{
    char rxbuffer [1];
    i2c.read(ucDeviceAddress, rxbuffer, 1 );
    return (unsigned char)rxbuffer[0];
} //I2C_ReadSingleByte()

/*------------------------------------------------------------------------------
 * Perform I2C single read from address.
 *------------------------------------------------------------------------------*/
unsigned char I2C_ReadSingleByteFromAddr(unsigned char ucDeviceAddress, unsigned char Addr)
{
    char txbuffer [1];
    char rxbuffer [1];
    txbuffer[0] = (char)Addr;
    i2c.write(ucDeviceAddress, txbuffer, 1 );
    i2c.read(ucDeviceAddress, rxbuffer, 1 );
    return (unsigned char)rxbuffer[0];
} //I2C_ReadSingleByteFromAddr()

/*------------------------------------------------------------------------------
 * Perform I2C read of more than 1 byte.
 *------------------------------------------------------------------------------*/
int I2C_ReadMultipleBytes(unsigned char ucDeviceAddress, char *ucData, unsigned char ucLength)
{
    int status;
    status = i2c.read(ucDeviceAddress, ucData, ucLength);
    return status;
} //I2C_ReadMultipleBytes()

/*------------------------------------------------------------------------------
 * Perform I2C write of a single byte.
 *------------------------------------------------------------------------------*/
int I2C_WriteSingleByte(unsigned char ucDeviceAddress, unsigned char Data, bool bSendStop)
{
    int status;
    char txbuffer [1];
    txbuffer[0] = (char)Data; //data
    status = i2c.write(ucDeviceAddress, txbuffer, 1, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteSingleByte()

/*------------------------------------------------------------------------------
 * Perform I2C write of 1 byte to an address.
 *------------------------------------------------------------------------------*/
int I2C_WriteSingleByteToAddr(unsigned char ucDeviceAddress, unsigned char Addr, unsigned char Data, bool bSendStop)
{
    int status;
    char txbuffer [2];
    txbuffer[0] = (char)Addr; //address
    txbuffer[1] = (char)Data; //data
    //status = i2c.write(ucDeviceAddress, txbuffer, 2, false); //stop at end
    status = i2c.write(ucDeviceAddress, txbuffer, 2, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteSingleByteToAddr()

/*------------------------------------------------------------------------------
 * Perform I2C write of more than 1 byte.
 *------------------------------------------------------------------------------*/
int I2C_WriteMultipleBytes(unsigned char ucDeviceAddress, char *ucData, unsigned char ucLength, bool bSendStop)
{
    int status;
    status = i2c.write(ucDeviceAddress, ucData, ucLength, !bSendStop); //true: do not send stop
    return status;
} //I2C_WriteMultipleBytes()

//********************************************************************************************************************************************
//* Battery Voltage
//********************************************************************************************************************************************

void Read_Battery_Volts(void)
{
        float Volts;
        Volts = Analog0 * VOLTS_SCALE;
        PRINTF("Voltage: %0.3f Volts \r\n", Volts); 
        sprintf(SENSOR_DATA.Battery_Voltage, "%0.2f", Volts);
}

//********************************************************************************************************************************************
//* Si7020/Si7021 temperature & humidity sensor
//********************************************************************************************************************************************

bool bSi7020_present = false;
void Init_Si7020(void)
{
    char SN_7020 [8];
    //SN part 1:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0xFA, 0x0F, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &SN_7020[0], 4);

    //SN part 1:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0xFC, 0xC9, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &SN_7020[4], 4);

    char Ver_7020 [2];
    //FW version:
    I2C_WriteSingleByteToAddr(Si7020_PMOD_I2C_ADDR, 0x84, 0xB8, false);
    I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Ver_7020[0], 2);

    if (SN_7020[4] != 0x15)  //Si7021 this is 0x15, for Si7020 this is 0x14
    {
        bSi7020_present = false;
        PRINTF("Si7020 sensor not found. SN=0x%02X Si7020addr=0x%02X \r\n", SN_7020[4], Si7020_PMOD_I2C_ADDR  );  //TODO: take out SN print
    }
    else 
    {
        bSi7020_present = true;
        PRINTF("Si7020 SN = 0x%02X%02X%02X%02X%02X%02X%02X%02X \r\n", SN_7020[0], SN_7020[1], SN_7020[2], SN_7020[3], SN_7020[4], SN_7020[5], SN_7020[6], SN_7020[7]);
        PRINTF("Si7020 Version# = 0x%02X \r\n", Ver_7020[0]);
    } //bool bSi7020_present = true

} //Init_Si7020()

void Read_Si7020(void)
{
    if (bSi7020_present)
    {   
        char Humidity [2];
        char Temperature [2];
        //Command to measure humidity (temperature also gets measured):
        I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xF5, false); //no hold, must do dummy read
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Humidity[0], 1); //dummy read, should get an nack until it is done
        wait (0.05); //wait for measurement.  Can also keep reading until no NACK is received
        //I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xE5, false); //Hold mod, the device does a clock stretch on the read until it is done (crashes the I2C bus...
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Humidity[0], 2); //read humidity
        //PRINTF("Read Si7020 Humidity = 0x%02X%02X\n", Humidity[0], Humidity[1]);
        int rh_code = (Humidity[0] << 8) + Humidity[1];
        float fRh = (125.0*rh_code/65536.0) - 6.0; //from datasheet
        PRINTF("Si7020 Humidity = %0.1f %% \r\n", fRh); //double % sign for escape //PRINTF("%*.*f\n", myFieldWidth, myPrecision, myFloatValue);
        sprintf(SENSOR_DATA.Humidity_Si7020, "%0.1f", fRh);
        
        //Command to read temperature when humidity is already done:
        I2C_WriteSingleByte(Si7020_PMOD_I2C_ADDR, 0xE0, false);
        I2C_ReadMultipleBytes(Si7020_PMOD_I2C_ADDR, &Temperature[0], 2); //read temperature
        //PRINTF("Read Si7020 Temperature = 0x%02X%02X\n", Temperature[0], Temperature[1]);
        int temp_code = (Temperature[0] << 8) + Temperature[1];
        float fTemp = (175.72*temp_code/65536.0) - 46.85; //from datasheet in Celcius
        PRINTF("Si7020 Temperature = %0.1f deg F \r\n", CTOF(fTemp));
        sprintf(SENSOR_DATA.Temperature_Si7020, "%0.1f", CTOF(fTemp));
    } //bool bSi7020_present = true

} //Read_Si7020()


//********************************************************************************************************************************************
//* Read the FXOS8700CQ - 6-axis combo Sensor Accelerometer and Magnetometer
//********************************************************************************************************************************************
bool bMotionSensor_present = false;
void Init_motion_sensor()
{
    // Note: this class is instantiated here because if it is statically declared, the cellular shield init kills the I2C bus...
    // Class instantiation with pin names for the motion sensor on the FRDM-K64F board:
    FXOS8700CQ fxos(PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // SDA, SCL, (addr << 1)
    int iWhoAmI = fxos.get_whoami();

    PRINTF("FXOS8700CQ WhoAmI = %X\r\n", iWhoAmI);
    // Iterrupt for active-low interrupt line from FXOS
    // Configured with only one interrupt on INT2 signaling Data-Ready
    //fxos_int2.fall(&trigger_fxos_int2);
    if (iWhoAmI != 0xC7)
    {
        bMotionSensor_present = false;
        PRINTF("FXOS8700CQ motion sensor not found\r\n");
    }
    else
    {
        bMotionSensor_present = true;
        fxos.enable();
    }
} //Init_motion_sensor()

void Read_motion_sensor()
{
    // Note: this class is instantiated here because if it is statically declared, the cellular shield init kills the I2C bus...
    // Class instantiation with pin names for the motion sensor on the FRDM-K64F board:
    FXOS8700CQ fxos(PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // SDA, SCL, (addr << 1)
    if (bMotionSensor_present)
    {
        fxos.enable();
        fxos.get_data(&accel_data, &magn_data);
        //PRINTF("Roll=%5d, Pitch=%5d, Yaw=%5d;\r\n", magn_data.x, magn_data.y, magn_data.z);
        sprintf(SENSOR_DATA.MagnetometerX, "%5d", magn_data.x);
        sprintf(SENSOR_DATA.MagnetometerY, "%5d", magn_data.y);
        sprintf(SENSOR_DATA.MagnetometerZ, "%5d", magn_data.z);
    
        //Try to normalize (/2048) the values so they will match the eCompass output:
        float fAccelScaled_x, fAccelScaled_y, fAccelScaled_z;
        fAccelScaled_x = (accel_data.x/2048.0);
        fAccelScaled_y = (accel_data.y/2048.0);
        fAccelScaled_z = (accel_data.z/2048.0);
        PRINTF("Acc: X=%2.3f Y=%2.3f Z=%2.3f;\r\n", fAccelScaled_x, fAccelScaled_y, fAccelScaled_z);
        sprintf(SENSOR_DATA.AccelX, "%2.3f", fAccelScaled_x);
        sprintf(SENSOR_DATA.AccelY, "%2.3f", fAccelScaled_y);
        sprintf(SENSOR_DATA.AccelZ, "%2.3f", fAccelScaled_z);
    } //bMotionSensor_present
} //Read_motion_sensor()


//********************************************************************************************************************************************
//* Read the HTS221 temperature & humidity sensor on the Cellular Shield
//********************************************************************************************************************************************
// These are to be built on the fly
string my_temp;
string my_humidity;
HTS221 hts221;

//#define CTOF(x)  ((x)*1.8+32)
bool bHTS221_present = false;
void Init_HTS221()
{
    int i;
    void hts221_init(void);
    i = hts221.begin();
    if (i)
    {
        bHTS221_present = true;
        PRINTF(BLU "HTS221 Detected (0x%02X)\n\r",i);
        PRINTF("  Temp  is: %0.2f F \n\r",CTOF(hts221.readTemperature()));
        PRINTF("  Humid is: %02d %%\n\r",hts221.readHumidity());
    }
    else
    {
        bHTS221_present = false;
        PRINTF(RED "HTS221 NOT DETECTED!\n\r");
    }
} //Init_HTS221()

void Read_HTS221()
{
    if (bHTS221_present)
    {
        sprintf(SENSOR_DATA.Temperature, "%0.1f", CTOF(hts221.readTemperature()));
        PRINTF("HTS221 Temp  is: %0.1f F \n\r",CTOF(hts221.readTemperature()));
        sprintf(SENSOR_DATA.Humidity, "%02d", hts221.readHumidity());
        PRINTF("HTS221 Humid is: %02d %%\n\r",hts221.readHumidity());     
        
    } //bHTS221_present
} //Read_HTS221()

//********************************************************************************************************************************************
//* Read the xadow gps module connedted to i2c1
//********************************************************************************************************************************************

bool bGPS_present = false;
void Init_GPS(void)
{
    char scan_id[GPS_SCAN_SIZE+2]; //The first two bytes are the response length (0x00, 0x04)
    I2C_WriteSingleByte(GPS_DEVICE_ADDR, GPS_SCAN_ID, true); //no hold, must do read

    unsigned char i;
    for(i=0;i<(GPS_SCAN_SIZE+2);i++)
    {
        scan_id[i] = I2C_ReadSingleByte(GPS_DEVICE_ADDR);
    }

    if(scan_id[5] != GPS_DEVICE_ID)
    {
        bGPS_present = false;
        PRINTF("Xadow GPS not found \r\n");
    }
    else 
    {
        bGPS_present = true;
        PRINTF("Xadow GPS Scan ID response = 0x%02X%02X (length), 0x%02X%02X%02X%02X\r\n", scan_id[0], scan_id[1], scan_id[2], scan_id[3], scan_id[4], scan_id[5]);
        char status = gps_get_status();
        
         
 /*  rather not wait for valid GPS before reporting sensors       
        if ((status != 'A') && (iSensorsToReport == TEMP_HUMIDITY_ACCELEROMETER_GPS))
          { //we must wait for GPS to initialize
              PRINTF("Waiting for GPS to become ready... ");
              while (status != 'A')
              {
                  wait (5.0);        
                  status = gps_get_status();
                  unsigned char num_satellites = gps_get_sate_in_veiw();
                  PRINTF("%c%d", status, num_satellites);
              }
              PRINTF("\r\n");
          } //we must wait for GPS to initialize
 */
 
        PRINTF("gps_check_online is %d\r\n", gps_check_online());
        unsigned char *data;
        data = gps_get_utc_date_time();       
        PRINTF("gps_get_utc_date_time : %d-%d-%d,%d:%d:%d\r\n", data[0], data[1], data[2], data[3], data[4], data[5]); 
        PRINTF("gps_get_status        : %c ('A' = Valid, 'V' = Invalid)\r\n", gps_get_status());
        PRINTF("gps_get_latitude      : %c:%f\r\n", gps_get_ns(), gps_get_latitude());
        PRINTF("gps_get_longitude     : %c:%f\r\n", gps_get_ew(), gps_get_longitude());
        PRINTF("gps_get_altitude      : %f meters\r\n", gps_get_altitude());
        PRINTF("gps_get_speed         : %f knots\r\n", gps_get_speed());
        PRINTF("gps_get_course        : %f degrees\r\n", gps_get_course());
        PRINTF("gps_get_position_fix  : %c\r\n", gps_get_position_fix());
        PRINTF("gps_get_sate_in_view  : %d satellites\r\n", gps_get_sate_in_veiw());
        PRINTF("gps_get_sate_used     : %d\r\n", gps_get_sate_used());
        PRINTF("gps_get_mode          : %c ('A' = Automatic, 'M' = Manual)\r\n", gps_get_mode());
        PRINTF("gps_get_mode2         : %c ('1' = no fix, '1' = 2D fix, '3' = 3D fix)\r\n", gps_get_mode2()); 
    } //bool bGPS_present = true
} //Init_GPS()

void Read_GPS()
{
    unsigned char gps_satellites = 0; //default
    int lat_sign;
    int long_sign;
    if (bGPS_present)
    {
        if ((gps_get_status() == 'A') && (gps_get_mode2() != '1'))
        {
            gps_satellites = gps_get_sate_in_veiw(); //show the number of satellites
        }
        if (gps_get_ns() == 'S')
        {
            lat_sign = -1; //negative number
        }
        else
        {
            lat_sign = 1;
        }    
        if (gps_get_ew() == 'W')
        {
            long_sign = -1; //negative number
        }
        else
        {
            long_sign = 1;
        }    
#if (1)
        PRINTF("gps_satellites        : %d\r\n", gps_satellites);
        PRINTF("gps_get_latitude      : %f\r\n", (lat_sign * gps_get_latitude()));
        PRINTF("gps_get_longitude     : %f\r\n", (long_sign * gps_get_longitude()));
        PRINTF("gps_get_altitude      : %f meters\r\n", gps_get_altitude());
        PRINTF("gps_get_speed         : %f knots\r\n", gps_get_speed());
        PRINTF("gps_get_course        : %f degrees\r\n", gps_get_course());
#endif
        sprintf(SENSOR_DATA.GPS_Satellites, "%d", gps_satellites);
        sprintf(SENSOR_DATA.GPS_Latitude, "%f", (lat_sign * gps_get_latitude()));
        sprintf(SENSOR_DATA.GPS_Longitude, "%f", (long_sign * gps_get_longitude()));
        sprintf(SENSOR_DATA.GPS_Altitude, "%f", gps_get_altitude());
        sprintf(SENSOR_DATA.GPS_Speed, "%f", gps_get_speed());
        sprintf(SENSOR_DATA.GPS_Course, "%f", gps_get_course());
    } //bGPS_present
} //Read_GPS()

 
void sensors_init(void)
{
    Init_HTS221();
    Init_Si7020();
    Init_motion_sensor();
    Init_GPS();
} //sensors_init

void read_sensors(void)
{
    Read_Battery_Volts();
    Read_HTS221();
    Read_Si7020();
    Read_motion_sensor();
    Read_GPS();
} //read_sensors
