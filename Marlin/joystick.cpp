#include "Configuration.h"
#include "pins.h"
#include "joystick.h"
#include "Marlin.h"
#include "temperature.h"
// stepper suspension
#include "stepper.h"

#define JOYSTICK_MIDPOINT 8192

static bool joystick_enabled = false;

inline float aroundZero(float a)
{
  if (a > JOYSTICK_MIDPOINT - JOYSTICK_MIDPOINT / 10 &&
      a < JOYSTICK_MIDPOINT + JOYSTICK_MIDPOINT / 10) {
    return 0;
  } else {
    return (a - JOYSTICK_MIDPOINT) / JOYSTICK_MIDPOINT;
  }
}

static void get(float& x, float& y, int delta)
{
  int vals[JOYSTICK_NUM_AXIS];
  getJoystick(vals);
  x = aroundZero(vals[0]) * delta / 10000.0;
  y = -aroundZero(vals[1]) * delta / 10000.0;
  // x = vals[0];
  // y = vals[1];
}

void joystick_init()
{
  SERIAL_ECHO_START;
  SERIAL_ECHOLNPGM("Joystick configured (enable by pushing button)");
  pinMode(JOYSTICK_BUTTON, INPUT);
}

void joystick_handle(float current_position[NUM_AXIS],
                     float destination[NUM_AXIS],
                     float& feedrate)
{
  if (!joystick_enabled && digitalRead(JOYSTICK_BUTTON)) {
    joystick_enabled = true;
    SERIAL_ECHO_START;
    SERIAL_ECHOLNPGM("Joystick enabled by pushing button");
  }

  if (!joystick_enabled) {
    return;
  }

  static bool was_moved = false;
  unsigned long time_now = micros();
  static unsigned long time_prev = 0;
  unsigned long delta = time_now - time_prev;
  time_prev = time_now;
  if (delta > 30000) {
    delta = 30000;
  }
  float x_adj, y_adj;
  get(x_adj, y_adj, delta);

  if (x_adj != 0 || y_adj != 0) {
    SERIAL_ECHO_START;
    SERIAL_ECHOPAIR(" X ", x_adj); 
    SERIAL_ECHOPAIR(" Y ", y_adj); 
    SERIAL_ECHOPAIR(" delta ", delta); 
    SERIAL_ECHOLN(""); 

    destination[X_AXIS] = current_position[X_AXIS] + x_adj;
    destination[Y_AXIS] = current_position[Y_AXIS] + y_adj;

    feedrate = sqrt(x_adj * x_adj + y_adj * y_adj) * 5000;

    prepare_move();

    was_moved = true;
  } else {
    if (was_moved) {
      st_synchronize();
      disable_e0();
      disable_e1();
      disable_e2();
      finishAndDisableSteppers();
    }
    was_moved = false;
  }
}
