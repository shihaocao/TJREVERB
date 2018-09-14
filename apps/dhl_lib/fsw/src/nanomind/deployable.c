
#include "dhl_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "deployable.h"
#include "mpio.h"
#include "cam_lib.h"

// The Antenna deploy confirmation switch is on IO_PIN_47
// The Magnetometer deploy confirmation switch is on IO_PIN_48

#define ENABLE_MASK 0x20 // 5th pin
#define BUCK_MASK   0x10 // 4th pin

/*
 * Deploy sequence TODO: Not fully implemented
 *
 * 1) Read confirmation switch states (GPIO5 (Ant Rel) and GPIO4 (Mag Rel) and record states as pre-deployment.
 * 2) Activate the enables on specified GPIO Extender. Monitor the EPS Current on 12V bus and the current monitor at a rate of 10 Hz.
 * 3) Read confirmation switch (GPIO5 and GPIO4) and record states as post-deployment.
 *
 * gpio_ext_addr: GPIO_EXT_A for Primary, GPIO_EXT_B for Backup.
 */
void DeployOn(uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state) {
	uint8_t readval = 0;

	io_init(IO_PIN_47, 0); // Set gpio to input for antenna
	io_init(IO_PIN_48, 0); // for magnetometer
	*ant_state = (uint8_t) io_get(IO_PIN_47); // TODO: Make return bool
	*mag_state = (uint8_t) io_get(IO_PIN_48); // TODO: Make return bool

	SSC_SC16_ReadAreg(gpio_ext_addr, IOSTATE_ADDR, &readval);
	readval = readval | ENABLE_MASK;
	readval = readval | BUCK_MASK;
	printf("\n\n\nDeployOn Called. Uncommented VERSION line to DEPLOY. 0x%2X,0x%2X\n\n\n\n", gpio_ext_addr, readval);
	// SSC_SC16_WriteAreg(gpio_ext_addr, IOSTATE_ADDR, readval);
}

void DeployOff(uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state) {
	uint8_t readval = 0;

	SSC_SC16_ReadAreg(gpio_ext_addr, IOSTATE_ADDR, &readval);
	readval = readval & (~ENABLE_MASK);
	readval = readval & (~BUCK_MASK);
	printf("\n\n\nDeployOff Called. Uncommented VERSION  line. 0x%2X,0x%2X\n\n\n\n", gpio_ext_addr, readval);
	// SSC_SC16_WriteAreg(gpio_ext_addr, IOSTATE_ADDR, readval);

	*ant_state = (uint8_t) io_get(IO_PIN_47); // TODO: make return bool
	*mag_state = (uint8_t) io_get(IO_PIN_48);
}
