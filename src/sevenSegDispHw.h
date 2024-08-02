/**
  ******************************************************************************
  * @file	: SevenSegDispHw.h
  * @brief	: Header file for SevenSegDispHw library classes
  *
  * @details The library builds Seven Segment Display hardware classes. Each class will provide the hardware specific implementation for the methods required by all hardware displays:
  * - Hardware pins configuration
  * - Communications protocol implementation
  * - Turn on, turn off, suspend, restart
  * - Brightness control
  * - Non-standard display amenities:
  * 	- Colons
  * 	- Apostrophes
  * 	- Non-standard icons
  *	- Display color change
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 16/11/2023
  * 			: Last modification:	11/06/2024
  * @copyright GPL-3.0 license
  *
  * @note FreeRTOS Kernel V10.3.1, some implementations have been limited to comply to the services provided by the version.
  ******************************************************************************
  * @attention	This library was developed as part of the refactoring process for an industrial machines security enforcement and control (hardware & firmware update). As such every class included complies **AT LEAST** with the provision of the attributes and methods to make the hardware & firmware replacement transparent to the controlled machines. Generic use attribute and methods were added to extend the usability to other projects and application environments, but no fitness nor completeness of those are given but for the intended refactoring project.
  * **Use of this library is under your own responsibility**
  *
  ******************************************************************************
  */

#ifndef _SEVENSEGDISPHW_H_
#define _SEVENSEGDISPHW_H_

#include <string>
//===========================>> Next lines included for developing purposes, corresponding headers must be provided for the production platform/s
#ifndef MCU_SPEC
	#define MCU_SPEC 1
	#ifndef __STM32F4xx_HAL_H
		#include "stm32f4xx_hal.h"
	#endif
	#ifndef __STM32F4xx_HAL_GPIO_H
		#include "stm32f4xx_hal_gpio.h"
	#endif
	#ifndef STM32F4xx_HAL_TIM_H
		#include "stm32f4xx_hal_tim.h"
	#endif

#endif

#define HAL_TIM_PERIOD_ELAPSED_CB_ID 0x0EU   /*!< TIM Period Elapsed Callback ID*/
//===========================>> Previous lines included for developing purposes, corresponding headers must be provided for the production platform/s


//===========================>> BEGIN libraries used to avoid CMSIS wrappers
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//#include "queue.h"
//#include "semphr.h"
//===========================>> END libraries used to avoid CMSIS wrappers

//===========================>> BEGIN User type definitions
#ifndef GPIOPINID_T
	#define GPIOPINID_T
	/**
	 * @brief Type used to keep GPIO pin identification as a single parameter, independently of the platform requirements.
	 *
	 * GPIO pin identification is hardware and development environment framework dependents, for some platforms it needs one, some two, some more parameters, and each one of these parameters' type depends once again on the platform. This type is provided to define each pin referenced as a single parameter for class methods and attributes declarations, as platform independent as possible.
	 *
	 * @struct gpioPinId_t
	 */
	struct gpioPinId_t{
		GPIO_TypeDef* portId;	/**< The port identification as a pointer to a GPIO_TypeDef information structure*/
		uint16_t pinNum;	/**< The number of pin represented as a one bit set binary with the set bit position indicating the pin number*/
	};
#endif	//GPIOPINID_T
//===========================>> END User type definitions

//===========================>> BEGIN General use function prototypes
uint8_t singleBitPosNum(uint16_t mask);
void delay10UsTck(const uint32_t &actDelay);
void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef *htim);	// Defining the TM163X Timer Register CB function

//===========================>> END General use function prototypes
//===========================>> BEGIN (Global) static variables, the use must be reduced to minimum if not completely avoided
static FlagStatus timIntFlg {RESET};
//===========================>> END (Global) static variables, the use must be reduced to minimum if not completely avoided
//============================================================> Class declarations separator

/**
 * @brief Implements a generic Seven Segments LEDs hardware interface to displays the logically generated characters
 *
 * This class implements all methods and attributes common to a large variety of seven segments led displays hardware.
 * Specific hardware methods and attributes will be included in corresponding subclasses definitions.
 *
 * @class SevenSegDispHw
 */
class SevenSegDispHw{
    static uint8_t _dspHwSerialNum;
protected:
    gpioPinId_t* _ioPins{};
    uint8_t* _digitPosPtr{nullptr};
    uint8_t _dspDigitsQty{}; //Display size in digits
    const uint8_t _dspDigitsQtyMax{}; // Maximum display size in digits, hardware dependent
    bool _commAnode {true}; //SevenSegDisplays objects will retrieve this info to build the right segments for each character

    uint8_t _brghtnssLvls{0};
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspHwInstNbr{0};

