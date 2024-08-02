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

const int MAX_DIGITS_PER_DISPLAY{16};
const uint8_t diyMore8Bits[8] {3, 2, 1, 0, 7, 6, 5, 4};
const uint8_t noName4Bits[4] {0, 1, 2, 3};

//--------------------------------------------------------------- User Function prototypes
void Error_Handler(void);
bool setGPIOPinAsInput(const gpioPinId_t &inPin);
bool setGPIOPinAsOutput(const gpioPinId_t &outPin);
void txTM163xTmrCB(TIM_HandleTypeDef *htim);
//--------------------------------------------------------------- User Static variables
uint8_t SevenSegDispHw::_dspHwSerialNum = 0;
uint8_t SevenSegTM163X::_usTmrUsrs = 0;

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

void SevenSegDynHC595::tmrCbRefreshDyn(TimerHandle_t rfrshTmrCbArg){
	SevenSegDynHC595* argObj = (SevenSegDynHC595*)pvTimerGetTimerID(rfrshTmrCbArg);
   //Timer Callback to keep the display lit by calling the display's refresh() method
	argObj -> refresh();

    return;
}

//============================================================> Class methods separator

SevenSegStatic::SevenSegStatic(gpioPinId_t* ioPins, uint8_t dspDigits, bool commAnode)
:SevenSegDispHw(ioPins, dspDigits, commAnode)
{
}

SevenSegStatic::~SevenSegStatic()
{
}

//============================================================> Class methods separator

SevenSegTM163X::SevenSegTM163X(gpioPinId_t* ioPins, uint8_t dspDigits)
:SevenSegStatic(ioPins, dspDigits, true)
{
	 _clk = ioPins[_clkArgPos];
	 _dio = ioPins[_dioArgPos];

	setGPIOPinAsOutput(_clk);	//Setting pin directions
	setGPIOPinAsOutput(_dio);
}

SevenSegTM163X::~SevenSegTM163X()
{
}

bool SevenSegTM163X::begin(TIM_HandleTypeDef &newTxTM163xTmr){
	bool result{false};
	/*
	 * As TM163X use a non standard I2C-like communications protocol, the begin method must do
	 * - Create a Timer interrupt to produce the CLK speed of the communications
	 * - Start the Timer int
	 * - Send a turn On command using the turnOn() method
	 * - Clear the display (test if needed)
	 * - Stop the interrupt Timer
	 */

	//Timer parameters configuration
	//---------------------------------------------------------------------
	TIM_OC_InitTypeDef sConfigOC = {0};

	_txTM163xTmr.Instance = TIM11;	//Adress of the TIMER11, must be variable for different timers use
	_txTM163xTmr.Init.Prescaler = 84-1;	// Prescaled to 1MHz, must be variable to generate that clockspeed for any MCU
	_txTM163xTmr.Init.CounterMode = TIM_COUNTERMODE_UP;
	_txTM163xTmr.Init.Period = 10-1;
	_txTM163xTmr.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	_txTM163xTmr.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&_txTM163xTmr) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_TIM_OC_Init(&_txTM163xTmr) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_OC_ConfigChannel(&_txTM163xTmr, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	//---------------------------------------------------------------------
	  HAL_TIM_RegisterCallback(&_txTM163xTmr, HAL_TIM_PERIOD_ELAPSED_CB_ID, txTM163xTmrCB);	//Associates the callback with the interrupt source
	/*HAL_StatusTypeDef HAL_TIM_RegisterCallback(TIM_HandleTypeDef *htim, HAL_TIM_CallbackIDTypeDef CallbackID, pTIM_CallbackTypeDef pCallback);
	 *                                                            -------                           ----------                       ---------
	 *                                                               |                                  |                                 |
	 *                                                               |                                  |                                 --> Pointer to the TIM callback function, the declaration of the function must respect the function pointer as follows: typedef void (*pTIM_CallbackTypeDef)(TIM_HandleTypeDef *htim)
	 *                                                               |                                  ------------------------------------> Enum defining the Interrupt trigger event for this timer: HAL_TIM_OC_DELAY_ELAPSED_CB_ID = 0x14U // TIM Output Compare Delay Elapsed Callback ID
	 *                                                               -----------------------------------------------------------------------> TIM_HandleTypeDef reference to the timer
	*/
  //---------------------------------------------------------------------
	  HAL_TIM_Base_Start_IT(&_txTM163xTmr);



	return result;
}

void SevenSegTM163X::delay10UsTck(const uint32_t &actDelay){
	uint32_t tckCount{0};
	timIntFlg = RESET;	// The handmade CLK signal starts low to give the chance to use a "Rising Edge" mechanism

	do{
	  if(timIntFlg == SET){
		  ++tckCount;
		  timIntFlg = RESET;
	  }
	}while (tckCount <= actDelay);

	return;
}

