/**
 * @file		: SevenSegDispHw.h
 * @brief	: Header file for SevenSegDispHw library classes
 *
 * @author	: Gabriel D. Goldman
 * @date		: Created on: Nov 16, 2023
 */

#ifndef _SEVENSEGDISPHW_H_
#define _SEVENSEGDISPHW_H_

#include <string>
//===========================>> Next lines included for developing purposes, corresponding headers must be provided for the production platform/s
//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_gpio.h"
//===========================>> Previous lines included for developing purposes, corresponding headers must be provided for the production platform/s

#ifndef MCU_SPEC
	#define MCU_SPEC 1

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
#ifndef GPIOPINID_T
#define GPIOPINID_T
struct gpioPinId_t{	// Type used to keep GPIO pin identification as a single parameter, as platform independent as possible
	GPIO_TypeDef* portId;	/**< The port identification as a pointer to a GPIO_TypeDef information structure*/
	uint16_t pinNum;	/**< The number of pin represented as a one bit set binary with the set bit position indicating the pin number*/
};
#endif	//GPIOPINID_T
//===========================>> END User type definitions

//===========================>> BEGIN General use function prototypes
uint8_t singleBitPosNum(uint16_t mask);
//===========================>> END General use function prototypes

//============================================================> Class declarations separator

/**
 * @brief Implements a generic Seven Segments LEDs hardware interface to displays the logically generated characters
 *
 * This class implements all methods and attributes common to a large variety of seven segments led displays hardware.
 * Specific hardware specific methods and attributes will be included in corresponding subclasses defintions.
 *
 * @class SevenSegDispHw
 */
class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    gpioPinId_t* _ioPins{};
    uint8_t* _digitPosPtr{nullptr};
    uint8_t _dspDigitsQty{}; //Display size in digits
    bool _commAnode {true}; //SevenSegDisplays objects will retrieve this info to build the right segments for each character

    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspHwInstNbr{0};

	 virtual void send(uint8_t* digitsBuffer){};
	 virtual void send(const uint8_t &segments, const uint8_t &port){};
public:
    /**
     * @brief Default constructor
     *
     * class SevenSegDispHw
     *
     */
    SevenSegDispHw();
    /**
     * @brief Class constructor
     *
     * @param ioPins Pointer to a gpioPinId_t array with the needed MCU pins identification to drive the hardware display.
     * @param dspDigits Display's length in digits quantity (dspDigitsQty attribute).
     * @param commAnode Indicates the display leds wiring scheme, either **common anode** (true) or **common cathode** (fa.lse)
     *
     * @class SevenSegDispHw
     */
    SevenSegDispHw(gpioPinId_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class virtual destructor
     *
     * @classs SevenSegDispHw
     *
     */
    ~SevenSegDispHw();
    /**
     * @brief Gets the display leds wiring scheme setting for the object
     *
     * The display leds wiring scheme indicates how the individual led segment in a port (digit) are interconnected, either **common anode** (true) or **common cathode** (false). This wiring arrange affects the data to be sent to the digit, as one configuration needs a positive voltage level (pin set, or logic true) to get the segment lit, while the other needs ground (0v, pin reset, or logic false) to get the segment lit
     *
     * @retval true: the object is configured to instantiate a common anode display
     * @retval false: the object is configured to instantiate a common cathode display
     */
    bool getCommAnode();
    /**
     * @brief Gets the pointer to the display's data buffer
     *
     * The display's data buffer is the array of unsigned short int (byte) that holds the value to be exhibited in each display's port.
     *
     * @return A pointer to the display's data buffer.
     *
     */
    uint8_t* getDspBuffPtr();
    /**
     * @brief Gets the Display's length in digits quantity (dspDigitsQty attribute).
     *
     * @return The display's available display ports.
     */
    uint8_t getDspDigits();
    /**
     * @brief Sets a display ports sorting order.
     *
     * Different 7 segments displays are differently wired, while some follow the logic of ordering the ports designated number port from left to right, some do it the opposite way and some use different patterns based on hardware implementation decisions. The library implements a mechanism to provide the instantiated object to relate the positions on object's display buffer to the positions on the display hardware through a "translation array". This array has the size of the display instantiated, and each array element is meant to hold the number of the corresponding display port, being the first element of the array (array[0]) the corresponding to the leftmost display digit, array[1], the next to it's right and so on. The array is default defined in the constructor as (0, 1, 2,...) that is the most usual implementation found. If the order needs to be changed the `setDigitsOrder()` method is the way to set a new mapping.
     *
     * @param newOrderPtr Pointer to the "translation array".
     *
     * @retval true: All of the elements of the array were in the accepted range. The change was performed
     * @retval false: At least one of the values of the array passed were out of range. The change wasn't performed.
     *
     * @note Each value in the array passed as argument will be checked against the _dspDigits value to ensure that they are all in the range acceptable, 0 <= value <= _dspDigits - 1. If one of the values is out of the valid range no change will be done. Please note that no checking will be done to ensure all of the array values are different. A repeated value will be accepted. leading to unexpected display due to superimposing digits and not included digits.
     *
     */
    bool setDigitsOrder(uint8_t* newOrderPtr);
    /**
     * @brief Sets the pointer to the display's data buffer.
     *
     * The display's data buffer is the array of unsigned short int (byte) that holds the value to be exhibited in each display's port.
     * @param newDspBuffPtr Pointer to the designated display's data buffer
     *
     * @warning The method just sets the value of the pointer and has no involvement in other tasks, such as:
     * - The deletion of previously assigned buffer memory space.
     * - The creation of the new buffer space
     * - Checking the legitimacy of the pointer address.
     */
    void setDspBuffPtr(uint8_t* newDspBuffPtr);
    /**
     * @brief Starts the timer and/or services needed to keep the display updated
     *
     * For dynamic displays a timer is needed to keep the cinematic effect for having all the digits lit.
     * For static displays sets the corresponding communication channels to update the display every time the display buffer contents change
     *
     * @param rfrsFrq Refreshing frequency of the dynamic display in milliseconds.
     *
     * @retval true: The timer or update services were activated without issues.
     * @retval false: The timer or update services activation failed.
     */
//    bool begin(const unsigned long &rfrsFrq = 0){return true;};
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    /**
     * @brief Stops the timer and/or services needed to keep the display updated
     *
     * For dynamic displays the needed timer will be stopped and deleted.
     * For static displays sets the corresponding communication channels are disabled.
     *
     * @retval true: The timer or update services were deactivated without issues.
     * @retval false: The timer or update services deactivation failed.
     */
    bool end();
    /**
     * @brief Keep the dynamic type displays running
     *
     * The dynamic technology displays need to be periodically refreshed, as it can actively turn on only one digit at a time, so to keep all the digits visible the driving code must activate periodically each digit one by one independently to generate a "cinematic effect".
     *
     */
    virtual void refresh(){};
};

