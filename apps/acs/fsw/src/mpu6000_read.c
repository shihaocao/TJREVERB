#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ssc_lib.h"
#include "acs_msg.h"


//#define _MPU_DEBUG_

extern acs_mpu_tlm_t ACS_MpuTlmPkt;

int mpu6000_read(void)
{
   int index;
   uint16 val;
   
	  //for (index=0; index<=3; index++)
     {
        index=0;
        val = SSC_MPU6000_ReadTemp(index);
        ACS_MpuTlmPkt.mpu1_temp = val;
        #ifdef _MPU_DEBUG_
        printf("Temp %d: %f degC\n", index, val/340.0 + 36.53); 
        printf("Rate: "); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_XH);
        ACS_MpuTlmPkt.mpu1_rate[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_YH);
        ACS_MpuTlmPkt.mpu1_rate[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_ZH);
        ACS_MpuTlmPkt.mpu1_rate[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f g\n", (float)val/MPU_DEGDIV); 
        printf("Acc: "); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_XH);
        ACS_MpuTlmPkt.mpu1_acc[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_YH);
        ACS_MpuTlmPkt.mpu1_acc[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_ZH);
        ACS_MpuTlmPkt.mpu1_acc[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f deg/s\n", (float)val*MPU_GSCALE); 
        #endif

        index++;
        val = SSC_MPU6000_ReadTemp(index);
        ACS_MpuTlmPkt.mpu2_temp = val;
        #ifdef _MPU_DEBUG_
        printf("Temp %d: %f degC\n", index, (float)val/340.0+ 36.53); 
        printf("Rate: "); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_XH);
        ACS_MpuTlmPkt.mpu2_rate[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_YH);
        ACS_MpuTlmPkt.mpu2_rate[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_ZH);
        ACS_MpuTlmPkt.mpu2_rate[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f g\n", (float)val/MPU_DEGDIV); 
        printf("Acc: "); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_XH);
        ACS_MpuTlmPkt.mpu2_acc[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_YH);
        ACS_MpuTlmPkt.mpu2_acc[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_ZH);
        ACS_MpuTlmPkt.mpu2_acc[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f deg/s\n", (float)val*MPU_GSCALE); 
        #endif

        index++;
        val = SSC_MPU6000_ReadTemp(index);
        ACS_MpuTlmPkt.mpu3_temp = val;
        #ifdef _MPU_DEBUG_
        printf("Temp %d: %f degC\n", index, (float)val/340.0+ 36.53); 
        printf("Rate: "); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_XH);
        ACS_MpuTlmPkt.mpu3_rate[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_YH);
        ACS_MpuTlmPkt.mpu3_rate[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_ZH);
        ACS_MpuTlmPkt.mpu3_rate[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f g\n", (float)val/MPU_DEGDIV); 
        printf("Acc: "); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_XH);
        ACS_MpuTlmPkt.mpu3_acc[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_YH);
        ACS_MpuTlmPkt.mpu3_acc[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_ZH);
        ACS_MpuTlmPkt.mpu3_acc[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f deg/s\n", (float)val*MPU_GSCALE); 
        #endif
     
        index++;
        val = SSC_MPU6000_ReadTemp(index);
        ACS_MpuTlmPkt.mpu4_temp = val;
        #ifdef _MPU_DEBUG_
        printf("Temp %d: %f degC\n", index, (float)val/340.0+ 36.53); 
        printf("Rate: "); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_XH);
        ACS_MpuTlmPkt.mpu4_rate[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_YH);
        ACS_MpuTlmPkt.mpu4_rate[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val/MPU_DEGDIV); 
        #endif
        val = SSC_MPU6000_ReadRate(index, MPU_GYRO_ZH);
        ACS_MpuTlmPkt.mpu4_rate[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f g\n", (float)val/MPU_DEGDIV); 
        printf("Acc: "); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_XH);
        ACS_MpuTlmPkt.mpu4_acc[0] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_YH);
        ACS_MpuTlmPkt.mpu4_acc[1] = val;
        #ifdef _MPU_DEBUG_
        printf("%f ", (float)val*MPU_GSCALE); 
        #endif
        val = SSC_MPU6000_ReadAcc(index, MPU_ACCEL_ZH);
        ACS_MpuTlmPkt.mpu4_acc[2] = val;
        #ifdef _MPU_DEBUG_
        printf("%f deg/s\n", (float)val*MPU_GSCALE); 
        #endif
	  }

return 0;

}
