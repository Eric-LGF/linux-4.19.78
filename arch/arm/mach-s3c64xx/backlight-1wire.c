#include <linux/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>
#include <linux/delay.h>


static int one_wire_gpio = 0;


static unsigned char crc8(unsigned v, unsigned len);

static void set_pin_as_input(void)
{
	gpio_direction_input(one_wire_gpio);
}

static void set_pin_as_output(void)
{
}

static void set_pin_value(int v)
{
	gpio_direction_output(one_wire_gpio, v);
}

static int get_pin_value(void)
{
	return gpio_get_value(one_wire_gpio);
}

static void StartTimer(void)
{
}

static void StopTimer(void)
{
}

static void WaitTimerTick(void)
{
	udelay(104);
}

static int OneWireSession(unsigned char req, unsigned char res[])
{
	unsigned Req;
	unsigned *Res;
	unsigned int i;

	Req = (req << 24) | (crc8(req << 24, 8) << 16);
	Res = (unsigned *)res;

	set_pin_value(1);
	set_pin_as_output();
	StartTimer();
	for (i = 0; i < 60; i++) {
		WaitTimerTick();
	}
	set_pin_value(0);
	for (i = 0; i < 2; i++) {
		WaitTimerTick();
	}
	for (i = 0; i < 16; i++) {
		int v = !!(Req & (1U << 31));
		Req <<= 1;
		set_pin_value(v);
		WaitTimerTick();
	}
	WaitTimerTick();
	set_pin_as_input();
	WaitTimerTick();
	for (i = 0; i < 32; i++) {
		(*Res) <<= 1;
		(*Res) |= get_pin_value();
		WaitTimerTick();
	}
	StopTimer();
	set_pin_value(1);
	set_pin_as_output();

	return crc8(*Res, 24) == res[0];
}

static int TryOneWireSession(unsigned char req, unsigned char res[])
{
	int i;
	for (i = 0; i < 3; i++) {
		if (OneWireSession(req, res)) {
			return 1;
		}
	}
	return 0;
}

int SetBacklightOfLCD(unsigned Brightness)
{
	unsigned char res[4];
	int ret;
	if (Brightness > 127) {
		Brightness = 127;
	}
	ret = TryOneWireSession(Brightness | 0x80U, res);
	return ret;
}

static unsigned char crc8(unsigned v, unsigned len)
{
	unsigned char crc = 0xACU;
	while (len--) {
		if ((crc & 0x80U) != 0) {
			crc <<= 1;
			crc ^= 0x7U;
		} else {
			crc <<= 1;
		}
		if ((v & (1U << 31)) != 0) {
			crc ^= 0x7U;
		}
		v <<= 1;
	}
	return crc;
}

static int one_wire_setup(void)
{
	printk("gpio = %d\n", one_wire_gpio);

	if (!gpio_is_valid(one_wire_gpio)) {
		printk("gpio: %d is invalid\n", one_wire_gpio);

		return -ENODEV;
	}

	// 请求控制获取的gpio
	if (gpio_request(one_wire_gpio, "one-wrie")) {
		printk("gpio %d request failed\n", one_wire_gpio);
		gpio_free(one_wire_gpio);
		
		return -ENODEV;
	}

	// 设置背光
	printk("set backlight\n");
	SetBacklightOfLCD(127);

	return 0;
}

int backlight_init(int onewire_pin)
{
    one_wire_gpio = onewire_pin;

    return one_wire_setup();
}