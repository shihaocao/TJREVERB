/*
** dhl_hw_defs.h
**
** Hardware definitions for Dellingr HW interfaces.
*/
#ifndef _dhl_hw_defs_h_
#define _dhl_hw_defs_h_


/*
** Nanomind / ARM CPU devices
**
**   Nanomind Analog Pins
**
**     TEMP  1 - Internal Nanomind Temperature 1
**     TEMP  2 - Internal Nanomind Temperature 2
**     CSS   1 - A1 is CSS X+
**     CSS   2 - A2 is CSS Z+
**     CSS   3 - A3 is CSS Y+
**     CSS   4 - A4 is CSS X-
**     CSS   5 - A5 is CSS Z-
**     CSS   6 - A6 is CSS Y-
*/

#define DHL_NM_TEMP_1 0
#define DHL_NM_TEMP_2 1
#define DHL_NM_CSS_1  2
#define DHL_NM_CSS_2  3
#define DHL_NM_CSS_3  4
#define DHL_NM_CSS_4  5
#define DHL_NM_CSS_5  6
#define DHL_NM_CSS_6  7

/*
**
**     Tourquer interfaces - Using the Nanomind Analog / PWM 
**     Channel 1 goes to the Z Panels
**     Channel 2 goes to the Y Panels
**     Channel 3 goes to the X Panels
**     ( see nanomind/sol_lib MT command function ) 
*/

#define DHL_NM_PWM_CH1 1
#define DHL_NM_PWM_CH2 2
#define DHL_NM_PWM_CH3 3

/*
**   Nanomind GPIO Pins
**      GPIO1 - Cadet CTS
**      GPIO2 - Cadet RTS
**      GPIO3 - Mag Deploy USW1
**      GPIO4 - Antenna Deploy?
**      GPIO5 - Deploy Power Monitor
**      GPIO6 -
**      GPIO7 - 1PPS
*/
#define DHL_NM_GPIO_1_CADET_CTS   1
#define DHL_NM_GPIO_2_CADET_RTS   2
#define DHL_NM_GPIO_3_MAG_DEPLOY  3
#define DHL_NM_GPIO_4_ANT_DEPLOY  4
#define DHL_NM_GPIO_5_DEP_PWR_MON 5
#define DHL_NM_GPIO_6             6
#define DHL_NM_GPIO_1PPS          7

/*
**   Nanomind UARTS
**
**      USART 0 - Console
**      USART 1 - GPS
**      USART 2 - Cadet Radio
*/
#define DHL_NM_CONSOLE_USART 0
#define DHL_NM_GPS_USART     1
#define DHL_NM_CADET_USART   2


/*
** Special Services Card (SSC) Devices
** 
** The SSC has 2 SC16IS760 USART and GPIO extender ICs
** It also has 4 LTC2309 12-Bit ADC devices for Thermistors
**
**    SSC SC16IS760 USARTS  
** 
**      SCI Device 1  - Camera 
**      SCI Device 2  - STIM_210 IMU
**  Not sure how to enumerate these devices..
*/

/*
**
**    SSC SC16IS760 GPIO Pins
**
**      SCI Device 1
**         GPIO 0 - nc?
**         GPIO 1 - nc? 
**         GPIO 2 - nc
**         GPIO 3 - nc
**         GPIO 4 - Pri Buck Enable
**         GPIO 5 - Pri FET enable
**         GPIO 6 - TLC Enable 1
**         GPIO 7 - Camera enable 1
*/
#define DHL_SSC_GPIO_0_0  0
#define DHL_SSC_GPIO_0_1  1
#define DHL_SSC_GPIO_0_2  2
#define DHL_SSC_GPIO_0_3  3
#define DHL_SSC_GPIO_0_4_PRI_BUCK_ENABLE 4
#define DHL_SSC_GPIO_0_5_PRI_FET_ENABLE  5
#define DHL_SSC_GPIO_0_6_TLC_ENABLE_1    6
#define DHL_SSC_GPIO_0_7_CAM_ENABLE_1    7

/*
**
**      SCI Device 2
**
**         GPIO 0 - Modem Reset cadet
**         GPIO 1 - Reset host Cadet
**         GPIO 2 - Mag reset
**         GPIO 3 - nc
**         GPIO 4 - Backup Buck Enable
**         GPIO 5 - Backup FET enable
**         GPIO 6 - Thermal Louvre enable 2
**         GPIO 7 - Camera Enable 2
**
*/
#define DHL_SSC_GPIO_1_0_CADET_MODEM_RESET  0
#define DHL_SSC_GPIO_1_1_CADET_HOST_RESET   1
#define DHL_SSC_GPIO_1_2_MAG_RESET          2
#define DHL_SSC_GPIO_1_3  3
#define DHL_SSC_GPIO_1_4_BACKUP_BUCK_ENABLE 4
#define DHL_SSC_GPIO_1_5_BACKUP_FET_ENABLE  5
#define DHL_SSC_GPIO_1_6_TLC_ENABLE_2       6
#define DHL_SSC_GPIO_1_7_CAM_ENABLE_2       7


