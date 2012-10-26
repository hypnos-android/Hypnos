#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fsm.h>
#include <linux/state.h>
#include <linux/transition.h>

#define ON_POWER 100
#define GPIO_VIB 21
#define VIB_TIMEOUT 10

struct fsm* vib_fsm;
struct transition* timeout_trans;
int vib_switch_state(char* argv, int argc){
	unsigned gpio;
	int value;
	int curr;
	int expected_argc;

	curr = 0;
	expected_argc = sizeof(gpio) + sizeof(value);
	if(argc == expected_argc){
		memcpy( (void*)&gpio, argv, sizeof(gpio) );
		curr += sizeof(gpio);
		memcpy( (void*)&value, argv + curr, sizeof(value) );
		kfree(argv);
		if(gpio == GPIO_VIB){
			if(value)
				vib_fsm->current_state = search_state( vib_fsm, ON_POWER );
			else
				vib_fsm->current_state = vib_fsm->base_state;
			return 1;
		}
	}
	return 0;
}

static struct kprobe kp = {
	.symbol_name	= "gpio_enable",
};

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	long time;
#ifdef CONFIG_ARM
	time = regs->ARM_r1;
	printk(KERN_INFO "time: %ld\n", time);
	set_timeout(timeout_trans, 1+(time/1000));
#endif
	return 0;
}

static int __init kprobe_init(void) {
	int ret;
	struct state* on_state;
	struct transition* trans;

	vib_fsm = new_fsm(GPIO_DIRECTION_OUTPUT);
	on_state = add_state( vib_fsm, ON_POWER);
	trans = add_transition( vib_fsm->base_state, on_state, PROGRAM );
	timeout_trans = add_transition( on_state, vib_fsm->base_state, TIMEOUT );
	set_timeout(timeout_trans, VIB_TIMEOUT);
	vib_fsm->switch_state = vib_switch_state;
	register_fsm( vib_fsm );

	kp.pre_handler = handler_pre;
	ret = register_kprobe(&kp);
	if (ret < 0) {
		printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
		return ret;
	}
	printk(KERN_INFO "Planted kprobe at %p\n", kp.addr);
	return 0;
}

static void __exit kprobe_exit(void) {
	unregister_fsm( vib_fsm );
	unregister_kprobe(&kp);
	printk(KERN_INFO "kprobe at %p unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
