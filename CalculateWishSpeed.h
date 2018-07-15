#ifndef CALCULATEWISHSPEED_H_
#define CALCULATEWISHSPEED_H_

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

#define desired_speed 2
#define desired_speed_pwm desired_speed/(0.026)

extern volatile uint8_t desired_speed_left;
extern volatile uint8_t desired_speed_right;

#endif /* CALCULATEWISHSPEED_H_ */