	 virtual void send(uint8_t* digitsBuffer){};
	 virtual void send(const uint8_t &segments, const uint8_t &port){};
public:
    /**
     * @brief Default constructor
     *
     * class SevenSegDispHw
     */
    SevenSegDispHw();
    /**
     * @brief Class constructor
     *
     * @param ioPins Pointer to a gpioPinId_t array with the needed MCU pins identification to drive the hardware display.
     * @param dspDigits Display's length in digits quantity (dspDigitsQty attribute).
     * @param commAnode Indicates the display leds wiring scheme, either **common anode** (true) or **common cathode** (false)
     *
     * @class SevenSegDispHw
     */
    SevenSegDispHw(gpioPinId_t* ioPins, uint8_t dspDigits = 4, bool commAnode = true);
    /**
     * @brief Class virtual destructor
     *
     * @class SevenSegDispHw
     */
    virtual ~SevenSegDispHw();
    /**
     * @brief Starts the timer and / or services needed to keep the display updated
     *
     * For dynamic displays a timer is needed to keep the cinematic effect of having all the digits displaying it's content at the same time.
     * For static displays sets the corresponding configuration of the display driving chipset.
     *
     * @param rfrsFrq Refreshing frequency of the dynamic display in milliseconds.
     *
     * @retval true: The timer for update services or configuration parameters were correctly set.
     * @retval false: The timer or update services activation failed.
     */
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    /**
     * @brief Loads the hardware display driver's internal buffer with the current display's data buffer contents.
     *
     * The method is invoked every time the display's data buffer contents change. This mechanism implementation avoids the need of periodically checking the display's data buffer for changes. The standard invocation of this method is done by the SevenSegDisplays class methods that modify the display's data buffer contents (print(), write(), blink(), wait() and others).
     *
     * @note The dynamic technology displays usually don't have internal buffers and need constant reading of the display buffer to refresh the displaying content. For this kind of displays this method is unneeded, so it might be not implementation for those sub-clases, or might be implemented as an empty method.
     */
    virtual void dspBffrCntntChng(){};
    /**
     * @brief Stops the timer and/or services needed to keep the display updated
     *
     * For dynamic displays the needed timer will be stopped and deleted.
     * For static displays sets the corresponding communication channels as disabled.
     *
     * @retval true: The timer or update services were deactivated without issues.
     * @retval false: The timer or update services deactivation failed.
     */
    bool end();
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
     * @retval false: At least one of the values of the array parameter is out of range. The change wasn't performed.
     *
     * @note Each value in the array passed as argument will be checked against the _dspDigits value to ensure that they are all in the range acceptable, 0 <= value <= _dspDigits - 1. If one of the values is out of the valid range no change will be done. Please note that no checking will be done to ensure all of the array values are different. A repeated value will be accepted, leading to unexpected display behavior due to superimposing digits and not including digits.
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
     * @brief Keeps the dynamic type displays running by redisplaying the content of each display port.
     *
     * The dynamic technology displays need to be periodically refreshed, as it can actively turn on only one digit at a time, so to keep all the digits visible the driving code must activate periodically each digit one by one independently to generate a "cinematic effect".
     */
//    virtual void refresh(){};
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
     */
    SevenSegDynamic();
    /**
     * @brief Class constructor
     *
     * @param ioPins Pointer to a gpioPinId_t array with the needed MCU pins identification to drive the hardware display.
     * @param dspDigits Display's length in digits quantity (dspDigitsQty attribute).
     * @param commAnode Indicates the display leds wiring scheme, either **common anode** (true) or **common cathode** (fa.lse)
     */
    SevenSegDynamic(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode);
    /**
     * @brief Virtual class destructor
     */
    virtual ~SevenSegDynamic();
    /**
     * @brief Starts the timer and / or services needed to keep the display updated
     *
     * For dynamic displays a timer is needed to keep the cinematic effect of having all the digits displaying it's content at the same time.
     *
     * @param rfrsFrq Refreshing frequency of the dynamic display in milliseconds.
     *
     * @retval true: The timer or update services were activated without issues.
     * @retval false: The timer or update services activation failed.
     */
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    /**
     * @brief Stops the timer and/or services needed to keep the display updated
     *
     * The dynamic displays' refreshing timer will be stopped and deleted.
     *
     * @retval true: The timer or update services were deactivated without issues.
     * @retval false: The timer or update services deactivation failed.
     */
    virtual bool end();
};

//============================================================> Class declarations separator

/**
 * @brief Implements specific Seven Segments LEDs dynamic display hardware based on 74HC595 shift register array
 *
 * The hardware targeted is implemented as a two shift register matrix, one for the leds segments to be lit, one for the port selection
 * The display implemented with this arrangement is capable of driving from 2 and up to 8 ports displays
 *
 * @class SevenSegDynHC595
 */
