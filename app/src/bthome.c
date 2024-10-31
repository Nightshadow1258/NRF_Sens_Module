#include "bthome.h"

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
}

int bthome_test(void)
{
	int err;
	int temp = 0;

	printk("Starting BTHome sensor template\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	for (;;) {
		/* Simulate temperature from 0C to 25C */
		service_data[IDX_TEMPH] = (temp * 100) >> 8;
		service_data[IDX_TEMPL] = (temp * 100) & 0xff;
		if (temp++ == 25) {
			temp = 0;
		}
		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			printk("Failed to update advertising data (err %d)\n", err);
		}
		k_sleep(K_MSEC(BT_GAP_ADV_SLOW_INT_MIN));
	}
	return 0;
}