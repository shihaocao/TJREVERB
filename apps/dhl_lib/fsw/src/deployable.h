
/*
 * Deployable.h
 *
 *  Created on: Mar 26, 2015
 *      Author: James Marshall
 */

#ifndef DEPLOYABLE_H_
#define DEPLOYABLE_H_

#include <inttypes.h>

#define DEP_SWITCH_ON_TIME 10  // deployment switches left on for 10 seconds
#define DEP_DATA_RATE      10  // record current data at 10 Hz

void DeployOn (uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state);
void DeployOff(uint8_t gpio_ext_addr, uint8_t *ant_state, uint8_t *mag_state);

#endif /* DEPLOYABLE_H_ */
