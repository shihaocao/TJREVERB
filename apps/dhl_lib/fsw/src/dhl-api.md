# Dellingr Hardware Library (DHL) 

## Introduction 

The purpose of the DHL is to abstract the hardware devices on the Dellingr Cubesat Satellite in order to:

  - Make it easier to develop the flight software applications 
  - Provide a single point for function documentation
  - To provide a transparent method for simulating the hardware devices 

The DHL is implemented as a cFS loadable library. This library could be completely simulated on another platform to run the cFS and Dellingr flight software applications unmodified 

## DHL sections:

The DHL is comprised of the following device subsections

### DHL Initialization


### Camera Library


#### Purpose 

The DHL Camera Library provides the interface to the serial camera.

#### Defines


#### Data


#### Functions 

```
int CAM_RxReady(void);
int CAM_Init_SC16IS760 (void);
int32 CAM_LibInit(void);
int CAM_ResetCamera(void);
int CAM_SnapImage(void);
int CAM_GetCapturedImage(void);
int CAM_EndCaptureImage(void);
int CAM_GetImage(char *FileName);
```

### DAGR Magnetometer Library

#### Purpose 

The DHL Magnetometer Library provides the interface to the DAGR Magnetometer instrument.

#### Defines

```
#define DAGR_PWR_CMD_1  1
#define DAGR_PWR_CMD_2  2
#define DAGR_PWR_CMD_3  3
#define DAGR_PWR_CMD_4  4
#define DAGR_PWR_CMD_5  5
```

#### Data


#### Functions 

```
int32 DAGR_LibInit(void);
int32 DAGR_InitMagDevice(void);
uint8 DAGR_WriteRead(uint8 value);
int32 DAGR_ReadDagrPacket(uint8 CommandCode, uint8 *DagrData);
```

### Deployables Library

#### Purpose 

The DHL Deployables library provides .. TBD

#### Defines

```
#define DEP_SWITCH_ON_TIME 10  // deployment switches left on for 10 seconds
#define DEP_DATA_RATE      10  // record current data at 10 Hz
```

#### Data

#### Functions 

```
void DeployAll(void);
```

### EPS Library


#### Purpose 

The DHL EPS Library proides an interface to the EPS and Battery Subsystems. 

#### Defines

```
#define EPS_ADDRESS       0x2B
```

#### Data

```
extern const uint8 bat_address[];
extern const uint8 bcr_address[];
```

#### Functions 

```
int32   EPS_LibInit(void);

int32   EPS_init_hardware(void);

void    EPS_read_battery_pack_hk(uint8 address, float voltage_array[], float current_array[], float temp_array[]);

uint16  EPS_read_battery_pack_version(uint8 address); 

uint16  EPS_read_battery_pack_status(uint8 address); 

uint16  EPS_read_eps_version(void);

uint16  EPS_read_eps_status(void);

void    EPS_read_analog_eps(uint8 cmd_data, float * data_output);

boolean EPS_read_eps_sun_detector_a(uint8 bcr_num);

boolean EPS_read_eps_sun_detector_b(uint8 bcr_num);

void EPS_set_switch(uint8 switch_num, uint8 state);
```

### GPS Library

#### Purpose 

The DHL GPS Library provides an interface to the GPS subsystem. 

#### Defines

```
#define CMD_LOG				         1
#define CMD_FRESET		            20
#define CMD_SERIALCONFIG	         1246
#define LOG_BESTPOS 		         42
#define LOG_TRACKSTAT 		         83
#define LOG_RXSTATUS 		         93
#define LOG_TIME		               101
#define LOG_BESTXYZ 		        241
#define LOG_RAWALM                 74
#define LOG_SETAPPROXTIME          102
#define LOG_RAWEPHEMA              41
#define LOG_HW	 	               963
#define LOG_BESTVEL                99
#define LOG_PDPXYZ                 471
#define LOG_PDPFILTER              424
#define LOG_PDPVEL                 470
#define LOG_APPROXTIME             102  // to set initial default gps time

#define TRIGGER_ONNEW		        0
#define TRIGGER_ONCHANGED	        1
#define TRIGGER_ONTIME		        2
#define TRIGGER_ONNEXT		        3

#define TRIGGER_ONCE		        4
#define TRIGGER_ONMARK		        5

#define MSG_TYPE_BIN		        0
#define MSG_TYPE_ASCII  	        1
#define MSG_TYPE_ABB_ASCII 	     2
 
/* response id's */
#define OK			              1
#define REQ_LOG_NOT_VALID	       2
#define INVALID_MSG_ID		       6
#define INVALID_CHECKSUM	       8

#define NOVATEL_CSP_MAX_LEN	   190
```

