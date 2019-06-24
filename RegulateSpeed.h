#ifndef REGULATESPEED_H_
#define REGULATESPEED_H_

void
regulate_left();

void
regulate_right();

void
configure_pins_obs();

#define OBS_DIR DDRD
#define OBS_PORT PORTD

#define OBS_LEFT PD2
#define OBS_RIGHT PD3

volatile extern int pwm_left;
volatile extern int pwm_right;

#endif /* REGULATESPEED_H_ */
