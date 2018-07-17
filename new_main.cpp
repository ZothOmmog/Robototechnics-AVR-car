#include <avr/interrupt.h>
#include <avr/iom16.h>
#include <avr/sfr_defs.h>
#include <stdint.h>
#include <util/delay.h>

#include "adjust_speed.h"
#include "PWM.h"
#include "CalculateWishSpeed.h"

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

	regulate_right();
	regulate_left();

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
	//DDRD |= _BV(PD4);
	while (1)
	{
		/*for(int i = 0; i < 4; i++)
		{
			desired_speed_left = 8 + i*2;
			desired_speed_right = 8 + i*2;

			PORTD ^= _BV(PD4);
			_delay_ms(20000);
		}*/

		//desired_speed_left = 1;
		//desired_speed_right = 1;
		asm("nop");
	}
	return 0;
}