#### Data

#### Functions 

```
int32 GPS_LibInit(void);

int32 GPS_LibHwInit(void);

int GPS_GetTime(unsigned int *week, unsigned int *sec);

int GPS_GetXyz(double *px, double *py, double *pz, double *vx, double *vy, double *vz);

int GPS_GetPosition(double *lat, double *lgt, double* hgt);

int GPS_PpsOn(void);

int GPS_PpsOff(void);

int GPS_Unlog(void);

int GPS_ComStatus(void);

int GPS_SetBaudRate_115200(void);

int GPS_SetBaudRate_9600(void);

int GPS_AsciiCommand(char * cmd, uint16 cmd_len, char * response, uint16 max_res_len, uint16 timeout_ms);
```

### INMS Library


#### Purpose 

The DHL INMS Library provides an interface to the INMS Instrument on Dellingr.

#### Defines


```
#define F_CPU     40000000
#define F_OSC     8000000
#define F_USART   500000
#define SPCK      1000000

/* Maximum Bytes Per Second (BPS) rate that will be forced using the CTS pin. */
#define MAX_BPS             500
/* Size of the receive buffer used by the PDC, in bytes. */
#define BUFFER_SIZE         1

/* response id's */
#define ACK		       0x06
#define NAK             0x15
#define TIMEOUT_ERROR   0x19

#define INMS_CSP_MAX_LEN        8    // command data
```

#### Data

```
typedef struct inms_csp_backdoor_s 
{
   uint8 opcode;				// opcode
   uint8 checksum;			// checksum
   uint8 data[INMS_CSP_MAX_LEN];	// Used for holding command for INMS
}   OS_PACK inms_csp_backdoor_t;

/* INMS message header */
typedef struct inms_msg_header_s 
{
   uint8 sync;
   uint8 t0; // utc sec
   uint8 t1; // utc sec
   uint8 t2; // utc sec
   uint8 t3; // utc sec
   uint8 s0; // utc sub sec
   uint8 s1; // utc sub sec
} OS_PACK inms_msg_header_t;

/* INMS response header */
typedef struct inms_response_header_s 
{
	inms_msg_header_t msg;
} OS_PACK inms_response_header_t;

/* INMS Housekeeping log response */
typedef struct inms_housekeeping_s 
{
   uint8 opcode;
   uint8 counter;
   uint32 tof;
   uint8 max_tof;
   uint8 start_pulse;
   uint8 delay_start;
   uint16 dac;
   uint32 v1v2;
   uint32 v3v4;
   uint32 v5v6;
   uint32 v7v8;  //printf("EXIT INMS_SPI_SETUP_CS\r\n");
   uint32 temp;
   uint16 nop;
   uint32 blank;
} OS_PACK inms_housekeeping_t;

/* inms detector1 log response */
typedef struct inms_detector1_s 
{
   uint16 totalCount;
   uint8  totalCount1;
   uint16 specie1Cnt;
   uint16 s1s2;
   uint16 specie2Cnt;
   uint16 specie3Cnt;
   uint16 s3s4;
   uint16 specie4Cnt;
} OS_PACK inms_detector1_t;

/* inms detector2 log response */
typedef struct inms_detector2_s 
{
   uint16 totalCount;
   uint8  totalCount1;
   uint16 specie1Cnt;
   uint16 s1s2;
   uint16 specie2Cnt;
   uint16 specie3Cnt;
   uint16 s3s4;
   uint16 specie4Cnt;
} OS_PACK inms_detector2_t;

typedef struct inms_telemetry_s 
{
   inms_housekeeping_t HK;
   inms_detector1_t DT1;
   inms_detector2_t DT2;
   uint16 CHKSUM;

} OS_PACK inms_telemetry_t;

/* INMS request command */
typedef struct inms_request_s 
{
	uint8 opcode;
	uint8 param1;
	uint8 param2;
	uint8 param3;
	uint8 param4;
	uint8 param5;
	uint8 param6;
	uint8 chksum;
} OS_PACK inms_request_t;
```

