#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
//#include "header.h"
#include "PWM.h"
#include "adjust_speed.h"

volatile uint8_t cnt1 = 0;
volatile uint8_t cnt2 = 0;

void
configure_pins_engines()
{
	ENGINE_DIR |= _BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT);
	ENGINE_PORT &= ~(_BV(ENGINE_LEFT) | _BV(ENGINE_RIGHT));
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


//Запускает ШИМ для двигателей
void
do_pwm_left()
{
	if (!pwm_counter_left(pwm_left, 20)) PUSK_LEFT;
	else STOP_LEFT;
}

void
do_pwm_right()
{
	if (!pwm_counter_right(pwm_right, 20)) PUSK_RIGHT;
	else STOP_RIGHT;
}
