#ifndef HYPNOS_H
#define HYPNOS_H
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/rtc.h>
#include <linux/clk.h>

struct hypnos_update_time{
	struct work_struct update;
	struct fsm* dev_fsm;
};

extern void hypnos_resume_late(void);
extern int hypnos_suspend_late(void);
extern void hypnos_gpio_set_value(unsigned gpio, int value);
extern void hypnos_gpio_direction_output(unsigned gpio, int value);
extern void hypnos_rtc_set_alarm(struct rtc_wkalrm* rtc_alarm);
#endif