#### Functions 

```
int32 INMS_LibInit(void);
void  INMS_LibInitDevice(void);
int32 INMS_LibTransferINMSData(char *CommandBuffer, char *TelemetryBuffer, uint32 TelemetryLength);
```

### L3 Cadet Radio (L3C) Library

#### Purpose 

The DHL L3 Cadet Radio Library provides an interface to the L3 Cadet Radio.

#### Defines

#### Data

```
typedef void (*L3C_callback_t) (uint8 * buf, int len, void * pxTaskWoken);
```

#### Functions 

```
int32   L3C_LibInit(void);

int32   L3C_SendMessageToRadio(uint8 *MsgHdr,  uint32 HdrSize,
                               uint8 *Payload, uint32 PayloadSize);

int32   L3C_InstallCallback (L3C_callback_t callback);

int32   L3C_OpenSerialPort(void);

uint32  L3C_crc32(uint8 *block, unsigned int length);
```

### Nanomind Library

#### Purpose 

The DHL Nanomind Library provides an interface to the miscellaneous sensors on the Nanomind board such as the magnetometer module.

#### Defines

#### Data

```
```
#### Functions 

```
int32 NM_LibInit(void);

int   NM_LibReadMagnetometer(float *X, float *Y, float *Z);
```

### Reaction Wheel Library

#### Purpose 

The DHL Reaction Wheel Library provides an interface to the reaction wheel assembly. 

#### Defines

```
#define RW_MODE_IDLE     0x00
#define RW_MODE_CURRENT  0x02
#define RW_MODE_SPEED    0x05
#define RW_MODE_TORQUE   0x16
#define RW_MODE_BURNIN   0x17
#define RW_MODE_SFFT     0x18

#define RW_PARAM_TEMP        0x03
#define RW_PARAM_SPEED       0x05
#define RW_PARAM_TORQUE      0x1C
#define RW_PARAM_SEU_COUNT   0x29
#define RW_PARAM_SFFT_START  0x80
#define RW_PARAM_SFFT_END    0xE3
```

#### Data

#### Functions 

```
int32 RW_LibInit(void);

int32 RW_LibCommandRW(float x, float y, float z);

int rwlib_turnon(uint8 dest_addr);

int rwlib_turnoff(uint8 dest_addr);

int rwlib_ping(uint8 dest_addr);

int rwlib_receiveModeTelem(uint8 dest_addr, uint8 *mode_type, float *mode_value);

int rwlib_receiveParamTelem(uint8 dest_addr, uint8 *param_number, float *param_value);

int rwlib_sendModeCommand(uint8 dest_addr, uint8 mode_number, float mode_value);

int rwlib_sendParamCommand(uint8 dest_addr, uint8 param_number, float param_value);
```

### Solar Library

#### Purpose 

The DHL Solar Library provides an interface to the solar sensors on the Dellingr Satellite including the Fine and Course Sun Sensors. 

#### Defines

```
#define FSS_DEVICE_0      0
#define FSS_DEVICE_1      1
#define FSS_DEVICE_2      2

#define FSS_DEVICE_0_SPI  8
#define FSS_DEVICE_1_SPI  9
#define FSS_DEVICE_2_SPI  12 

#define FSS_NUM_DEVICES   3
```
#### Data

#### Functions 

```
int32 SOL_LibInit(void);

int32 SOL_LibCommandMT(float x, float y, float z);

int SOL_FSSSetup(int FssDevice);

int SOL_FSSRead(int FssDevice, float *AzimuthAngle, float *ElevationAngle);
```

### Special Services Card (SSC) Library

#### Purpose 

The DHL Special Services Card Libary provides an interface to the functions on the special services card including the MPU 6000 devices. 

#### Defines

