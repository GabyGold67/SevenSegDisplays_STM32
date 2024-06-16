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

//--------------------------------------------------------------- User Function prototypes
bool setGPIOPinAsOutput(const gpioPinId_t &outPin);
//--------------------------------------------------------------- User Static variables
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
//============================================================> Class methods separator

SevenSegDispHw::SevenSegDispHw()
{

}

SevenSegDispHw::SevenSegDispHw(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode)
:_ioPins{ioPins}, _digitPosPtr{new uint8_t[dspDigits]}, _dspDigitsQty {dspDigits}, _commAnode {commAnode}
{
    _dspHwInstNbr = _dspHwSerialNum++;
    for (uint8_t i{0}; i < _dspDigitsQty; i++){
        *(_digitPosPtr + i) = i;
    }
}

SevenSegDispHw::~SevenSegDispHw() {
    delete [] _digitPosPtr;
}

bool SevenSegDispHw::begin(const unsigned long int &rfrshFrq){

	return true;
}

bool SevenSegDispHw::end(){

	return true;
}

bool SevenSegDispHw::getCommAnode(){

    return _commAnode;
}

uint8_t* SevenSegDispHw::getDspBuffPtr(){

    return _dspBuffPtr;
}

uint8_t SevenSegDispHw::getDspDigits(){

    return _dspDigitsQty;
}

bool SevenSegDispHw::setDigitsOrder(uint8_t* newOrderPtr){
    bool result{true};

    for(int i {0}; i < _dspDigitsQty; i++){
        if (*(newOrderPtr + i) >= _dspDigitsQty){
            result = false;
            break;
        }
    }
    if (result){
        for(int i {0}; i < _dspDigitsQty; i++){
            *(_digitPosPtr + i) = *(newOrderPtr + i);
        }
    }

    return result;
}

void SevenSegDispHw::setDspBuffPtr(uint8_t* newDspBuffPtr){
    _dspBuffPtr = newDspBuffPtr;

    return;
}

//============================================================> Class methods separator

SevenSegDynamic::SevenSegDynamic()
{
}

SevenSegDynamic::SevenSegDynamic(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{
}

SevenSegDynamic::~SevenSegDynamic()
{
   if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
   	end();
   }
}

bool SevenSegDynamic::begin(const unsigned long int &rfrshFrq){
	bool result {false};
   BaseType_t tmrModResult {pdFAIL};
   TickType_t tmrRfrshFrqInTcks{0};

   SevenSegDispHw::begin();

   //Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
	if (!_svnSgDynTmrHndl){
		if (rfrshFrq)	//Calculate the Timer Period
			tmrRfrshFrqInTcks = pdMS_TO_TICKS(rfrshFrq);
		else
			tmrRfrshFrqInTcks = pdMS_TO_TICKS(static_cast<int>(1000/(30 * _dspDigitsQty)));
		//Create a valid unique Name for identifying the timer created for this Dynamic Display
		std::string _svnSgDynSerNumStr{ "00" + std::to_string((int)_dspHwInstNbr) };
		_svnSgDynSerNumStr = _svnSgDynSerNumStr.substr(_svnSgDynSerNumStr.length() - 2, 2);
		std::string _svnSgDynRfrshTmrName {"DynDsp"};
		_svnSgDynRfrshTmrName = _svnSgDynRfrshTmrName + _svnSgDynSerNumStr + "rfrsh_tmr";
		//Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
		_dspRfrshTmrHndl = xTimerCreate(
							_svnSgDynRfrshTmrName.c_str(),
							tmrRfrshFrqInTcks,	//Timer period
							pdTRUE,  //Autoreload
							this,   //TimerID, data to be passed to the callback function
							tmrCbRefreshDyn  //Callback function
		);
		if((_dspRfrshTmrHndl != NULL) && (!xTimerIsTimerActive(_dspRfrshTmrHndl))){
			tmrModResult = xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);
			if (tmrModResult == pdPASS)
				result = true;
		}
	}

	return result;
}

bool SevenSegDynamic::end() {
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
   	 tmrModResult = xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
   	 if(tmrModResult == pdPASS){
   		 tmrModResult = xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
      	 if(tmrModResult == pdPASS){
				 _dspRfrshTmrHndl = NULL;
				 result = true;
      	 }
   	 }
    }

    return result;
}

void SevenSegDynamic::tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg){
	SevenSegDynamic* argObj = (SevenSegDynamic*)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling the display's refresh() method
	argObj -> refresh();

    return;
}

//============================================================> Class methods separator

