#include "CalculateWishSpeed.h"

#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>


#define desired_speed 0.5 //1 метр в секунду - максимальная скорость
#define desired_speed_pwm desired_speed/(0.026)//

volatile uint8_t cmd = 0;//
volatile bool left_ob = 1;//
volatile bool right_ob = 1;//
volatile uint8_t cnt0 = 0;//

volatile uint8_t desired_speed_left = (uint8_t) desired_speed_pwm;//
volatile uint8_t desired_speed_right = (uint8_t) desired_speed_pwm;//

volatile uint8_t cmd_desired_speed = 0b00000000;

void
configure_pins_detectors()
{
	DETECTOR_DIR |= (_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //0 - настройка ноги на запись в неё
	DETECTOR_DIR &= ~_BV(COM_DETECTOR_INPUT); //1 - настройка ноги на чтение

	DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //Выключаем ноги детекторов
	DETECTOR_PIN &= ~(_BV(COM_DETECTOR_INPUT)); //Выключаем ногу, которая отвечает за передачу данных с дальномера
}

bool
delay_counter(uint8_t target)
{
		if (cnt0 > target)
		{
			cnt0 = 0;
			return true;
		}

		cnt0++;

		return false;
}

//Главная функция, которая определяет, какой должна быть скорость
void
get_direction() {
	uint8_t c;

	switch(cmd) {
	case 0: //Включение питания левого датчика
		DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_LEFT);

		cmd = 1;
		break;

	case 1: //Ожидание
		if (delay_counter(8)) cmd = 2;
		break;

	case 2: //Считывание данных из левого датчика
		c = 0;

		for (int i = 0; i < 10; i++) {
			if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
		}

		if(c > 7) cmd_desired_speed |= _BV(7);

		cmd = 3;
		break;

	case 3: //Выключение левого датчика
		DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT));
		cmd = 4;
		break;

	case 4: //Включение правого датчика
		DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_RIGHT);
		cmd = 5;
		break;

	case 5: //Ожидание
		if(delay_counter(8)) cmd = 6;
		break;

	case 6: //Считывание данных из правого датчика
		c = 0;
		for (int i = 0; i < 10; ++i) {
			if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
		}

		if(c > 7) cmd_desired_speed |= _BV(6);

		cmd = 7;

		break;

	case 7: //Выключение правого датчика
		DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_RIGHT));
		cmd = 8;
		break;

	case 8:
		switch(cmd_desired_speed)
		{
		case 0b00000000:
			desired_speed_left = desired_speed_pwm;
			desired_speed_right = desired_speed_pwm;
			break;

		case 0b10000000:
			desired_speed_left = desired_speed_pwm;
			desired_speed_right = 0;
			break;

		case 0b01000000:
			desired_speed_left = 0;
			desired_speed_right = desired_speed_pwm;
			break;

		case 0b11000000:
			desired_speed_left = 0;
			desired_speed_right = 0;
			break;
		}

		cmd_desired_speed = 0b00000000;
		cmd = 0;
		break;
	}
}