```
#define LTC_DEVICE_0     0
#define LTC_DEVICE_1     1
#define LTC_DEVICE_2     2
#define LTC_DEVICE_3     3

#define SSC_LTC_I2C_ADDR0 0x18    
#define SSC_LTC_I2C_ADDR1 0x19    
#define SSC_LTC_I2C_ADDR2 0x1A    
#define SSC_LTC_I2C_ADDR3 0x1B    

/* Single-Ended Channel Configuration */
#define LTC_CH0                0x80
#define LTC_CH1                0xC0
#define LTC_CH2                0x90
#define LTC_CH3                0xD0
#define LTC_CH4                0xA0
#define LTC_CH5                0xE0
#define LTC_CH6                0xB0
#define LTC_CH7                0xF0

/* 
** Modified for Nanomind 1/25/15 (E.L)
** LTC1867 Configuration Bits
*/
#define LTC_SLEEP_MODE         0x04
#define LTC_EXIT_SLEEP_MODE    0x00
#define LTC_UNIPOLAR_MODE      0x08
#define LTC_BIPOLAR_MODE       0x00
#define LTC_SINGLE_ENDED_MODE  0x80
#define LTC_DIFFERENTIAL_MODE  0x00

/*
** MPU 6050 defines
*/
#define MPU_GSCALE 9.80665/16384
#define MPU_DEGDIV 16.4

/*MPU 6050 reg addr:      hex     dec    description    */
#define MPU_ACCEL_XH 	 0x3B    //59   ACCEL_X[15:8]
#define MPU_ACCEL_XL 	 0x3C    //60   ACCEL_X[7:0]
#define MPU_ACCEL_YH 	 0x3D    //61   ACCEL_Y[15:8]
#define MPU_ACCEL_YL 	 0x3E    //62   ACCEL_Y[7:0]
#define MPU_ACCEL_ZH 	 0x3F    //63   ACCEL_Z[15:8]
#define MPU_ACCEL_ZL 	 0x40    //64   ACCEL_Z[7:0]
#define MPU_TEMP_H   	 0x41    //65   TEMP[15:8]
#define MPU_TEMP_L   	 0x42    //66   TEMP[7:0]
#define MPU_GYRO_XH  	 0x43    //67   GYRO_X[15:8]
#define MPU_GYRO_XL  	 0x44    //68   GYRO_X[7:0]
#define MPU_GYRO_YH  	 0x45    //69   GYRO_Y[15:8]
#define MPU_GYRO_YL  	 0x46    //70   GYRO_Y[7:0]
#define MPU_GYRO_ZH  	 0x47    //71   GYRO_Z[15:8]
#define MPU_GYRO_ZL  	 0x48    //72   GYRO_Z[7:0]

#define MPU_DEVICE_0     0
#define MPU_DEVICE_1     1
#define MPU_DEVICE_2     2
#define MPU_DEVICE_3     3
```

#### Data

```
typedef union
{
   uint16 LT_uint16;
   uint8  LT_byte[2];
} LT_union_int16_2bytes;
```

#### Functions 

```
int32 SSC_LibInit(void);

int SSC_LTC_init(void);

int SSC_LTC2309_init(uint8 addr, uint16 speed);


/*
** Reads 12-bit code from LTC2309, programs channel and mode for next conversion.
** Returns the state of the acknowledge bit after the I2C address write. 
*/
int8 SSC_LTC2309_read(uint8   i2c_address,  // I2C address of device
                      uint8   adc_command,  // ADC command / address bits
                      uint16 *adc_data);    // Returns code read from ADC

/*
**
*/
float SSC_LTC2309_code_to_voltage(uint16 adc_code, float vref, uint8 uni_bipolar);

/*
** MPU6050 functions
*/
int32 SSC_InitMPU6050(int32 MpuDeviceNum);

float SSC_mpu6050_read_temp(uint32 MpuDeviceNum);

float SSC_mpu6050_read_rate(uint32 MpuDeviceNum, uint8 addrH) ;

float SSC_mpu6050_read_angle(uint32 MpuDeviceNum, uint8 addrH) ;
```

### Thermal Louvre Control (TLC) Library

TBD:

```
int32 TLC_LibInit(void);
```

