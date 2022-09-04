#include "ch.h"
#include "hal.h"
#include "logger.h"
#include "console.h"
#include "joystick.h"

#define VOLTAGE_RES (3.3 / 4096.0)

#define ADC_GRP_NUM_CHANNELS  5
#define ADC_GRP_BUF_DEPTH    64

static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

// Questi sono i bottoni centrali dei joystick
#define BUTTON_LINE_1 PAL_LINE(GPIOA, 9)
#define BUTTON_LINE_2 PAL_LINE(GPIOC, 7)

struct PhysicalJoystick {
  Joystick base;
  Button button;
  Sensitivity sensitivity;
};

typedef struct {
  _Bool running;
  unsigned int pow_filter_size;
  float        pow_value;
  PhysicalJoystick joysticks[2];
} Console;

typedef struct {
  int low_x, high_x;
  int low_y, high_y;
} SensitivityThresholds;

// TODO: Valuta i migliori profili
static const SensitivityThresholds sensitivity_thresholds[] = {
  [SENSITIVITY_LOW]    = { .low_x = 10, .high_x = 90, .low_y = 10, .high_y = 90 },
  [SENSITIVITY_MEDIUM] = { .low_x = 10, .high_x = 90, .low_y = 10, .high_y = 90 },
  [SENSITIVITY_HIGH]   = { .low_x = 10, .high_x = 90, .low_y = 10, .high_y = 90 },
};

static Console console;

/*
 * ADC conversion group.
 * Mode:        Continuous on 4 channels, SW triggered.
 * Channels:    IN10 (GPIOC0), IN11 (GPIOC1), IN4 (GPIOA4), IN1 (GPIOA1)
 */
static const ADCConversionGroup adcgrpcfg = {
  FALSE,
  ADC_GRP_NUM_CHANNELS,
  0,
  0,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3) |
  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_3) | ADC_SMPR1_SMP_AN13(ADC_SAMPLE_3) |
  ADC_SMPR1_SMP_AN14(ADC_SAMPLE_3), /* SMPR1 */
  0,                        /* SMPR2 */
  0,                        /* HTR */
  0,                        /* LTR */
  ADC_SQR1_NUM_CH(ADC_GRP_NUM_CHANNELS),  /* SQR1 */
  0,                        /* SQR2 */
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN11) | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10) |
  ADC_SQR3_SQ3_N(ADC_CHANNEL_IN1) | ADC_SQR3_SQ4_N(ADC_CHANNEL_IN4) |
  ADC_SQR3_SQ5_N(ADC_CHANNEL_IN0)/* SQR3 */
};

static float converted[ADC_GRP_NUM_CHANNELS];

