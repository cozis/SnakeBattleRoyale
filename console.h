

typedef enum {
  SENSITIVITY_LOW,
  SENSITIVITY_MEDIUM,
  SENSITIVITY_HIGH,
} Sensitivity;

void Console_init(void);
void Console_quit(void);

typedef struct PhysicalJoystick PhysicalJoystick;
PhysicalJoystick *Console_getPhysicalJoystick(int index);
void PhysicalJoystick_setSensitivity(PhysicalJoystick *joystick, Sensitivity level);

