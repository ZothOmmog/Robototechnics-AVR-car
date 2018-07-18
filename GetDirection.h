#ifndef GETDIRECTION_H_
#define GETDIRECTION_H_

#include <stdint.h>

void
get_direction();

void
configure_pins_detectors();

//===============Дальномеры=================
#define DETECTOR_DIR DDRB

#define DETECTOR_PORT PORTB
#define DETECTOR_PIN PINB

//Нога на которую подаётся питание (оба датчика подключены к 1 ноге)
#define COM_DETECTOR_INPUT PB2

//Ноги с которых считываем сигнал
#define COM_DETECTOR_OUTPUT_LEFT PB4
#define COM_DETECTOR_OUTPUT_RIGHT PB1
//==========================================

#define OBS_LEFT 0b01000000
#define OBS_RIGHT 0b10000000
#define OBS_FRONT 0b00000000
#define NOT_OBS 0b11000000

/*
#define desired_speed 0.2
#define desired_speed_pwm ((uint8_t)(desired_speed/0.104)) //Прерывания в 1/4 секунды
*/

extern volatile uint8_t desired_speed_left;
extern volatile uint8_t desired_speed_right;

#endif /* GETDIRECTION_H_ */
