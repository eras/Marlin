#ifndef JOYSTICK_H
#define JOYSTICK_H

void joystick_init();
void joystick_handle(float current_position[NUM_AXIS],
                     float destination[NUM_AXIS],
                     float& feedrate);

#endif
