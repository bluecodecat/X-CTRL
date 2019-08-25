#ifndef __FILEGROUP_H
#define	__FILEGROUP_H

//*************** STM32 Core *****************//
#include "delay.h"
#include "pwm.h"
#include "adc.h"
#include "gpio.h"
#include "exti.h"
#include "timer.h"

//************* Arduino API **************//
#include "Arduino.h"
#include "WMath.h"
#include "Tone.h"

//************* User Libraries ************//
#include "ButtonEvent.h"
#include "EncoderEvent.h"
#include "MillisTaskManager.h"
#include "RC_Protocol.h"
#include "CommonMacro.h"
#include "Config.h"

namespace EEPROM_Chs {
typedef enum {
    ReadData,
    SaveData,
    ClearData
} EEPROM_Chs_t;
}

//************* Object & Struct & Value**************///*Object*/
extern ButtonEvent btEcd;
extern EncoderEvent ecd;

/*Struct*/
extern Joystick_t JS_L;
extern Joystick_t JS_R;

/*Value*/
extern bool State_BuzzSound;
extern float BattUsage;
extern float BattVoltage;

//****************** Functions********************//
void BuzzMusic(uint8_t music);
void BuzzTone(uint32_t freq, uint32_t time);
void BuzzTone(uint32_t freq);
void ButtonEventMonitor();

void EncoderMonitor();
bool EEPROM_Handle(EEPROM_Chs::EEPROM_Chs_t chs);
bool EEPROM_Register(void *pdata, uint16_t size);

void Init_X_CTRL();
bool Init_Value();
void Init_Buzz();
void Init_BottonEvent();
void Init_Display();
void Init_EncoderEvent();
void Init_GUI();
bool Init_NRF();
bool Init_SD();

void Thread_GUI();
void Task_SensorUpdate();
void Task_TransferData();
void Task_MusicPlayerRunning();

#endif