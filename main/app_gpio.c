#include "driver/gpio.h"

void init_gpio(void)
{
	// GPIO 設定
	gpio_config_t io_conf;

	// GPIO4 を使用
	io_conf.pin_bit_mask = 1ULL << GPIO_NUM_4
	                     | 1ULL << GPIO_NUM_5
	                     | 1ULL << GPIO_NUM_6
	                     | 1ULL << GPIO_NUM_7;

	// 出力に設定
	io_conf.mode = GPIO_MODE_OUTPUT;

	// PULL DOWN 無効
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

	// PULL-UP 無効
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

	// 割り込み禁止
	io_conf.intr_type = GPIO_INTR_DISABLE;

	// GPIO レジスタに設定
	gpio_config(&io_conf);
}

