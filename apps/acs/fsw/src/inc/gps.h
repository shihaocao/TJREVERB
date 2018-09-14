#ifndef __GPS_
#define __GPS_

#include <stdint.h>

/* response id's */
#define OK			1
#define INVALID_CHECKSUM	8



/* Example obtained:
#BESTPOSA,COM2,0,82.5,FINESTEERING,1836,317693.000,00000000,6145,6588;SOL_COMPUTED,SINGLE,38.99546795891,-76.85675261198,73.0259,-33.5000,WGS84,8.0836,29
#BESTVELA,COM2,0,82.5,FINESTEERING,1836,317693.000,00000000,827b,6588;SOL_COMPUTED,DOPPLER_VELOCITY,0.150,0.000,0.1395,183.354399,-0.0699,0.0*9e374c71
#PDPXYZA,COM2,0,82.5,FINESTEERING,1836,3
*/


int getGPSTime(unsigned int *week, unsigned int *sec);
int getxyz(double *px,double *py,double*pz,double*vx,double*vy,double*vz);
int getPosition(double *lat,double *lgt, double* hgt);

/**
 * ascii_command is used for ascii communication with the gps.
 * 
 * @param cmd pointer to ascii command for GPS
 * @param cmd_len length of command pointed to by cmd
 * @param response pointer to buffer to store the command response from the GPS
 * @param max_res_len maximum length of GPS response
 * @param timeout_ms time in ms to wait for the response from the GPS
 * @return length of response from GPS
 */
int ascii_command(char * cmd, uint16_t cmd_len, char * response, uint16_t max_res_len, uint16_t timeout_ms);

#endif /* __GPS_ */
