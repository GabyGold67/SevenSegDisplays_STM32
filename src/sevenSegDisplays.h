/**
  ******************************************************************************
  * @file	: SevenSegDisplays.h
  * @brief	: Header file for SevenSegDisplays library classes
  *
  * @details This library builds Seven Segments LEDs displays independently from the hardware involved to execute the display. The included attributes and methods are hardware implementation agnostic, the hardware used might be changed by design requirements, and several different hardware display models might be used at the same time with the same API even if they are based on different driver chips or management technologies.
  *
  * @author	: Gabriel D. Goldman
  * @version v1.0.0
  * @date	: Created on: 16/11/2023
  * 			: Last modification:	11/06/2024
  * @copyright GPL-3.0 license
  *
  ******************************************************************************
  * @attention	This library was developed as part of the refactoring process for an industrial machines security enforcement and control (hardware & firmware update). As such every class included complies **AT LEAST** with the provision of the attributes and methods to make the hardware & firmware replacement transparent to the controlled machines. Generic use attribute and methods were added to extend the usability to other projects and application environments, but no fitness nor completeness of those are given but for the intended refactoring project.
  * **Use this library is under your own responsibility**
  *
  ******************************************************************************
  */

#ifndef _SEVENSEGDISPLAYS_STM32_H_
	#define _SEVENSEGDISPLAYS_STM32_H_

#include "SevenSegDispHw.h"

#include <string>
//===========================>> Next lines included for developing purposes, corresponding headers must be provided for the production platform/s
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
//===========================>> Previous lines included for developing purposes, corresponding headers must be provided for the production platform/s

//===========================>> Next lines provided to use FreeRTOS and avoid CMSIS wrappers
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//#include "queue.h"
//#include "semphr.h"
//===========================>> Previous lines provided to use FreeRTOS and avoid CMSIS wrappers

// Maximum limit constants, those are provided just to avoid lack of resources errors in the testing stage. They might be changed to lower or higher values, as much as needed and as long as no lack of resources errors are triggered
const int MAX_DIGITS_PER_DISPLAY{16};
const int MAX_DISPLAYS_QTY{16};

//============================================================> Class declarations separator

/**
 * @brief Implements a seven segment display object.
 *
 * The class implements a hardware independent API for seven segment display objects. The data to be shown by the physical display, after generated, is stored and passed to the underlying hardware display.
 *
 * @class SevenSegDisplays
 */
class SevenSegDisplays {
    static uint8_t _displaysCount;
    static uint16_t _dspSerialNum;
    static uint8_t _dspPtrArrLngth;
    static SevenSegDisplays** _instancesLstPtr;

    static void tmrCbBlink(TimerHandle_t blinkTmrCbArg);
    static void tmrCbWait(TimerHandle_t waitTmrCbArg);
private:
    uint8_t _waitChar {0xBF};
    uint8_t _waitCount {0};
    bool _waiting {false};
    unsigned long _waitRate {250};
    unsigned long _waitTimer {0};
protected:
    const unsigned long _minBlinkRate{100};
    const unsigned long _maxBlinkRate{2000};

    bool _blinking{false};
    bool* _blinkMaskPtr{nullptr};
    bool _blinkShowOn{false};
    unsigned long _blinkOffRate{500};
    unsigned long _blinkOnRate{500};
    unsigned long _blinkRatesGCD{500};  //Holds the value for the minimum timer checking the change ON/OFF of the blinking, saving unneeded timer interruptions, and without the need of the std::gcd function.
    unsigned long _blinkTimer{0};
    TimerHandle_t _blinkTmrHndl{NULL};
    uint8_t* _dspAuxBuffPtr{nullptr};
    uint8_t* _dspBuffPtr{nullptr};
    uint8_t _dspDigitsQty{};
    SevenSegDispHw* _dspUndrlHwPtr{};
    SevenSegDisplays* _dspInstance;
    uint16_t _dspInstNbr{0};
    int32_t _dspValMax{};
    int32_t _dspValMin{};
    std::string _charSet{"0123456789AabCcdEeFGHhIiJLlnOoPqrStUuY-_=~* ."}; // for using indexOf() method
    uint8_t _charLeds[45] = {   //Values valid for a Common Anode display. For a Common Cathode display values must be logically bit negated
        0xC0, // 0
        0xF9, // 1
        0xA4, // 2
        0xB0, // 3
        0x99, // 4
        0x92, // 5
        0x82, // 6
        0xF8, // 7
        0x80, // 8
        0x90, // 9
        0x88, // A
        0xA0, // a
        0x83, // b
        0xC6, // C
        0xA7, // c
        0xA1, // d
        0x86, // E
        0x84, // e
        0x8E, // F
        0xC2, // G
        0x89, // H
        0x8B, // h
        0xF9, // I
        0xFB, // i
        0xF1, // J
        0xC7, // L
        0xCF, // l
        0xAB, // n
        0xC0, // O
        0xA3, // o
        0x8C, // P
        0x98, // q
        0xAF, // r
        0x92, // S
        0x87, // t
        0xC1, // U
        0xE3, // u
        0x91, // Y
        0xBF, // Minus -
        0xF7, // Underscore _
        0xB7, // Low =
        0xB6, //~ for Equivalent symbol
        0x9C, // °
        0xFF, // Space
        0x7F  //.
    };
    uint8_t _dot {0x7F};
    uint8_t _space {0xFF};
    std::string _spacePadding{""};
    TimerHandle_t _waitTmrHndl{NULL};
    std::string _zeroPadding{""};

