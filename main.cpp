#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
#include <util/delay.h>

#include "GetDirection.h"
#include "Pwm.h"
#include "RegulateSpeed.h"

volatile uint8_t cnt_d = 0;

void
configure_timer()
{
	TCCR0 = _BV(CS01);
	TIMSK |= _BV(TOIE0);
}

bool
counter_delay(uint8_t target)
{
	cnt_d++;
	if(cnt_d > target)
	{
		cnt_d = 0;
		return true;
	}
	else return false;
}

//Прерывание от таймера
ISR(TIMER0_OVF_vect)
{
	if(counter_delay(16))
	{
	//get_direction();
//
//	regulate_right();
	regulate_left();
	}
//
//	do_pwm_right();
	//pwm_left = 6;
	do_pwm_left();
}

int
main(void)
{
	cli();

	configure_pins_detectors();
	configure_pins_engines();
	configure_timer();
	configure_pins_obs();

	sei();

	while (1)
	{
		asm("nop");
	}
	return 0;
}
