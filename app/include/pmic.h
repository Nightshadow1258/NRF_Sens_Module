#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/npm1300_charger.h>
#include <zephyr/dt-bindings/regulator/npm1300.h>
#include <zephyr/drivers/mfd/npm1300.h>


#define SW_LSW1 DT_NODELABEL(loadsw0)
//#define SW_TEST DT_NODELABEL(loadsw1)

static const struct gpio_dt_spec lsw1 = GPIO_DT_SPEC_GET(SW_LSW1, gpios);
//static const struct gpio_dt_spec swtest = GPIO_DT_SPEC_GET(SW_TEST, gpios);

// PMIC stuff
static const struct device *regulators = DEVICE_DT_GET(DT_NODELABEL(npm1300_ek_regulators));

static const struct device *charger = DEVICE_DT_GET(DT_NODELABEL(npm1300_ek_charger));

static const struct device *leds = DEVICE_DT_GET(DT_NODELABEL(npm1300_ek_leds));

static const struct device *pmic = DEVICE_DT_GET(DT_NODELABEL(npm1300_ek_pmic));


void read_sensors(void);