static THD_WORKING_AREA(waAdc, 2048);
static THD_FUNCTION(thdAdc, arg) {

  (void) arg;

  unsigned int pow_sample_count = 0;
  float pow_avg = 0;

  while (console.running) {

    msg_t msg = adcConvert(&ADCD1, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);
    // TODO: Check for errors
    if (msg != MSG_OK) {
      Logger_printf("Conversion error");
      continue;
    }

    // Inizializza il buffer
    for (int i = 0; i < ADC_GRP_NUM_CHANNELS; i++)
      converted[i] = 0.0f;

    // Normalizza i valori campionati
    for (int i = 0; i < ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH; i++)
      converted[i % ADC_GRP_NUM_CHANNELS] += (float) samples[i] * 100 / 4096;

    // Calcola la media
    for (int i = 0; i < ADC_GRP_NUM_CHANNELS; i++)
      converted[i] /= ADC_GRP_BUF_DEPTH;

    /* -- Joystick 0 -- */
    {
      float x = converted[0];
      float y = converted[1];

      PhysicalJoystick *joystick = console.joysticks + 0;
      const SensitivityThresholds th = sensitivity_thresholds[joystick->sensitivity];
      Button button;
      if (x < th.low_x)       button = BUTTON_LEFT;
      else if (x > th.high_x) button = BUTTON_RIGHT;
      else if (y < th.low_y)  button = BUTTON_DOWN;
      else if (y > th.high_y) button = BUTTON_UP;
      else if (palReadLine(BUTTON_LINE_1) == PAL_LOW)
        button = BUTTON_MIDDLE;
      else button = BUTTON_NULL;

#ifndef NOLOGGING
      if (joystick->button != button) {
        Logger_printf("Joystick 0 pressed %s", buttonName(button));
      }
#endif
      joystick->button = button;
    }

    /* -- Joystick 1 -- */
    {
      float x = converted[2];
      float y = converted[3];

      PhysicalJoystick *joystick = console.joysticks + 1;
      SensitivityThresholds th = sensitivity_thresholds[joystick->sensitivity];
      Button button;
      if (x < th.low_x)       button = BUTTON_LEFT;
      else if (x > th.high_x) button = BUTTON_RIGHT;
      else if (y < th.low_y)  button = BUTTON_DOWN;
      else if (y > th.high_y) button = BUTTON_UP;
      else if (palReadLine(BUTTON_LINE_2) == PAL_LOW)
        button = BUTTON_MIDDLE;
      else button = BUTTON_NULL;

#ifndef NOLOGGING
      if (joystick->button != button) {
        Logger_printf("Joystick 1 pressed %s", buttonName(button));
      }
#endif
      joystick->button = button;
    }

    /* -- Potenziometro -- */
    pow_avg += converted[4];
    pow_sample_count++;

    if(pow_sample_count == console.pow_filter_size) {
      pow_avg= pow_avg / console.pow_filter_size;
      console.pow_value = 8.197 * pow_avg; // TODO
      pow_sample_count = 0;
      pow_avg = 0.0;
    }

    chThdSleepMilliseconds(50);
  }
}

static Button getButton(Joystick *joystick, int player);

static JoystickMethodTable table = {
  .getButton = getButton,
  .free = 0,
};

static Button getButton(Joystick *joystick, int player)
{
  PhysicalJoystick *joystick2 = (PhysicalJoystick*) joystick;

  (void) player;

  return joystick2->button;
}

static void PhysicalJoystick_init(PhysicalJoystick *joystick)
{
  joystick->base.table = &table;
  joystick->button = BUTTON_NULL;
  joystick->sensitivity = SENSITIVITY_MEDIUM;
}

void Console_init(void)
{
  palSetLineMode(BUTTON_LINE_1, PAL_MODE_INPUT_PULLUP);
  palSetLineMode(BUTTON_LINE_2, PAL_MODE_INPUT_PULLUP);

  /* Setting as analog input:
   *    PORTC PIN 0 -> ADC1_CH10
   *    PORTC PIN 1 -> ADC1_CH11
   */
  palSetGroupMode(GPIOC, PAL_PORT_BIT(0) | PAL_PORT_BIT(1),
                  0, PAL_MODE_INPUT_ANALOG);

  /* Setting as analog input:
   *    PORTA PIN 1 -> ADC1_CH1
   *    PORTA PIN 4 -> ADC1_CH4
   */
  palSetGroupMode(GPIOA, PAL_PORT_BIT(0) | PAL_PORT_BIT(1) |
                  PAL_PORT_BIT(4), 0, PAL_MODE_INPUT_ANALOG);

  adcStart(&ADCD1, NULL);
  //adcSTM32EnableTSVREFE();

  console.running = 1;
  console.pow_filter_size = 32;
  PhysicalJoystick_init(console.joysticks + 0);
  PhysicalJoystick_init(console.joysticks + 1);

  chThdCreateStatic(waAdc, sizeof(waAdc), NORMALPRIO+1,
                    thdAdc, (void*) NULL);
}

void Console_quit(void)
{
  adcStop(&ADCD1);
  console.running = 0;
}

PhysicalJoystick *Console_getPhysicalJoystick(int index)
{
  if (index < 2)
    return console.joysticks + index;
  return 0;
}

void PhysicalJoystick_setSensitivity(PhysicalJoystick *joystick, Sensitivity level)
{
  joystick->sensitivity = level;
}