    unsigned long blinkTmrGCD(unsigned long blnkOnTm, unsigned long blnkOffTm);
    void restoreDspBuff();
    void saveDspBuff();
    void setAttrbts();
    void updBlinkState();
    void updWaitState();

public:
    /**
     * @brief Class constructor
     *
     * @param dspUndrlHwPtr A pointer to the underlying hardware display, a SevenSegDispHw instantiable class object.
     *
     *  The underlying hardware display is the physical device used to display the information generated by this class objects, shared through it's buffer.
     *  As such the underlying display has physical properties that might condition some of this object's characteristics -some has no practical incidence- including:
     *  - Digits (ports) quantity.
     *  - Type of led connection (common Anode/common Cathode).
     *  - Turn On/Turn Off control
     *  - Quantity and position of semicolons, apostrophes and special icons.
     *  - Managed brightness levels.
     *  - Display color change.
     */
    SevenSegDisplays(SevenSegDispHw* dspUndrlHwPtr);
    /**
     * @brief Virtual class destructor
     */
    ~SevenSegDisplays();
    /**
     * @brief Starts and initializes the underlying hardware services needed to keep the display data visible and updated.
     *
     * Depending on the underlying hardware characteristics, several tasks and services might be configured and started.
     * As these activities are hardware-aware each **SevenSegDispHw** subclass must implement those initialization tasks in it's own begin() method.
     *
     * @return The value returned by the **SevenSegDispHw** subclass implementation of the begin() method
     * @retval true: the hardware related services could be started as needed
     * @retval false: the hardware related services could not be started as needed
     */
    virtual bool begin();
    /**
     * @brief Makes the displayed contents blink, by turning periodically off and then turning on the display ports.
     *
     * Once started the display will continue blinking even if the contents are changed. By default all the digits are set to blink, but each digit might be configured individually to blink or not by using the setBlinkMask(const bool*) method. When invoking this method with no parameters the existing blinking times parameters -stating the time the blinking ports will stay off an then the time they will stay on- will be used. The blinking times are set at instantiation time to generate a symmetrical blinking effect, meaning that the time the display shows the contents and the time the display is blank are equal.
     *  The display blinks until a noBlink() method is invoked, or until a wait() is invoked, as **blinking** and **waiting** are mutually exclusive states.
     * The blinking rate can be changed by using the setBlinkRate(const unsigned long, const unsigned long) method. After changing the blinking rate, the new blinking rate will be kept until it is modified with a new setBlinkRate(const unsigned long, const unsigned long) call, or it is restarted by a blink(const unsigned long, const unsigned long) with parameters.
     * @note To restart the blinking with a blink() method invocation the service must first be stopped, as the blink() method call has no effect if the blinking service was already running.
     *
     * @retval true: If the display was not already set to blink and the blinking was successfully started, or if it was already running.
     * @retval false: The display could not be set for blink due to errors setting the timer that controls the blinking effect.
     */
    bool blink();
    /**
     * @brief Makes the displayed contents blink using the times passed as parameters.
     *
     * This blink() method implementation first calls the setBlinkRate(const unsigned long, const unsigned long) method for setting the time parameters. If the time parameters update succeed, a blink() -no parameters- is invoked afterwards.
     *
     * @param onRate Time (in milliseconds) the display must stay on, the value must be in the range minBlinkRate <= onRate <= maxBlinkRate. Those preset values can be known by the use of the getMinBlinkRate() and the getMaxBlinkRate() methods.
     * @param offRate (Optional) Time (in milliseconds) the display must stay off, the value must be in the range minBlinkRate <= offRate <= maxBlinkRate. Those preset values might be known by the use of the getMinBlinkRate() and the getMaxBlinkRate() methods. If no offRate value is provided the method will assume it's a symmetric blink call and use a value for offRate equal to the value passed for onRate.
     *
     * @retval true: The conditions to start blinking with the passed parameters were met, so the blinking starts.
     * @retval false: The conditions to start blinking with the passed parameters were **NOT** met, no changes could be made to the blinking state.Those conditions include:
     * - One or more of the parameters passed was out of range. The blinking won't be started.
     * - The blink() invocation failed as described in the method (timer activation failure).
     */
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    /**
     * @brief Clears the display, turning off all the segments and dots.
     *
     * @note The clear() method can be invoked while the display is in "blinking state" or "waiting state". In the first case the effect will be not visible, as it will imply a empty display turning on and off, no visual clue will be perceived. In the second case, after the "wait state" ends the original display will not be restored as usual, but an empty display will be set after the noWait() is invoked.
     *
     * @note The clearing process involves writing a **space** character to every display port. Due to physical display characteristics this might be or not be equal to writing a '\0' to them.
     */
    void clear();
    /**
     * @brief Displays a basic graphical representation of the level of fulfillment or completeness of **two** segmented values or tasks.
     *
     *  The display gives a general fast notion on the matter, as a battery charge level, liquids deposit level, time remaining, tasks completeness and so on. The levels are represented by the horizontal segments (0, 1, 2 or 3 from bottom to top), and a character that might be added before each of the graphical representations to give an indication of what the display is showing. As four digit ports must be used, this method only applies to displays with 4 or more digits.
     *  The display is splitted in two sectors, the left side and the right side, and each one of them must have a valid value (0 <= value <= 3) to enable them to be displayed, and might have (or not) a single displayable character to give a visual hint to what the value is showing.
     *  For more information see the gauge(const int, char) method.
     *
     * @param levelLeft The value to display for the two left side 7 segments display ports, must be in the range 0 <= level <= 3.
     * @param levelRight The value to display for the two right side 7 segments display ports, must be in the range 0 <= level <= 3.
     * @param labelLeft (Optional) A char that will be displayed in the leftmost port of the display. The character must be one of the "displayable" characters, as listed in the **print(std::string)** method description. If not specified the default value, a **space**, will be assumed,
     * @param labelRight (Optional) A char that will be displayed in the position left to the **levelRight** display port of the display. The character must be one of the "displayable" characters, as listed in the **print(std::string)** method description. If not specified the default value, a **space**, will be assumed,
     *
     * @retval true: The values could be represented.
     * @retval false: The values couldn't be represented, being that the **levelLeft** and/or **levelRight** parameter was out of range and/or the **labelLeft** and/or **labelRight** parameter was not in the list of displayable characters. The display will be blanked.
     */
    bool doubleGauge(const int &levelLeft, const int &levelRight, char labelLeft = ' ', char labelRight = ' ');
    /**
     * @brief Stops and ends the underlying hardware services needed to keep the display data updated and visible
     *
     * Depending on the underlying hardware several tasks and services were configured and started by the **begin()** method.
     * As those configuration activities are hardware-aware each **SevenSegDispHw** subclass implement those initialization tasks, and each subclass must implement the correct termination of those tasks and services if required.
     * The **end()** method invokes the underlying hardware **end()** method
     *
     * @return The value returned by the **SevenSegDispHw** subclass implementation of the end() method
     * @retval true: the hardware related services could be stopped and ended as needed
     * @retval false: the hardware related services could not be stopped or ended as needed
     */
    virtual bool end();
    /**
     * @brief Displays a basic graphical representation of the level of fulfillment or completeness of a segmented value or task.
     *
     * The display gives a general fast notion on the matter, as a battery charge level, liquids deposit level, time remaining, tasks completeness and so on. The levels are represented by the horizontal segments (0, 1, 2 or 3 from bottom to top, and from left to right), and a character might be added before -leftmost port- the graphical representation to give an indication of what the display is showing. This method is usable in displays which have from 4 digits and up, as the representation makes use of 4 digits.
     *
     * @param level The integer value to display, must be in the range 0 <= level <= 3.
     * @param label (Optional) A character that might be added before (leftmost port) the graphical representation to give an indication of what the display is showing, if not specified the default value, a space, will be assumed. The character must be one of the "displayable" characters, as listed in the print(std::string) method description.
     *
     * @return The success in the display of the intended value.
     * @retval true: The value could be represented.
     * @retval false: The value couldn't be represented, being that:
     * - The **level** parameter was out of range
     * - The **label** parameter was not in the list of displayable characters.
     * The display will be blanked.
     */
    bool gauge(const int &level, char label = ' ');
    /**
     * @brief Displays a basic graphical representation of the level of fulfillment or completeness of a segmented value or task.
     *
     * See gauge(const int, char) for details
     *
     * @param level The double value to display, must be in the range 0.0 <= level <= 1.0, being the range the representation of the percentage of the 'full' level, so that the ranges are:
	  * 0.0 <= level < 0.25 for the first level,
	  * 0.25 <= level < 0.50 for the second level,
	  * 0.50 <= level < 0.75 for the third level, and
	  * 0.75 <= level <= 1.00 for the fourth and upper level.
     * @param label (Optional) A character that might be added before (leftmost port) the graphical representation to give an indication of what the display is showing, if not specified the default value, a space, will be assumed. The character must be one of the "displayable" characters, as listed in the print(std::string) method description.
     *
     * @return The success in the display of the intended value.
     * @retval true: The value could be represented.
     * @retval false: The value couldn't be represented, being that:
     * - The **level** parameter was out of range
     * - The **label** parameter was not in the list of displayable characters.
     * The display will be blanked.
     */
    bool gauge(const double &level, char label = ' ');
    /**
     * @brief Gets the quantity of ports -digits- of the display as informed by the underlying hardware (**UHW**)
     *
     * The object instantiation requires a **underlying physical display object** as parameter, and such information is requested from that object. Each time this class is instantiated the object is created with the needed resources -display memory buffer, auxiliary buffers, blinking mask, etc.), and the range of values displayable are calculated based on the display's ports quantity parameter, and that value is the one returned by this method.
     *
     * @return The number indicating the quantity of digits of the instantiated display.
     */
    uint8_t getDigitsQty();
    /**
     * @brief Gets the maximum integer number value displayable by the display.
     *
     * The value is calculated according to the quantity of digits -ports- the display has as retrieved from the underlying hardware (**UHW**) at the object instantiation.
     *
     * @return The greatest integer value display capability.
     */
    int32_t getDspValMax();
    /**
     * @brief Gets the minimum integer number value displayable by the display.
     *
     * The value is calculated according to the quantity of digits -ports- the display has as retrieved from the underlying hardware (**UHW**) at the object instantiation.
     *
     * @return The smallest integer value display capability.
     *
     * @note The use of one port to represent the **-** (minus) sign is considered in the value calculation.
     */
    int32_t getDspValMin();
    /**
     * @brief Gets the instantiation number it was given to the object.
     *
     * Each time the class is instantiated the created object receives a serial instantiation number that can be used in order to easily identify each object in case of need.
     *
     * @return The instantiation serial number of the object.
     */
    uint16_t getInstanceNbr();
    /**
     * @brief Gets the maximum value the blink rates for the blink() method might take.
     *
     * The speed at witch the physical displays might be turned on and off is limited. In the case of this parameter a too high value might void the blinking strobe effect of being perceived, giving the sensation that the display is fixed on, or was cleared from values. So a higher limit is imposed.
     *
     * @return The maximum value that might be used as a blink() function parameter.
     *
     * @note This value is also the maximum parameter accepted by the following methods: blink(const unsigned long, const unsigned long), setBlinkRate(const unsigned long, const unsigned long), wait(const unsigned long) and setWaitRate(const unsigned long)
     */
    unsigned long getMaxBlinkRate();
    /**
     * @brief Gets the minimum value the blink rates for the blink() method might take
     *
     * The speed at witch the physical displays might be turned on and off are limited. In the case of this parameter a too low value might avoid the blinking strobe effect of being perceived, or be not tolerated by the underlying hardware, giving the sensation that the display is fixed on, or was cleared from values. So a lower limit is imposed.
     *
     * @return The minimum value that might be used as a blink() function parameter.
     *
     * @note This value is also the minimum parameter accepted by the following methods: blink(const unsigned long, const unsigned long), setBlinkRate(const unsigned long, const unsigned long), wait(const unsigned long) and setWaitRate(const unsigned long)     */
    unsigned long getMinBlinkRate();
    /**
     * @brief Returns a value indicating if the display is blank
     *
     * @retval true: all the display ports hold a **space** value.
     * @retval false: at least one of the display ports hold a value different to a **space** value.
     *
     * @note The condition to be blank is that all the display ports are exhibiting the "space" character, not to be confused with '\0'
    */
    bool isBlank();
    /**
     * @brief Gets if the display is in **blinking mode**.
     *
     * @retval true: The display is set to blink.
     * @retval false: The display is set to not blink.
     *
     */
    bool isBlinking();
    /**
     * @brief Gets if the display is in **waiting mode**.
     *
     * @retval true: The display is set in waiting mode.
     * @retval false: The display is not set in waiting mode.
     */
    bool isWaiting();
    /**
     * @brief Stops the display blinking, if it was doing so, leaving the display turned on.
     *
     * @retval true: The display is set not to blink.
     * @retval false: The display was set to blink but the blinking couldn't be stopped due to timer management failures.
     * @warning The failure of the method might indicate a bug in the development. Direct manipulation of the handles and pointers to the resources (or the resources themselves) used by the blinking mechanism might disable this method capabilities to correctly exiting the **blinking state**
     */
    bool noBlink();
    /**
     * @brief Stops the **waiting mode** in process, if the display was in it.
     *
     * @retval true: The display is set out of **waiting mode**, and the waiting is stopped.
     * @retval false: The display was set to wait, but the waiting couldn't be stopped due to timer management failures.
     *
     * @warning The failure of the method might indicate a bug in the development. Direct manipulation of the handles and pointers to the resources (or the resources themselves) used by the waiting mechanism might disable this method capabilities to correctly exiting the **waiting state**
      */
    bool noWait();
    /**
     * @brief Displays a string.
     *
     * Displays the string text if it contains all **displayable** characters, which are the ones included in the following list: **0123456789AabCcdEeFGHhIiJLlnOoPqrStUuY-_.** and the **space**.
     * There are other 3 characters that can be represented in the display, but the conversion from a character to use while programming is "host language setting dependent", so those where assigned to available ASCII non displayable characters of easy access in any keyboard layout in most languages, they can be used as part of the text string to display, and they are:
     *
     * = Builds a character formed by lighting the lower 2 horizontal segments of the digit display, can be described as a "lower equal" symbol.
     *
     * ~ Builds a character formed by lighting the 3 horizontal segments of the digit display, can be described as an "equivalent" symbol.
     *
     * _*_ (asterisk) Builds a character by lighting the upper 4 segments, forming a little square, can be described as the "degrees" symbol or º.
     *
     * @param text String, up to **DigitsQty** -see getDigitsQty()- displayable characters long PLUS usable dots, all characters included in the representable characters list. Each valid character might be followed by a "." if needed, without being counted as a character, even spaces and special chars. If two or more consecutive dots are passed, an intermediate space is considered to be included between each pair of them, and that space counts as one of the available characters to display.
     *
     * @retval true: The text could be represented.
     * @retval false: The text couldn't be represented, and the display will be blanked.
     *
     * @note Several reasons might be causing a string to not display, including:
     * - The inclusion of a character not part of the **displayable list**.
     * - The text is longer than the object's DigitsQty (dots are not included in the count).
     * - The use of two or more consecutive "." -dots- which implies the addition of a space between them, and failing to take them into the account of the text length.
     *
     * @note If a print(std::string) method is invoked when the display is in **wait mode**, the method will end the **wait mode** by invoking a noWait() method.
     */
    bool print(std::string text);
    /**
     * @brief Displays an integer value.
     *
     *  The value will be displayed as long as the length of the representation fits the available digits of the display
     *
     * @param value The integer value to display which must be in the range (-1)*(pow(10, (dspDigits - 1)) - 1) <= value <= (pow(10, dspDigits) - 1)
     * @param rgtAlgn (Optional, if not specified the default value, **false**, will be assumed) Indicates if the represented value must be displayed right aligned, with the unneeded heading characters being completed with spaces or zeros, depending in the **zeroPad** optional parameter. When a negative value is displayed and it's less than (dspDigits - 1) digits long, a right aligned display will keep the '-' sign in the leftmost position, and the free space to the leftmost digit will be filled with spaces or zeros, depending in the **zeroPad** optional parameter.
     * @param zeroPad (Optional, if not specified the default value, false, will be assumed) indicates if the heading free spaces of the integer right aligned displayed must be filled with zeros (true) or spaces (false). In the case of a negative integer the spaces or zeros will fill the gap between the '-' sign kept in the leftmost position, and the first digit.
     *
     * @retval true: The value could be represented.
     * @retval false: The value couldn't be represented, and the display will be blanked.
     *
     */
    bool print(const int32_t &value, bool rgtAlgn = false, bool zeroPad = false);
    /**
     * @brief Displays a floating point value.
     *
     * The value will be displayed as long as the length representation fits the available space of the display. If the integer part of value is not in the displayable range or if the sum of the spaces needed by the integer part plus the indicated decimal places to display is greater than the available digits space, the method will fail.
     *
     * @param value The floating point value to display, which must be in the range ((-1)*(pow(10, ((dspDigits - decPlaces) - 1)) - 1)) <= value <= (pow(10, (dspDigits - decPlaces)) - 1).
     * @param decPlaces Decimal places to be displayed after the decimal point, ranging 0 <= decPlaces < dspDigits, selecting 0 value will display the number as an integer, with no '.' displayed. In any case the only modification that will be applied if value has a decimal part longer than the decPlaces number of digits is **truncation**, if any other rounding criteria is desired the developer must apply it to **value** before calling this method.
     * @param rgtAlgn Right alignement, see print(const int32_t, bool, bool)
     * @param zeroPad Zero padding, see print(const int32_t, bool, bool)
     *
     * @retval true: The value could be represented.
     * @retval false: The value couldn't be represented, and the display will be blanked.
     */
    bool print(const double &value, const unsigned int &decPlaces, bool rgtAlgn = false, bool zeroPad = false);
    /**
     * @brief Resets the blinking mask.
     *
     * The blinking mask configures which digits of the display will be affected by the blink() method, resetting the mask will restore the original setting by which all the ports of the display will be affected when **blinking mode** is active.
     */
    void resetBlinkMask();
    /**
     * @brief Modifies the blinking mask.
     *
     * The blinking mask indicates which digits will be involved after a blink() method is invoked. Indicating true for a digit makes it blink when the method is called, indicating false makes it display steady independently of the others. The parameter is positional referenced to the display, and for ease of use the index numbers of the parameter array indicate their position relative to the rightmost digit (blnkPort0). The mask might be reset to its original value (all digits set to blink) by using this method with all array's elements set to **true** or by using the resetBlinkMask() method.
     *
     * @param newBlnkMsk A pointer to a boolean array the size of DigitsQty long, the first element of the array corresponding to the leftmost display port, the following element representing the next display port to it's left. A true value in the array element will set the corresponding display port to blink when the blink() methods are invoked, a false will make it's displayed value steady.
     */
    void setBlinkMask(const bool* newBlnkMsk);
    /**
     * @brief Changes the time parameters to use for the display blinking the contents it shows.
     *
     * The parameters change will take immediate effect, either if the display is already blinking or not, in the latter case the parameters will be the ones used when a blink() method is called without parameters. The blinking will be **symmetrical** if only one parameter is passed, **asymmetrical** if two different parameters are passed, meaning that the time the display shows the contents and the time the display is blank will be equal (symmetrical) or not (asymmetrical), depending of those two parameters. The blink rate set will be kept after a `noBlink()` or new `blink()` without parameters call is done, until it is modified with a new `setBlinkRate()` call, or it is restarted by a `blink()` with parameters.
     *
     * @note To restart the blinking with a `blink()` the service must first be stopped, as the method makes no changes if the blinking service was already running.
     *
     * @param newOnRate Contains the time (in milliseconds) the display must stay on, the value must be in the range minBlinkRate <= onRate <= maxBlinkRate. Those built-in values can be known by the use of the `getMinBlinkRate()` and the `getMaxBlinkRate()` methods.
     * @param newOffRate (Optional) Contains the time (in milliseconds) the display must stay off, the value must be in the range minBlinkRate <= offRate <= maxBlinkRate. Those built-in values can be known by the use of the `getMinBlinkRate()` and the `getMaxBlinkRate()` methods. If no offRate value is provided the method will assume it's a symmetric blink call and use a value of offRate equal to the value passed by onRate.
     *
     * @retval true: If the parameter or parameters passed are within the valid range, and the change will take effect immediately.
	  * @retval false: One or more of the parameters passed were out of range. The rate change would not be made for none of the parameters.
     *
     */
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    /**
     * @brief Sets the character to use when the display is in **wait mode**.
     *
     * The parameters change will take immediate effect, either if the display is already in wait mode or not. The new character will be changed for further calls of the method until a new setWaitChar() is invoked with a valid argument.
     *
     * @param newChar A character the display must use for symbolizing the progress, the value must be in the displayable characters list as explained in the print(std::string) method.
     *
     * @retval true: If the character passed is within the displayable characters range, and the change will take effect immediately.
     * @retval false: The parameter passed was invalid, i.e. it was a non displayable character. In this case the character change will fail.
     */
    bool setWaitChar(const char &newChar);
    /**
     * @brief Changes the time parameter to use for the display to show the "in-progress bar" advancement.
     *
     * The parameters change will take immediate effect, either if the display is already in wait mode or not, in the latter case the parameter will be used in the next `wait()` method call without parameters. The wait rate set will be kept until it is modified with a new `setWaitRate()` call, or it is restarted by a `wait(const unsigned long)` with parameters. Note that to restart the waiting with a `wait()` the service must first be stopped, as the method makes no changes if the waiting service was already running.
     *
     * @param newWaitRate The time (in milliseconds) the display must take to advance the next character symbolizing the progress, the value must be in the range minBlinkRate <= newWaitRate <= maxBlinkRate. (See `getMinBlinkRate()`and `getMaxBlinkRate()` methods).
     *
     * @retval true: The parameter passed is within the valid range, and the change takes immediate effect.
     * @retval false: The parameter passed is out of range, the rate change would not be made.
     *
     */
    bool setWaitRate(const unsigned long &newWaitRate);
    /**
     * @brief Makes the display show a "simple animated progress bar".
     *
     * The simple animation mechanism has as it's main purpose to show the final user the system is working and not in any "hang" situation. As in most O.S. progress bar animation, the sequence starts with  a blank display and a defined character is added from left to right at a configured pace, until all the ports are lit, starting over with the display blanking. The rate at which the characters are added and the character used for display are both configurable (see `setWaitChar(const char)` and setWaitRate(const unsigned long) for details).
     *
     * @retval true: The display was not in **waiting mode** and the required timer could be attached. The **waiting mode** is started.
     * @retval false: The display was in **waiting mode**, no change was made.
     * @retval false: The timer couldn't be attached, the display wasn't set to **wait mode**
     */
    bool wait();
    /**
     * @brief  Makes the display show a "simple animated progress bar".
     *
     * See `wait()` for details.
     *
     * @param newWaitRate The time (in milliseconds) the display must take to advance the next character symbolizing the progress, the value must be in the range minBlinkRate <= newWaitRate <= maxBlinkRate. (See `getMinBlinkRate()`and `getMaxBlinkRate()` methods).
     * @retval true: The display was not in **waiting mode** and the required timer could be attached. The **waiting mode** is started.
     * @retval false: The display was in **waiting mode**, no change was made.
     * @retval false: The parameter passed is out of range, the display wasn't set to **wait mode**
     * @retval false: The timer couldn't be attached, the display wasn't set to **wait mode**
     */
    bool wait(const unsigned long &newWaitRate);
    /**
     * @brief Prints one character to the display, at a defined port, without affecting the rest of the characters displayed.
     *
     * @param segments An integer value representing which segments to turn on and which off to get the graphic representation of a character in the seven segment display.
     * @param port An integer value representing the digit where the character will be set, being the range of valid values 0 <= port < dspDigits, the 0 value is the rightmost port, the 1 value the second from the right and so on.
     * @retval true: The parameters are within the acceptable range, in this case 0 <= port < dspDigits.
     * @retval false: The port value was outside the acceptable range.
     *
     * @note The corresponding value can be looked up in the **_charLeds[]** array definition in the header file of the library. In the case of a common cathode display the values there listed must be complemented. Any other uint8_t (char or unsigned short int is the same here) value is admissible, but the displayed result might not be easily recognized as a known ASCII character, but might be useful to display pre-defined special **pre-defined significance** characters.
     */
    bool write(const uint8_t &segments, const uint8_t &port);
    /**
     * @brief Prints one character to the display, at a defined port, without affecting the rest of the characters displayed.
     *
     * @param character A single character string that must be displayable, as defined in the `print()` method.
     * @param port An integer value representing the digit where the character will be set, being the range of valid values 0 <= port <= (dspDigits - 1), the 0 value is the rightmost digit, the 1 value the second from the right and so on.
     *
     * @retval true:**character** is a displayable one char string, and **port** value is in the range 0 <= value <= (dspDigits - 1).
     * @retval: false: The **character** was not "displayable" or the **port** value was out of range.
     *
     */
    bool write(const std::string &character, const uint8_t &port);
};

