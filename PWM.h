#ifndef PWM_H_
#define PWM_H_

void
do_pwm_left();

void
do_pwm_right();

void
configure_pins_engines();

//===============Двигатели==================
#define ENGINE_DIR DDRD

#define ENGINE_PORT PORTD

#define ENGINE_LEFT PD5
#define ENGINE_RIGHT PD7

//Запуск двигателей
#define PUSK_LEFT ENGINE_PORT |= _BV(ENGINE_LEFT)
#define PUSK_RIGHT ENGINE_PORT |= _BV(ENGINE_RIGHT)
//-----------------

//Остановка двигателей
#define STOP_LEFT ENGINE_PORT &= ~_BV(ENGINE_LEFT)
#define STOP_RIGHT ENGINE_PORT &= ~_BV(ENGINE_RIGHT)
//--------------------
//==========================================

#endif /* PWM_H_ */