/*
**    SSC A2D Pins
**
**       2309 A2D Device 1
**
**          1_0 -> Thermistor P401 Baseplate 1
**          1_1 -> Thermistor P402 Baseplate 2
**          1_2 -> Thermistor P403 Solar Panel -Z
**          1_3 -> Thermistor P404 Solar Panel +X
**          1_4 -> Thermistor P405 Solar Panel -X
**          1_5 -> Thermistor P406 Solar Panel +Y
**          1_6 -> Thermistor P407 Solar Panel -Y
**          1_7 -> Thermistor P408 RW Housing Top
*/
#define DHL_SSC_ADC_1_0_P401   0
#define DHL_SSC_ADC_1_1_P402   1
#define DHL_SSC_ADC_1_2_P403   2
#define DHL_SSC_ADC_1_3_P404   3
#define DHL_SSC_ADC_1_4_P405   4
#define DHL_SSC_ADC_1_5_P406   5
#define DHL_SSC_ADC_1_6_P407   6
#define DHL_SSC_ADC_1_7_P408   7

/*
**
**      2309 A2D Device 2
** 
**          2_0 -> Thermistor P409 RW Housing Wall - L3 Radio Mount
**          2_1 -> Thermistor P410 L3 Radio
**          2_2 -> Thermistor P411 Billingsley - by RW ( board ) 
**          2_3 -> Thermistor P412 Billingsley - by RW ( bracket )
**          2_4 -> Thermistor P413 INMS Structure - Backplane
**          2_5 -> Thermistor P414 INMS Structure - Side Wall
**          2_6 -> Thermistor P415 INMS Structure - Cubesat I/F
**          2_7 -> Thermistor P416 Mems Gyro
*/
#define DHL_SSC_ADC_2_0_P409   0
#define DHL_SSC_ADC_2_1_P410   1
#define DHL_SSC_ADC_2_2_P411   2
#define DHL_SSC_ADC_2_3_P412   3
#define DHL_SSC_ADC_2_4_P413   4
#define DHL_SSC_ADC_2_5_P414   5
#define DHL_SSC_ADC_2_6_P415   6
#define DHL_SSC_ADC_2_7_P416   7


/*
**   
**      2309 A2D Device 3
**
**          1_0 -> Thermistor P417 By INMS  AL Bracket
**          1_1 -> Thermistor P418 E-Card Housing
**          1_2 -> Thermistor P419 E-Card Housing 
**          1_3 -> Thermistor P420 Mag Card
**          1_4 -> Thermistor P421 GPS Card
**          1_5 -> Thermistor P422 SSC Card
**          1_6 -> Thermistor P423 Battery Top
**          1_7 -> Thermistor P424 Battery Bottom
*/
#define DHL_SSC_ADC_3_0_P417   0
#define DHL_SSC_ADC_3_1_P418   1
#define DHL_SSC_ADC_3_2_P419   2
#define DHL_SSC_ADC_3_3_P420   3
#define DHL_SSC_ADC_3_4_P421   4
#define DHL_SSC_ADC_3_5_P422   5
#define DHL_SSC_ADC_3_6_P423   6
#define DHL_SSC_ADC_3_7_P424   7


/*
**
**      2309 A2D Device 4
**
**          1_0 -> TLC_SENSOR_VOUT
**          1_1 -> Thermistor P425
**          1_2 -> Thermistor P426
**          1_3 -> nc
**          1_4 -> nc
**          1_5 -> nc
**          1_6 -> INMS VMON
**          1_7 -> INMS IMON
*/
#define DHL_SSC_ADC_4_0_TLC_SEN_VOUT   0
#define DHL_SSC_ADC_4_1_P425           1
#define DHL_SSC_ADC_4_2_P426           2
#define DHL_SSC_ADC_4_3                3
#define DHL_SSC_ADC_4_4                4
#define DHL_SSC_ADC_4_5                5
#define DHL_SSC_ADC_4_6_INMS_VMON      6
#define DHL_SSC_ADC_4_7_INMS_IMON      7


/*
**
**  I2C Devices
**   
**    Reaction Wheels - Is this just one I2C device? 
**    Gomspace/Nanomind Fine Sun Sensors (2) 
**    Clydespace EPS/Battery Subsystem
**    2309 A2D devices (4) for analog to digital inputs
**    SC16IS760 USART and GPIO Exenders (2)
**
*/

/*    
**  SPI Devices
**
**    SPI CS 0 - SD Card
**    SPI CS 1 - INMS
**    SPI CS 2 - MAG/DAGR
**    SPI CS 3 - IMU 1 - MPU6000
**    SPI CS 4 - IMU 2 - MPU6000
**    SPI CS 5 - IMU 3 - MPU6000
**    SPI CS 6 - IMU 4 - MPU6000
**    SPI CS 12 - GSFC FSS
** 
*/

#define DHL_NM_SPI_CS_SD_CARD 0
#define DHL_NM_SPI_CS_INMS    1
#define DHL_NM_SPI_CS_DAGR    2
#define DHL_NM_SPI_CS_IMU1    3
#define DHL_NM_SPI_CS_IMU2    4
#define DHL_NM_SPI_CS_IMU3    5
#define DHL_NM_SPI_CS_IMU4    6
#define DHL_NM_SPI_CS_7       7
#define DHL_NM_SPI_CS_WFSS    12

#endif /* _dhl_hw_defs_h_ */
