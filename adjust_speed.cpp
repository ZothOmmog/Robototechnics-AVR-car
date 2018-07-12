#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
#include "CalculateWishSpeed.h"

//Макросы, которые используются в ПИД-регуляторе для преобразования входной скорости в значения, которые использует ШИМ
#define _MIN(x,y) ((x < y) ? x : y)
#define _MAX(x,y) ((x > y) ? x : y)

volatile uint8_t speed_cnt_left = 0;
volatile uint8_t speed_cnt_right = 0;

volatile int pwm_left = 0;
volatile int pwm_right = 0;

//Из файла "CalculateWishSpeed.h"
//volatile uint8_t desired_speed_left = 0;
//volatile uint8_t desired_speed_right = 0;

ISR(INT0_vect)
{
	speed_cnt_left++;
}

ISR(INT1_vect)
{
	speed_cnt_right++;
}

//Регулировка скорости колёс (надо дописать изменение желаемой скорости в зависимости от left_ob и right_ob)
void
regulate_left()
{
	uint8_t spd_left = 0; //Фактическая скорость
	spd_left = speed_cnt_left;
	speed_cnt_left = 0;
	const uint8_t Kp = 1;
	uint8_t error = _MIN(_MAX(desired_speed_left - spd_left,0),20); //Обчисление пропорциональной составляющей ошибки

	pwm_left = Kp * error; //Величина-посредник для изменения скорости (ШИМ)
}

void
regulate_right()
{
	uint8_t spd_right = 0; //Фактическая скорость
	spd_right = speed_cnt_right;
	speed_cnt_right = 0;
	const uint8_t Kp = 1;
	uint8_t error = _MIN(_MAX(desired_speed_right - spd_right,0),20); //Обчисление пропорциональной составляющей ошибки

	pwm_right = Kp * error; //Величина-посредник для изменения скорости (ШИМ)
}