void SevenSegTM163X::dspBffrCntntChng(){
	/* If it's low cost confirm the new buffer contents are different from the display content
	 * Create a message buffer according to the TM1637 I2C modified protocol:
	 * Invoke the send() method to output the message to the display
	 * Delete the message buffer
	 * >> SOT commands + buffer contents + EOT command
	 * SOT Commands: Command1 + Command2
	 * >> - Command1: Mode setup
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- - - ---
	 *   |   |  | |  |
	 *   |   |  | |  Data Write to display: 00
	 *   |   |  | Address auto-increment:  0
	 *   |   |  Normal/Test mode:         0
	 *   |   N/C:                       00
	 *   Data command setting:        01
	 *                              0b01000000
	 *
	 * >> - Command2: Address command setting, for TM1637 and TM1639 is 0xC0, 6 consecutive addresses for TM1637, 16 for TM1639
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- -------
	 *   |   |     |
	 *   |   |     C0H:  0000
	 *   |   N/C:      00
	 *   Add. comm.: 11
	 *             0b11000000
	 *
	 * >> Buffer contents: 6 ~ 16 bytes data sequence
	 *
	 * >> EOT commands: Command3:
	 * Command3: Display control
	 * -----------------
	 * |7|6|5|4|3|2|1|0|
	 *  --- --- - -----
	 *   |   |  |   |
	 *   |   |  |   Brightness control:    000~111
	 *   |   |  Display switch On/Off:    1/0
	 *   |   N/C:                       00
	 *   Display Control:             10
	 *                              0b1000XXXX
	 */

	return;
}

void SevenSegTM163X::_txStart(){
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_SET);
	HAL_GPIO_WritePin(_dio.portId, _dio.pinNum, GPIO_PIN_SET);
	delay10UsTck(2);
	HAL_GPIO_WritePin(_dio.portId, _dio.pinNum, GPIO_PIN_RESET);


	return;
}
void SevenSegTM163X::_txAsk(){
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_RESET);
	delay10UsTck(5);
	while (HAL_GPIO_ReadPin(_dio.portId, _dio.pinNum)){
	}
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_SET);
	delay10UsTck(2);
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_RESET);

	return;
}
void SevenSegTM163X::_txStop(){
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_RESET);
	delay10UsTck(2);
	HAL_GPIO_WritePin(_dio.portId, _dio.pinNum, GPIO_PIN_RESET);
	delay10UsTck(2);
	HAL_GPIO_WritePin(_clk.portId, _clk.pinNum, GPIO_PIN_SET);
	delay10UsTck(2);
	HAL_GPIO_WritePin(_dio.portId, _dio.pinNum, GPIO_PIN_SET);

	return;
}
void SevenSegTM163X::_txWrByte(const uint8_t data){

	return;
}

void SevenSegTM163X::send(const uint8_t* data, const uint8_t dataQty){

	_txStart();
	for(int i{0}; i < dataQty; i++){
		_txWrByte(*(data+i));
		_txAsk();
	}
	_txStop();

	return;
}

//============================================================> Generic use functions

bool setGPIOPinAsOutput(const gpioPinId_t &outPin){
	  HAL_GPIO_WritePin(outPin.portId, outPin.pinNum, GPIO_PIN_RESET);
	  GPIO_InitTypeDef pinInit = {
			  .Pin = outPin.pinNum,
			  .Mode = GPIO_MODE_OUTPUT_PP,
			  .Pull = GPIO_NOPULL,
			  .Speed = GPIO_SPEED_FREQ_LOW
	  };
	  HAL_GPIO_Init(outPin.portId, &pinInit);

	  return true;
}

bool setGPIOPinAsInput(const gpioPinId_t &inPin){

	  HAL_GPIO_WritePin(inPin.portId, inPin.pinNum, GPIO_PIN_RESET);

	  GPIO_InitTypeDef pinInit = {
			  .Pin = inPin.pinNum,
			  .Mode = GPIO_MODE_INPUT,
			  .Pull = GPIO_NOPULL,
	  };
	  HAL_GPIO_Init(inPin.portId, &pinInit);

	  return true;
}

bool setUsTmrInt(){	//Set a timer interrupt of 10 Us for timebase for synchronous communications CLK
	/*
	 * For a 84 MHz MCU SYSCLK:
	 * Prescaler of 84 sets the output speed to 1000KHz (1MHz)
	 * Count of 10 sets the output speed to 100KHz
	 * Any timer for this MCU is capable of those frequencies, the TIM10 is chosen
	 */
	bool result{false};

	return result;
}

void delay10UsTck(const uint32_t &actDelay){
	uint32_t tckCount{0};
	timIntFlg = RESET;	// The "hand-made" CLK signal starts low to give the chance to use a "Rising Edge" mechanism

	do{
	  if(timIntFlg == SET){
		  ++tckCount;
		  timIntFlg = RESET;
	  }
	}while (tckCount <= actDelay);

	return;
}

void txTM163xTmrCB(TIM_HandleTypeDef *htim)
{
	timIntFlg = SET;

	return;
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
