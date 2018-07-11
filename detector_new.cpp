#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
#include "header.h"

#define desired_speed 0.5 //1 метр в секунду - максимальная скорость
#define desired_speed_pwm desired_speed/(0.026)

volatile uint8_t cmd = 0;
volatile bool left_ob = 1;
volatile bool right_ob = 1;
volatile uint8_t detector = 0;
volatile uint8_t cnt0 = 0;
volatile uint8_t cnt1 = 0;
volatile uint8_t cnt2 = 0;
volatile bool left_engine_run = false;
volatile bool right_engine_run = false;

volatile uint8_t speed_cnt_left = 0;
volatile uint8_t speed_cnt_right = 0;
volatile uint8_t timer_cnt = 0;

volatile uint8_t speed_left = 0;
volatile uint8_t speed_right = 0;

volatile int pwm_left = 0;
volatile int pwm_right = 0;

volatile bool remove_= 0;

volatile uint8_t desired_speed_left = (uint8_t) desired_speed_pwm;
volatile uint8_t desired_speed_right = (uint8_t) desired_speed_pwm;

volatile int E_left = 0;
volatile int E_right = 0;

void
configure_pins()
{
	DETECTOR_DIR |= (_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //0 - настройка ноги на запись в неё
	DETECTOR_DIR &= ~_BV(COM_DETECTOR_INPUT); //1 - настройка ноги на чтение

	DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //Выключаем ноги детекторов
	DETECTOR_PIN &= ~(_BV(COM_DETECTOR_INPUT)); //Выключаем ногу, которая отвечает за передачу данных с дальномера

	ENGINE_DIR |= _BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT);
	ENGINE_PORT &= ~(_BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT));
}

void
configure_timer()
{
	TCCR0 = _BV(CS02);
	TIMSK |= _BV(TOIE0);
}

ISR(INT0_vect)
{
	speed_cnt_left++;
}

ISR(INT1_vect)
{
	speed_cnt_right++;
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

//Счетчик, через который мы определяем настало ли время сбрасывать счетчик прерываний от внешнего воздействия
bool
counter_regulate_speed(uint8_t target)
{
		if (cnt1 > target)
		{
			cnt1 = 0;
			return true;
		}

		cnt1++;

		return false;
}

//Счетчики для ШИМа
bool
pwm_counter_left(uint8_t target, uint8_t max = 10)
{
	if (cnt1 > max) cnt1 = 0;
	return cnt1++ > target;
}

bool
pwm_counter_right(uint8_t target, uint8_t max = 10)
{
	if (cnt2 > max) cnt2 = 0;
	return cnt2++ > target;
}

//Главная функция, которая определяет, какой должна быть скорость
void
get_direction() {
	uint8_t c;

	switch(cmd) {
	case 0: //Включение питания левого датчика
		DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_LEFT);

		pwm_left = 10;
		pwm_right = 10;

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

		if(c > 7) left_ob = true;
		else  left_ob = false;

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

		if(c > 7) right_ob = true;
		else right_ob = false;

		cmd = 7;

		break;

	case 7: //Выключение правого датчика
		DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_RIGHT));
		cmd = 8;
		break;
	}
}

//Запускает ШИМ для двигателей
void
do_pwm_left()
{
	if (!pwm_counter_left(pwm_left)) ENGINE_PORT |= _BV(ENGINE_LEFT);
	else ENGINE_PORT &= ~_BV(ENGINE_LEFT);
}

void
do_pwm_right()
{
	if (!pwm_counter_right(pwm_right)) ENGINE_PORT |= _BV(ENGINE_RIGHT);
	else ENGINE_PORT &= ~_BV(ENGINE_RIGHT);
}

//Регулировка скорости колёс (надо дописать изменение желаемой скорости в зависимости от left_ob и right_ob)
void
regulate_left() {
	uint8_t spd_left = 0;
	uint8_t trg_spd_left = 10;

	spd_left = speed_cnt_left;
	speed_cnt_left = 0;
	pwm_left = _MIN(_MAX(trg_spd_left - spd_left,0),10);
}

void
regulate_right() {
	uint8_t spd_right = 0;
	uint8_t trg_spd_right = 10;

	spd_right = speed_cnt_right;
	speed_cnt_right = 0;
	pwm_right = _MIN(_MAX(trg_spd_right - spd_right,0),10);
}



//Прерывание от таймера
ISR(TIMER0_OVF_vect)
{
//	get_direction();

	if(counter_regulate_speed(121)) {
	regulate_right();
	regulate_left();
	}

	/*pwm_left = 0;
	pwm_right = 0;*/

	do_pwm_right();
	do_pwm_left();
}

int
main(void)
{
	cli();

	configure_pins();
	configure_timer();

	sei();

	while (1)
	{
		asm("nop");
	}
	return 0;
}
