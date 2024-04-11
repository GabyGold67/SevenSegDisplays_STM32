/**
 * @file		: SevenSegDispHw.cpp
 * @brief	: Source file for SevenSegDispHw library classes methods
 *
 * @author	: Gabriel D. Goldman
 * @date		: Created on: Nov 16, 2023
 */
#include "sevenSegDispHw.h"

/*Prototype for a SevenSegDispHw classes and SUBClasses timer callback function
static void  tmrStaticCbBlink(TimerHandle_t blinkTmrCbArg){
    SevenSegDispHw* SevenSegUndrlHw = (SevenSegDispHw*) blinkTmrCbArg;


    return;
}*/

const int MAX_DIGITS_PER_DISPLAY{8};
const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t noName4Bits[4] {0, 1, 2, 3};

//---------------------------------------------------------------
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
//============================================================> Class methods separator

SevenSegDispHw::SevenSegDispHw() {}