class SevenSegDynHC595: public SevenSegDynamic{
private:
	const uint8_t _dspDigitsQtyMax{8}; // Maximum display size in digits, hardware dependent
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
    /**
     * @brief Class constructor
     *
     * Instantiates a seven segments display driven dynamically by an array of two 74HC595 8 bits shift registers.
     *
     * @param ioPins Pointer to a gpioPinId_t array of three elements, with the needed MCU pins identification to drive the hardware display pins: SCLK, RCLK and DIO
     * @param dspDigits Display's length in digits quantity (dspDigitsQty attribute).
     * @param commAnode Indicates the display leds wiring scheme, either **common anode** (true) or **common cathode** (false)
     */
    SevenSegDynHC595(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode);
    /**
     * @brief Virtual destructor
     */
    ~SevenSegDynHC595();
    /**
     * @brief See SevenSegDynamic::begin(const unsigned long int)
     */
    virtual bool begin(const unsigned long int &rfrshFrq = 0);
    /**
     * @brief See SevenSegDynamic::end()
     */
    virtual bool end();
    /**
     * @brief Sends a 8 bits value to a 74HC595 shift register
     *
     * The byte value is sent through the manipulation of the DIO and SCLK lines according to the IC specifications to it's internal 8 bits buffer
     *
     * @param content The 8 bits value to send to the IC internal 8 bits buffer
     */
    void send(uint8_t content);
    /**
     * @brief Sends a character to the display, the port (position) where it must be displayed and generate the displaying of the character.
     *
     * The displaying mechanism consist of the following steps:
     * - Send a byte consisting of the character to display using send(uint8_t)
     * - Send a byte consisting of the position in the display to show the character using send(uint8_t)
     * - Manipulate the RCLK line to "Lock & Present" the value to the Seven Segments Display module.
     *
     * @param segments A character set as a 8 bits value representing the segments to lit to form graphic representation of such character
     * @param port A 8 bits value representing the position on the display to show the character, often as a single bit set byte.
     */
    void send(const uint8_t &segments, const uint8_t &port);
};

//============================================================> Class declarations separator

class SevenSegStatic: public SevenSegDispHw{

public:
	SevenSegStatic(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode);
	virtual ~SevenSegStatic();
};

//============================================================> Class declarations separator

/**
 * @brief Implements specific Seven Segments LEDs static displays hardware based on Titan Micro TM163X series chips
 *
 * As TM163X series chips have some differences among them, this class implements the base common characteristics, de differences are implemented in corresponding subclasses.
 *
 * Common attributes include:
 * - Communications protocol (a non standard variation of the I2C protocol).
 * - Commands structure.
 * - Read/Write commands.
 * - Brightness commands, capabilities and values.
 * - Start and Stop commands.
 *
 * Different attributes include:
 * - Maximum number of ports addressable.
 *
 * @note As the communications protocol does't comply with the I2C protocol, the communications must be implemented in software. For that reason, for resources saving sake, the CLK speed will be reduced from the data sheet **Maximum clock frequency** stated as 500KHz to a less demanding 100KHz time slices, managed by a timer interrupt set at 100KHz, enabling the timer interrupt service only while transmitting data, and disabling it while idle. The transmission protocol will implement the data sheet requirements, part of it being the use of a 0.5 long CLK multiples for signaling, so **TWO** 100 KHz periods will be set as the CLK width standard.
 *
 * @class SevenSegTM163X
 */
class SevenSegTM163X: public SevenSegStatic{
   static TIM_HandleTypeDef _txTM163xTmr;
	static uint8_t _usTmrUsrs;

protected:
   gpioPinId_t _clk{};
   const uint8_t _clkArgPos {0};
   gpioPinId_t _dio{};
   const uint8_t _dioArgPos {1};

   uint8_t _brghtnss{};
   const uint8_t _brghtnssLvlMax{};
   const uint8_t _brghtnssLvlMin{};
   uint8_t* _mssgBffr{nullptr};
   uint8_t _mssgBffrLngth{0};

   bool _turnOff();
   bool _turnOn();

   void delay10UsTck(const uint32_t &actDelay);
   void send(const uint8_t* data, const uint8_t dataQty);
   void _txStart();
   void _txAsk();
   void _txStop();
   void _txWrByte(const uint8_t data);

public:
	SevenSegTM163X(gpioPinId_t* ioPins, uint8_t dspDigits);
	~SevenSegTM163X();
	bool begin(TIM_HandleTypeDef &newTxTM163xTmr);
	virtual void dspBffrCntntChng();
	bool end();
	bool getBrghtnss();
	bool getBrghtnssMaxLvl();
	bool getBrghtnssMinLvl();
	bool setBrghtnss(const uint8_t &newBrghtnssLvl);

};

//============================================================> Class declarations separator

class SevenSegTM1637: public SevenSegTM163X{
protected:
	const uint8_t _dspDigitsQtyMax{6}; // Maximum display size in digits, hardware dependent
   const uint8_t _brghtnssLvlMax{7};
   const uint8_t _brghtnssLvlMin{0};
public:
	SevenSegTM1637(gpioPinId_t* ioPins, uint8_t dspDigits);
	~SevenSegTM1637();

};

//============================================================> Class declarations separator

class SevenSegTM1639: public SevenSegTM163X{
protected:
	const uint8_t _dspDigitsQtyMax{16}; // Maximum display size in digits, hardware dependent
public:
	SevenSegTM1639(gpioPinId_t* ioPins, uint8_t dspDigits);
	~SevenSegTM1639();

};

//============================================================> Class declarations separator

#endif /* _SEVENSEGDISPHW_H_ */