//============================================================> Class declarations separator

class ClickCounter{
private:
    SevenSegDisplays* _displayPtr;
    int _count{0};
    int _beginStartVal{0};
    bool _countRgthAlgn{true};
    bool _countZeroPad{false};
public:
    /**
     * @brief Class constructor
     *
     * @param newDisplay A pointer to a SevenSegDisplays object which will display the counter status of the instantiated object
     *
     * @class ClickCounter
     */
    ClickCounter(SevenSegDisplays* newDisplay);
    /**
     * @brief Class destructor
     */
    ~ClickCounter();
    /**
     * @brief See SevenSegDisplays::blink()
     */
    bool blink();
    /**
     * @brief See SevenSegDisplays::blink(const unsigned long, const unsigned long)
     */
    bool blink(const unsigned long &onRate, const unsigned long &offRate = 0);
    /**
     * @brief See SevenSegDisplays::clear()
     */
    void clear();
    /**
     * @brief Initiates the counter operation.
     *
     * The counter initiation involves several actions including:
     * - Beginning the seven segments display operation, including the underlying hardware display
     * - Checking the starting value is within the valid range
     * - Saving the starting value for future reference
     *
     * @param startVal (optional) The starting value setting for the counter, it must be in the range _displayPtr->getDspValMin() <= startVal <= _displayPtr->getDspValMax(). If no value is specified the default value, 0, will be assumed.
     *
     * @return The success initiating the counter
     * @retval true: The initiation succeeded.
     * @retval false: The initiation failed, either caused with a fail initiating the SevenSegDisplay object or caused by a startVal out of acceptable range.
     */
    bool countBegin(int32_t startVal = 0);
    /**
     * @brief Decrements the counter value.
     *
     * Decrements the value of the current count and refreshes the display to keep it updated. The counter is decremented independently of the sign of the current count, as long as the new value resulting is in the displayable range.
     *
     * @param qty (Optional) The integer whose **absolute value** is to decrement from the current count. If a parameter is not provided a value of 1 is assumed.
     *
     * @return The success in decrementing the parameter value from the counter
     * @retval true: The result of subtracting the **absolute value** of parameter from the counter was within the valid range, counter updated.
     * @retval false: The result of subtracting **absolute value** of the parameter from the counter was out of the valid range, counter was not updated, i.e. (count - abs(qty)) < _displayPtr->getDspValMin().
     *
     * @attention According to the present definition, 0 is a valid parameter. As such the execution of `.countDown(0)` will be successful, even if the count value won't change!!
     */
    bool countDown(int32_t qty = 1);
    /**
     * @brief Returns the counter count value to the value of the parameter used in the countBegin(int32_t) method
     *
     * @return The success in returning the count to it's original value.
     */
    bool countReset();
    /**
     * @brief Changes the current count value without stopping and restarting it.
     *
     * The count value will be set to the parameter value passed if it's within valid range.
     *
     * @param restartValue (Optional) New value for the counter count attribute. If a parameter is not provided a value of 1 is assumed.
     *
     * @return The success restarting the counter
     * @retval true: The restarting succeeded.
     * @retval false: The restarting failed, caused by a restartVal out of acceptable range.     */
    bool countRestart(int32_t restartVal = 0);
    bool countEnd();
    /**
     * @brief Modifies the value of the current count by the adding or subtracting the parameter value passed to make the count absolute value smaller until it reaches zero.
     *
     * Modifies the value of the current count. The counter absolute value will be decremented, independently of the sign of the current count, as long as the absolute value of the modification is less than or equal to the current value. If the current count is negative, the absolute value of the parameter will be added, and so result in a count value increment. If the current count is positive, the absolute value of the parameter will be subtracted, resulting in a count decrement, with the concrete purpose of approaching the count value to 0.
     *
     * @param qty  (Optional) The unsigned integer value to add or subtract from the current count. If a parameter is not provided a value of 1 is assumed.
     *
     * @return The success in subtracting the absolute value of the parameter from the absolute value of the counter.
     * @retval true: The result of the operation value from the counter was within the valid range, counter updated.
     * @retval false: The result of subtracting the absolute value of the parameter from the absolute value of the counter resulted in less than 0, counter was not updated.
     *
     * @attention According to the present definition, 0 is a valid parameter. As such the execution of `.countDown(0)` will be successful, even if the count value won't change!!
     */
    bool countToZero(int32_t qty = 1);
    /**
     * @brief Increments the counter value.
     *
     * Increments the value of the current count and refreshes the display to keep it updated. The counter is incremented independently of the sign of the current count, as long as the new value resulting is in the displayable range.
     *
     * @param qty (Optional) The integer whose **absolute value** is to be increment from the current count. If a parameter is not provided a value of 1 is assumed.
     *
     * @return The success in incrementing the counter by the the parameter's absolute value.
     * @retval true: The result of adding the **absolute value** of parameter to the counter was within the valid range, counter updated.
     * @retval false: The result of adding **absolute value** of the parameter to the counter was out of the valid range, counter was not updated, i.e. (count + abs(qty)) > _displayPtr->getDspValMax().
     *
     * @attention According to the present definition, 0 is a valid parameter. As such the execution of `.countUp(0)` will be successful, even if the count value won't change!!
     */
    bool countUp(int32_t qty = 1);
    /**
     * @brief Gets the current value of the counter.
     *
     * @return The current value held by the counter.
     */
    int32_t getCount();
    /**
     * @brief Gets the value used to start the counter.
     *
     * Gets the value passed as parameter when the countBegin(int32_t) method was invoked, starting the counter. This is the value that will be used for the countReset() method.
     *
     * @return The value to which the counter was originally started.
     */
    int32_t getStartVal();
    /**
     * @brief See SevenSegDisplays::noBlink()
     */
    bool noBlink();
    /**
     * @brief See SevenSegDisplays::setBlinkRate(const unsigned long, const unsigned long)
     */
    bool setBlinkRate(const unsigned long &newOnRate, const unsigned long &newOffRate = 0);
    bool updDisplay();  //To be analyzed it's current need
};

//============================================================> Class declarations separator



#endif /* _SEVENSEGDISPLAYS_STM32_H_ */
