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

SevenSegDynamic::SevenSegDynamic(){}

SevenSegDynamic::~SevenSegDynamic()
{
   if(_dspRfrshTmrHndl){   //if the timer still exists and is running, stop and delete
   	end();
   }
}

bool SevenSegDynamic::begin(){
	bool result {false};
   BaseType_t tmrModResult {pdFAIL};

	//Verify if the timer service was attached by checking if the Timer Handle is valid (also verify the timer was started)
	if (!_svnSgDynTmrHndl){
		//Create a valid unique Name for identifying the timer created for this Dynamic Display
		std::string _svnSgDynSerNumStr{ "00" + std::to_string((int)_dspHwInstNbr) };
		_svnSgDynSerNumStr = _svnSgDynSerNumStr.substr(_svnSgDynSerNumStr.length() - 2, 2);
		std::string _svnSgDynRfrshTmrName {"DynDsp"};
		_svnSgDynRfrshTmrName = _svnSgDynRfrshTmrName + _svnSgDynSerNumStr + "rfrsh_tmr";
		//Initialize the Display refresh timer. Considering each digit to be refreshed at 30 Hz in turn, the freq might be (Max qty of digits * 30Hz)
		_dspRfrshTmrHndl = xTimerCreate(
							_svnSgDynRfrshTmrName.c_str(),
							pdMS_TO_TICKS((int)(1000/(30 * _dspDigitsQty))),
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
	SevenSegDispHw* argObj = (SevenSegDispHw*)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling each display's fastRefresh() method

   /*    for(uint8_t i {0}; i < _dspPtrArrLngth; i++){
        // if (*(_instancesLstPtr + i) != nullptr)
            // (*(_instancesLstPtr + i)) -> fastRefresh();
    }
*/
	argObj -> refresh();

    return;
}

//============================================================> Class methods separator

SevenSegDynHC595::SevenSegDynHC595(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode)
:_sclk{ioPins[_sclkArgPos]}, _rclk{ioPins[_rclkArgPos]}, _dio{ioPins[_dioArgPos]}
{
	 _commAnode = commAnode;
	//Set the declared GPIO pins
	 setGPIOPinAsOutput(_sclk);
	 setGPIOPinAsOutput(_rclk);
	 setGPIOPinAsOutput(_dio);

    begin();
}

SevenSegDynHC595::~SevenSegDynHC595(){}

void SevenSegDynHC595::refresh(){
//   bool tmpLogic {true};
   uint8_t tmpDigToSend{0};

    for (int i {0}; i < _dspDigitsQty; i++){
        tmpDigToSend = *(_dspBuffPtr + ((i + _firstRefreshed) % _dspDigitsQty));
        send(tmpDigToSend, uint8_t(1) << *(_digitPosPtr + ((i + _firstRefreshed) % _dspDigitsQty)));
    }
    ++_firstRefreshed;
    if (_firstRefreshed == _dspDigitsQty)
        _firstRefreshed = 0;

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
	  HAL_GPIO_Init(outPin.portId, &GPIO_InitStruct);

	  return true;
}