SevenSegDynHC595::SevenSegDynHC595(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDynamic(ioPins, dspDigits, commAnode), _sclk{ioPins[_sclkArgPos]}, _rclk{ioPins[_rclkArgPos]}, _dio{ioPins[_dioArgPos]}
{
	//Set the declared GPIO pins
	 setGPIOPinAsOutput(_sclk);
	 setGPIOPinAsOutput(_rclk);
	 setGPIOPinAsOutput(_dio);

	 HAL_GPIO_WritePin(_rclk.portId, _rclk.pinNum, GPIO_PIN_SET); // _rclk (data latched) will be lowered to let data in
	 HAL_GPIO_WritePin(_sclk.portId, _sclk.pinNum, GPIO_PIN_RESET);	//The _sclk  must start lowered (as data will be taken at rise flank)
}

SevenSegDynHC595::~SevenSegDynHC595(){}

void SevenSegDynHC595::refresh(){
   uint8_t tmpDigToSend{0};
   uint8_t tmpPosToSend{0};

    for (int i {0}; i < _dspDigitsQty; i++){
        tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
        tmpPosToSend = uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty));
//        send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
        send(tmpDigToSend, tmpPosToSend);
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

    return;
}

void SevenSegDynHC595::send(uint8_t content){
	bool prevPnLvl {false};

	HAL_GPIO_WritePin(_dio.portId, _dio.pinNum, GPIO_PIN_RESET);	//Ensuring starting state of the data out pin

	for (int i {7}; i >= 0; i--){   //Send each of the 8 bits representing the character
		if (((content & 0x80)!=0) xor prevPnLvl){
			HAL_GPIO_TogglePin(_dio.portId, _dio.pinNum);
			prevPnLvl = !prevPnLvl;
		}
		HAL_GPIO_WritePin(_sclk.portId, _sclk.pinNum, GPIO_PIN_SET);	//Rising edge to accept data presented
		content <<= 1;
		HAL_GPIO_WritePin(_sclk.portId, _sclk.pinNum, GPIO_PIN_RESET); //Lower back for next bit to be presented
	}

	return;
}

void SevenSegDynHC595::send(const uint8_t &segments, const uint8_t &port){

	HAL_GPIO_WritePin(_rclk.portId, _rclk.pinNum, GPIO_PIN_RESET);	//Set the shift register to accept data
	send(segments);
	send(port);
	HAL_GPIO_WritePin(_rclk.portId, _rclk.pinNum, GPIO_PIN_SET);	//Set the shift register to show latched data

   return;
}

bool SevenSegDynHC595::begin(const unsigned long int &rfrshFrq){
	bool result {false};
   BaseType_t tmrModResult {pdFAIL};
   TickType_t tmrRfrshFrqInTcks{0};

	//Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
	if (!_svnSgDynTmrHndl){
		if (rfrshFrq)	//Calculate the Timer Period
			tmrRfrshFrqInTcks = pdMS_TO_TICKS(rfrshFrq);
		else
			tmrRfrshFrqInTcks = pdMS_TO_TICKS(static_cast<int>(1000/(30 * _dspDigitsQty)));
		//Create a valid unique Name for identifying the timer created for this Dynamic Display
		std::string _svnSgDynSerNumStr{ "00" + std::to_string((int)_dspHwInstNbr) };
		_svnSgDynSerNumStr = _svnSgDynSerNumStr.substr(_svnSgDynSerNumStr.length() - 2, 2);
		std::string _svnSgDynRfrshTmrName {"DynDsp"};
		_svnSgDynRfrshTmrName = _svnSgDynRfrshTmrName + _svnSgDynSerNumStr + "rfrsh_tmr";
		//Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
		_dspRfrshTmrHndl = xTimerCreate(
							_svnSgDynRfrshTmrName.c_str(),
							tmrRfrshFrqInTcks,	//Timer period
							pdTRUE,  //Autoreload
							this,   //TimerID, data to be passed to the callback function
							tmrCbRefreshDyn  //Callback function
		);
		if((_dspRfrshTmrHndl != NULL) && (!xTimerIsTimerActive(_dspRfrshTmrHndl))){
			tmrModResult = xTimerStart(_dspRfrshTmrHndl, portMAX_DELAY);
			if (tmrModResult == pdPASS)
				result = true;
		}
	}

	return result;
}

bool SevenSegDynHC595::end() {
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
   	 tmrModResult = xTimerStop(_dspRfrshTmrHndl, portMAX_DELAY);
   	 if(tmrModResult == pdPASS){
   		 tmrModResult = xTimerDelete(_dspRfrshTmrHndl, portMAX_DELAY);
      	 if(tmrModResult == pdPASS){
				 _dspRfrshTmrHndl = NULL;
				 result = true;
      	 }
   	 }
    }

    return result;
}

void SevenSegDynHC595::tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg){
	SevenSegDynHC595* argObj = (SevenSegDynHC595*)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling the display's refresh() method
	argObj -> refresh();

    return;
}

//============================================================> Generic use functions

bool setGPIOPinAsOutput(const gpioPinId_t &outPin){

	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  HAL_GPIO_WritePin(outPin.portId, outPin.pinNum, GPIO_PIN_RESET);

	  GPIO_InitStruct.Pin = outPin.pinNum;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
//	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  HAL_GPIO_Init(outPin.portId, &GPIO_InitStruct);

	  return true;
}
