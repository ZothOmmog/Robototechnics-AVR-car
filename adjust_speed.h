#ifndef ADJUST_SPEED_H_
#define ADJUST_SPEED_H_

void regulate_left();
void regulate_right();

volatile extern int pwm_left;
volatile extern int pwm_right;

#endif /* ADJUST_SPEED_H_ */
