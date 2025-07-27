#include "sensors.h"
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(Sensor_Modul);


// *******************
// generic functions
// *******************
int sensors_init(void)
{
    int err = 0;

    err |= sensor_sht4x_init(); // Temperature / humidity
    err |= sensor_adc_init();   // Battery level
    err |= sensor_door_init();      // Door sensor or interrupt line
    LOG_INF("Sensor init complete: SHT4x OK | ADC: OK | GPIO: OK");
    return err;
}


// *******************
// SHT4x Section
// *******************

static int sensor_sht4x_init(void)
{
    if (!device_is_ready(dev_sht4x))
    {
        LOG_ERR("SHT4x not ready");
        return -ENODEV;
    }
    // Store device handle if needed
    return 0;
}

struct sensor_value sensor_sht4x_get_temperature(void)
{
    struct sensor_value temp;
    if (sensor_sample_fetch(dev_sht4x) ||
        sensor_channel_get(dev_sht4x, SENSOR_CHAN_AMBIENT_TEMP, &temp))
    {
        LOG_WRN("Failed to read temperature");
        return temp;
    }
    LOG_DBG("Temperature read: %0.2f C", sensor_value_to_double(&temp));
    return temp;
    // return sensor_value_to_double(&temp);
}

struct sensor_value sensor_sht4x_get_humidity(void)
{
    struct sensor_value hum;
    if (sensor_channel_get(dev_sht4x, SENSOR_CHAN_HUMIDITY, &hum))
    {
        LOG_WRN("Failed to read humidity");
        return hum;
    }
    LOG_DBG("Humidity read: %0.2f %%", sensor_value_to_double(&hum));
    return hum;
    // return sensor_value_to_double(&hum);
}



// *******************
// ADC Section
// *******************
uint16_t buf;

static struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                         DT_SPEC_AND_COMMA)};


struct adc_sequence sequence = {
    .buffer = &buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(buf),
};

static int sensor_adc_init(void)
{
    int err;
    LOG_INF("ADC initialized for battery measurement");

    /* Configure channels individually prior to sampling. */
    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
    {
        if (!adc_is_ready_dt(&adc_channels[i]))
        {
            LOG_DBG("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return 0;
        }

        err = adc_channel_setup_dt(&adc_channels[i]);
        if (err < 0)
        {
            LOG_DBG("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }
    }
    return 0;
}

double get_adc_data(void)
{
    int err;
    uint32_t count = 0;
    double vbat_raw_lsb;
    LOG_DBG("ADC reading[%u]:\n", count++);

    for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
    {
        int32_t val_mv;

        LOG_DBG("- %s, channel %d: ",
                adc_channels[i].dev->name,
                adc_channels[i].channel_id);

        (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

        err = adc_read_dt(&adc_channels[i], &sequence);
        if (err < 0)
        {
            LOG_DBG("Could not read (%d)\n", err);
            continue;
        }

        /*
         * If using differential mode, the 16 bit value
         * in the ADC sample buffer should be a signed 2's
         * complement value.
         */
        if (adc_channels[i].channel_cfg.differential)
        {
            val_mv = (int32_t)((int16_t)buf);
        }
        else
        {
            val_mv = (int32_t)buf;
        }

        vbat_raw_lsb = (uint16_t)val_mv / 4095 * 3.3 * 4 / 3;

        LOG_DBG("Battery Voltage %f\n", vbat_raw_lsb);
    }
    return vbat_raw_lsb;
}

// *******************
// Door Sensor Section
// *******************
struct gpio_dt_spec door = GPIO_DT_SPEC_GET(DOOR_NODE, gpios);


static void debounce_handler(struct k_work *work)
{
	int val = gpio_pin_get_dt(&door);
	LOG_INF("Debounced door state: %s\n", val ? "OPEN" : "CLOSED");
}

static void door_callback(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
	k_work_reschedule(&debounce_work, K_MSEC(30)); // 30 ms debounce window
}

static int sensor_door_init(void) {
	// Initialize GPIO for door sensor
	if (!device_is_ready(door.port))
	{
		LOG_ERR("Door GPIO device not ready");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&door, GPIO_INPUT | GPIO_PULL_UP))
	{
		LOG_ERR("Failed to configure door GPIO pin");
		return -ENODEV;
	}

	// Enable interrupt before adding callback
	if (gpio_pin_interrupt_configure_dt(&door, GPIO_INT_EDGE_BOTH))
	{
		LOG_ERR("Failed to configure door GPIO interrupt");
		return -ENODEV;
	}

	gpio_init_callback(&door_cb_data, door_callback, BIT(door.pin));

	if (gpio_add_callback(door.port, &door_cb_data))
	{
		LOG_ERR("Failed to add door callback");
		return -ENODEV;
	}

	// Debounce work item setup
	k_work_init_delayable(&debounce_work, debounce_handler);

	// Wakeup source for power management
	pm_device_wakeup_enable(door.port, true);
    return 0;
}

int sensor_get_door_state(void)
{
    int val = gpio_pin_get_dt(&door);
    LOG_DBG("Door state read: %s", val ? "OPEN" : "CLOSED");
    return val; // 0 for closed, 1 for open
}