//============================================================> Class declarations separator

/**
 * @brief Implements a generic Seven Segments LEDs dynamic hardware interface to displays the logically generated characters
 *
 * This class implements all methods and attributes common to **dynamic** hardware, including but not limited to
 * - Direct display digit manipulation with no driver chips.
 * - Shift registers arrays, with one shift register driving the leds of a port to turn on, one or more shift registers to select the port being lit.
 * Hardware specific methods and attributes will be included in corresponding subclasses.
 *
 * @class SevenSegDynamic
 */
class SevenSegDynamic: public SevenSegDispHw{
protected:
    TimerHandle_t _dspRfrshTmrHndl{NULL};
    uint8_t _firstRefreshed{0};
    TimerHandle_t _svnSgDynTmrHndl{NULL};

    virtual void refresh(){};
    static void tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg);
public:
    /**
     * @brief Default class constructor
     *
     * class SevenSegDynamic
     */
    SevenSegDynamic();
    SevenSegDynamic(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode);

    /**
     * @brief Virtual class destructor
     *
     * class SevenSegDynamic
     *
     */
    ~SevenSegDynamic();
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    virtual bool end();
};

//============================================================> Class declarations separator

/**
 * @brief Implements specific Seven Segments LEDs dynamic hardware based on 74HC595 shift register array
 *
 * The hardware targeted is implemented as a two shift register matrix, one for the leds segments to be lit, one for the port selection
 * The display implemented with this arrangement is capable of driving from 2 and up to 8 ports displays
 *
 * @class SevenSegDynHC595
 */
class SevenSegDynHC595: public SevenSegDynamic{
private:
    const uint8_t _sclkArgPos {0};
    const uint8_t _rclkArgPos {1};
    const uint8_t _dioArgPos {2};
    gpioPinId_t _sclk{};
    gpioPinId_t _rclk{};
    gpioPinId_t _dio{};
protected:
    virtual void refresh();
    static void tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg);
public:
    SevenSegDynHC595(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode);
    ~SevenSegDynHC595();
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    virtual bool end();
    void send(uint8_t content);
    void send(const uint8_t &segments, const uint8_t &port);
};

#endif /* _SEVENSEGDISPHW_H_ */
