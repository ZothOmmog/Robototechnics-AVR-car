#include <avr/interrupt.h>
#include <stdint.h>

#include "CalculateWishSpeed.h"

//Макросы, которые используются в ПИД-регуляторе для преобразования входной скорости в значения, которые использует ШИМ
#define _MIN(x,y) ((x < y) ? x : y)
#define _MAX(x,y) ((x > y) ? x : y)

volatile uint8_t speed_cnt_left = 0; //счетчик прерываний от левого энкодера
volatile uint8_t speed_cnt_right = 0; // счетчик прерываний от правого энкодера

volatile uint8_t spd_left; //кол-во прерываний за время dt от левого энкодера
volatile uint8_t spd_right; // кол-во прерываний за время dt от правого энкодера

volatile float e; //пропорциональная составляющая ошибки
volatile float Ie = 0; //интегральная составляющая ошибки
volatile float de; //дифференциальная составляющая ошибки
volatile float Kp = 7; //коэффицент пропорциональности
volatile float Ki = 0.1; //коэффицент интегрирования
volatile float Kd = 30; //коэффицент дифференцирования
volatile float dt = 30; //1 секунда = 122 прерывания от таймера
volatile int T = 0; //общее время работы контроллера

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
	if(++cnt_l > target - 1)
	{
		cnt_l = 0;
		return true;
	}
	else return false;
}

bool
regulate_right_counter(uint8_t target)
{
	if(cnt_r++ > target - 1)
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
		spd_left = speed_cnt_left; //Кол-во прерываний за время dt
		speed_cnt_left = 0;

		e = desired_speed_left - spd_left; //Пропорциональная составляющая ошибка

		Ie += e; //Интегральная составляющая ошибки

		T += dt; //Общее время езды
		de = e / T; //Дифференциальная составляющая ошибка

		pwm_right = _MIN(_MAX(Kp * e + Ki * Ie + Kd * de, 0), 20); //Величина-посредник для регулирования скорости (ШИМ)
	}
}

//регулировка скорости правого колеса
void
regulate_right()
{
	if(regulate_right_counter(dt))
	{
		spd_right = speed_cnt_right;
		speed_cnt_right = 0;

		e = desired_speed_right - spd_right;

		Ie += e;

		T += 0.25;
		de = e / T;

		pwm_right = _MIN(_MAX(Kp * e + Ki * Ie + Kd * de, 0), 20);
	}
}
