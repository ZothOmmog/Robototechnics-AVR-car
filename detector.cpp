#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>

//TODO: Снизить скорость, ЗАСТАВИТЬ ВСЕ ЭТО РАБОТАТЬ!

#define DETECTOR_DIR DDRB

#define DETECTOR_PORT PORTB

#define DETECTOR_PIN PINB

#define COM_DETECTOR_INPUT PB2 //ЧТЕНИЕ

//ЗАПИСЬ
#define COM_DETECTOR_OUTPUT_LEFT PB4
#define COM_DETECTOR_OUTPUT_RIGHT PB1

#define ENGINE_LEFT PD7
#define ENGINE_RIGHT PD5

#define ENGINE_DIR DDRD

#define ENGINE_PORT PORTD

#define LEFT ENGINE_PORT |= _BV(ENGINE_LEFT)
#define RIGHT ENGINE_PORT |= _BV(ENGINE_RIGHT)

#define STOP_LEFT ENGINE_PORT &= ~_BV(ENGINE_LEFT)
#define STOP_RIGHT ENGINE_PORT &= ~_BV(ENGINE_RIGHT)

#define INPUT_PIN PINA

#define desired_speed 0.5 //1 метр в секунду - максимальная скорость
#define desired_speed_pwm desired_speed/(0.026)
#define k 0.013
#define p 61

#define _MIN(x,y) ((x < y) ? x : y)
#define _MAX(x,y) ((x > y) ? x : y)

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
	DETECTOR_DIR |= (_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //0 - НАСТРОЙКА ПИНА НА ЗАПИСЬ
	DETECTOR_DIR &= ~_BV(COM_DETECTOR_INPUT); //1 - НАСТРОЙКА ПИНА НА ЧТЕНИЕ

	DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT) | _BV(COM_DETECTOR_OUTPUT_RIGHT)); //ЗАПИСЫВАЕМ В ПОРТ НУЛИ
	//DETECTOR_PIN &= ~(_BV(COM_DETECTOR_INPUT)); //ЗАПИЫВАЕМ В ПИН НОЛЬ
	ENGINE_DIR |= _BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT);
	ENGINE_PORT &= ~(_BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT));
}

void
configure_timer()
{
	TCCR0 = _BV(CS02);
	TIMSK |= _BV(TOIE0);
}

bool
counter(uint8_t target) //ФУНКЦИЯ ОБНУЛЕНИЯ СЧЁТЧИКА
{
	if (cnt0 > target)
	{
		cnt0 = 0;
		return true;
	}

	cnt0++;

	return false;
}

/*uint8_t minimax_left(uint8_t min, uint8_t max, int error)
{
	int pwm = 0;
	pwm += error;
	return pwm;
}

uint8_t minimax_right(uint8_t min, uint8_t max, int error)
{
	int pwm = 0;
	pwm += error;
	return pwm;
}*/

bool
delay_counter(uint8_t target) //ФУНКЦИЯ ОБНУЛЕНИЯ СЧЁТЧИКА
{
		if (cnt0 > target)
		{
			cnt0 = 0;
			return true;
		}

		cnt0++;

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

// Аппаратное прерывание от энкодеров
ISR(INT0_vect)
{
	speed_cnt_left++;
}

ISR(INT1_vect)
{
	speed_cnt_right++;
}

//Главная функция, которая определяет желаемую скорость в зависимости от наличия препятствий
void
get_d_irection() {
	uint8_t c;

	do {
		switch(cmd) {
		case 0: //ВКЛЮЧЕНИЕ ЛЕВОГО ДАТЧИКА

			DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_LEFT);

			pwm_left = 10;
			pwm_right = 10;

			right_ob = false;
			left_ob = false;

			cmd = 1;

			break;

		case 1: //НИЧЕГО НЕ ДЕЛАТЬ

			if (delay_counter(8)) cmd = 2;

			break;

		case 2: // СЧИТЫВАЕНИЕ ИЗ ЛЕВОГО ДАТЧИКA

			c = 0;
			for (int i = 0; i < 10; i++) {
				if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
			}

			if(c > 7) right_ob = true;

			cmd = 3;

			break;

		case 3: //ВЫКЛЮЧЕНИЕ ЛЕВОГО ДАТЧИКА
			DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT));
			cmd = 4;
			break;

		case 4:
			DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_RIGHT);
			cmd = 5;
			break;

		case 5:
			if(delay_counter(8)) cmd = 6;
			break;

		case 6:
			c = 0;
			for (int i = 0; i < 10; ++i) {
				if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
			}

			if(c > 7) left_ob = true;

			cmd = 7;

			break;

		case 7:
			DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_RIGHT));
			cmd = 8;
			break;
		}
	} while (cmd != 8);
}

//Скорость левого и правого колёс устанавливается в соответствии с желаемой скорости соответственных колёс
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

