#include <avr/interrupt.h>
#include <stdint.h>

#include "GetDirection.h"
#include "Pwm.h"

//макросы, которые используются в ПИД-регуляторе для преобразования входной скорости в значения, которые использует ШИМ
#define _MIN(x,y) ((x < y) ? x : y)
#define _MAX(x,y) ((x > y) ? x : y)

volatile uint8_t speed_cnt_left = 0; //счетчик прерываний от левого энкодера
volatile uint8_t speed_cnt_right = 0; // счетчик прерываний от правого энкодера

volatile uint8_t spd_left; //кол-во прерываний за время dt от левого энкодера
volatile uint8_t spd_right; // кол-во прерываний за время dt от правого энкодера

volatile int e; //пропорциональная составляющая ошибки
volatile int Ie_l = 0; //интегральная составляющая ошибки
volatile int Ie_r = 0; //интегральная составляющая ошибки
volatile float de; //дифференциальная составляющая ошибки

volatile float Kp = 0.8; //коэффицент пропорциональности
volatile float Ki = 0.08; //коэффицент интегрирования
volatile float Kd =  0.08; //коэффицент дифференцирования
volatile int dt = 30; //1 секунда = 122 прерывания от таймера
volatile int e0_l = 0; //общее время работы контроллера
volatile int e0_r = 0; //общее время работы контроллера

volatile int pwm_left = 0; //Переменная, которая передаётся в ШИМ-регулятор скорости для левого колеса
volatile int pwm_right = 0; // Переменная, которая передаётся в ШИМ-регулятор скорости для правого колеса

volatile uint8_t cnt_l = 0; //счетчик для времени регулирования (левое колесо)
volatile uint8_t cnt_r = 0; //счетчик для времени регулирования (правое колесо)

//Из файла "CalculateWishSpeed.h"
//volatile uint8_t desired_speed_left = 0;
//volatile uint8_t desired_speed_right = 0;

//прерывание от левого энкодера
ISR(INT0_vect)
{
	speed_cnt_left++;
}

//прерывание от правого энкодера
ISR(INT1_vect)
{
	speed_cnt_right++;
}

//счетчик времени регилировки
bool
regulate_left_counter(uint8_t target)
{
	if(++cnt_l > target)
	{
		cnt_l = 0;
		return true;
	}
	else return false;
}

bool
regulate_right_counter(uint8_t target)
{
	if(++cnt_r > target)
	{
		cnt_r = 0;
		return true;
	}
	else return false;
}

//Регулировка скорости левого колеса
void
regulate_left()
{
	if(regulate_left_counter(dt))
	{
		if(desired_speed_left == 0) pwm_left = 0;
		else
		{
			spd_left = speed_cnt_left; //Кол-во прерываний за время dt
			speed_cnt_left = 0;

			e = desired_speed_left - spd_left; //Пропорциональная составляющая ошибка

			Ie_l += e; //Интегральная составляющая ошибки

			de = e - e0_l; //Дифференциальная составляющая ошибка
			e0_l = e;

			pwm_left = _MIN(_MAX(Kp * e + Ki * Ie_l + Kd * de, 0), 20); //Величина-посредник для регулирования скорости (ШИМ)
			//pwm_left = 5;
		}
	}
}

//регулировка скорости правого колеса
void
regulate_right()
{
	if(regulate_right_counter(dt))
	{
		if(desired_speed_right == 0) pwm_right = 0;
		else
		{
			spd_right = speed_cnt_right;
			speed_cnt_right = 0;

			e = desired_speed_right - spd_right;

			Ie_r += e;

			de = e - e0_r; //Дифференциальная составляющая ошибка
			e0_r = e; //

			pwm_right = _MIN(_MAX(Kp * e + Ki * Ie_r + Kd * de, 0), 20);
			//pwm_right = 5;
		}
	}
}
