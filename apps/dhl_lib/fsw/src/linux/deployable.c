#include <stdio.h>
#include <stdlib.h>
#include "deployable.h"
#include "cam_lib.h"  

// The Antenna deploy confirmation switch is on IO_PIN_47
// The Magnetometer deploy confirmation switch is on IO_PIN_48

#define ENABLE_MASK 0x10 // 5th pin
#define BUCK_MASK   0x08 // 4th pin

#define GPIO_EXT_A  0x48
#define GPIO_EXT_B  0x49

void DeployOn (uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state) {
	printf("DeployOn called\n");
}

void DeployOff(uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state) {
	printf("DeployOff called\n");
}