ISR(TIMER0_OVF_vect)
{
//	get_direction();
	regulate_right();
	regulate_left();

	pwm_left = 10;
	pwm_right = 10;

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

	}
	return 0;
}

/*
// * ISR(TIMER0_OVF_vect)
//{
//	/*speed_left = speed_cnt_left * dt;
//	speed_right = speed_cnt_right * dt;
//
//	speed_cnt_left = 0;
//	speed_cnt_right = 0;
//
//	P = minimax(0, 10, E * K1 + SE * K2);
//	SE += E;                    ^^^^^^^^
//
//		P 0  10  10  10        10   2   2   1
//	E 0 100  90  80  70 60 20  18  16  14
//	V 0   0  10            80  82  84  85
// 	a 0  10  10            10   2   2   1
//
//	if (desired_speed_left - (speed_left*k) < 0) pwm_left--;
//	    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ОШИБКА (Е)
//	if (desired_speed_left - (speed_left*k) > 0) pwm_left++;
//	if(desired_speed_left == 0) pwm_left = 0;
//
//	if (desired_speed_right - (speed_right*k) < 0) pwm_right--;
//	if (desired_speed_right - (speed_right*k) > 0) pwm_right++;
//	if(desired_speed_right == 0) pwm_right = 0;
//
//	pwm_right == desired_speed_left - k * speed_right;
//
//
//
//
//
//	E = desired_speed_left - sp
//
//	*/
//
//	if(counter(61))
//	{
//		speed_left = speed_cnt_left;
//		speed_right = speed_cnt_right;
//
//		speed_cnt_left = 0;
//		speed_cnt_right = 0;
//
//		E_left = desired_speed_left - speed_left;
//		E_right = desired_speed_right - speed_right;
//
//		pwm_left += E_left;
//		pwm_right += E_right;
//
//		//GNU PLOT, GRAPHVIZ
//	}
//
//	uint8_t c;
//
//	switch(cmd) {
//	case 0: //ВКЛЮЧЕНИЕ ЛЕВОГО ДАТЧИКА
//
//		DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_LEFT);
//
//		pwm_left = 10;
//		pwm_right = 10;
//
//		right_ob = false;
//		left_ob = false;
//
//		cmd = 1;
//
//		break;
//
//	case 1: //НИЧЕГО НЕ ДЕЛАТЬ
//
//		if (delay_counter(8)) cmd = 2;
//
//		break;
//
//	case 2: // СЧИТЫВАЕНИЕ ИЗ ЛЕВОГО ДАТЧИКA
//
//		c = 0;
//		for (int i = 0; i < 10; i++) {
//			if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
//		}
//
//		if(c > 7) right_ob = true;
//
//		cmd = 3;
//
//		break;
//
//	case 3: //ВЫКЛЮЧЕНИЕ ЛЕВОГО ДАТЧИКА
//		DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_LEFT));
//		cmd = 4;
//		break;
//
//	case 4:
//		DETECTOR_PORT |= _BV(COM_DETECTOR_OUTPUT_RIGHT);
//		cmd = 5;
//		break;
//
//	case 5:
//		if(delay_counter(8)) cmd = 6;
//		break;
//	case 6:
//		c = 0;
//		for (int i = 0; i < 10; ++i) {
//			if (DETECTOR_PIN & _BV(COM_DETECTOR_INPUT)) c++;
//		}
//
//		if(c > 7) left_ob = true;
//
//		cmd = 7;
//
//		break;
//
//	case 7:
//		DETECTOR_PORT &= ~(_BV(COM_DETECTOR_OUTPUT_RIGHT));
//		cmd = 8;
//		break;
//
//	case 8:
//		if(!right_ob && left_ob) {
//			pwm_left = 10;
//			pwm_right = 0;
//
//			/*desired_speed_left = 1;
//			desired_speed_right = 0;*/
//		}
//
//		if(right_ob && !left_ob) {
//			pwm_left = 0;
//			pwm_right = 10;
//
//			/*desired_speed_left = 0;
//			desired_speed_right = 1;*/
//		}
//
//		if(!right_ob && !left_ob) {
//			pwm_left = 0;
//			pwm_right = 0;
//
//			/*desired_speed_left = 0;
//			desired_speed_right = 0;*/
//		}
//
//		if(right_ob && left_ob) {
//			pwm_left = 5;
//			pwm_right = 5;
//
//			/*desired_speed_left = 0.5;
//			desired_speed_right = 0.5;*/
//		}
//
//		cmd = 0;
//
//		break;
//	}
//
//	if (!pwm_counter_left(pwm_left)) ENGINE_PORT |= _BV(ENGINE_LEFT);
//	else ENGINE_PORT &= ~_BV(ENGINE_LEFT);
//
//	if (!pwm_counter_right(pwm_right)) ENGINE_PORT |= _BV(ENGINE_RIGHT);
//	else ENGINE_PORT &= ~_BV(ENGINE_RIGHT);
//}
// * /

