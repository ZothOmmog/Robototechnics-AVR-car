#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>

#include "adjust_speed.h"
#include "PWM.h"
#include "CalculateWishSpeed.h"

volatile uint8_t cnt3 = 0;

//Счетчик, через который мы определяем настало ли время сбрасывать счетчик прерываний от внешнего воздействия
bool
counter_regulate_speed(uint8_t target)
{
		if (cnt3 > target) cnt3 = 0;
		return cnt3++ > target;
}

void
configure_timer()
{
	TCCR0 = _BV(CS02);
	TIMSK |= _BV(TOIE0);
}

//Прерывание от таймера
ISR(TIMER0_OVF_vect)
{
	get_direction();

	if(counter_regulate_speed(121)) {
	regulate_right();
	regulate_left();
	}

	do_pwm_right();
	do_pwm_left();
}

int
main(void)
{
	cli();

	configure_pins_detectors();
	configure_pins_engines();
	configure_timer();

	sei();

	while (1)
	{
		asm("nop");
	}
	return 0;
}




