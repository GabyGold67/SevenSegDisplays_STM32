/**
 * @file		: SevenSegDispHw.h
 * @brief	: Header file for SevenSegDispHw library classes
 *
 * @author	: Gabriel D. Goldman
 * @date		: Created on: Nov 16, 2023
 */

#ifndef _SEVENSEGDISPHW_H_
#define _SEVENSEGDISPHW_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
//===========================>> Next lines included for developing purposes, corresponding headers must be provided for the production platform/s
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
//===========================>> Previous lines included for developing purposes, corresponding headers must be provided for the production platform/s
#define MCU_SPEC 1

#ifdef MCU_SPEC
	#ifndef __STM32F4xx_HAL_H
		#include "stm32f4xx_hal.h"
	#endif

	#ifndef __STM32F4xx_HAL_GPIO_H
		#include "stm32f4xx_hal_gpio.h"
	#endif
#endif

//===========================>> Next lines used to avoid CMSIS wrappers
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//#include "queue.h"
//#include "semphr.h"
//#include "event_groups.h"
//===========================>> Previous lines used to avoid CMSIS wrappers


//===========================>> BEGIN User type definitions
struct gpioPinId_t{	// Type used to keep GPIO pin identification as a single parameter, as platform independent as possible
	GPIO_TypeDef* portId;
	uint16_t pinNum;
};
//===========================>> END User type definitions

//===========================>> BEGIN General use function prototypes
uint8_t singleBitPosNum(uint16_t mask);
//===========================>> END General use function prototypes

//============================================================> Class declarations separator

class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    gpioPinId_t* _ioPins{};
    uint8_t* _digitPosPtr{nullptr};
    uint8_t _dspDigitsQty{}; //Display size in digits
    bool _commAnode {true}; //SevenSegDisplays objects will retrieve this info to build the right segments for each character
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspHwInstNbr{0};

    // virtual void send(uint8_t* digitsBuffer);  //===================>> To be implemented
    // virtual void send(const uint8_t &segments, const uint8_t &port);  //===================>> To be implemented
public:
    SevenSegDispHw();
    SevenSegDispHw(gpioPinId_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    ~SevenSegDispHw();
    bool getCommAnode();
    uint8_t* getDspBuffPtr();
    uint8_t getDspDigits();
    bool setDigitsOrder(uint8_t* newOrderPtr);
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
};





#endif /* _SEVENSEGDISPHW_H_ */
