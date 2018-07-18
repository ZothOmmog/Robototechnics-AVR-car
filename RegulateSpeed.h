#ifndef REGULATESPEED_H_
#define REGULATESPEED_H_

void
regulate_left();

void
regulate_right();

volatile extern int pwm_left;
volatile extern int pwm_right;

#endif /* REGULATESPEED_H_ */
