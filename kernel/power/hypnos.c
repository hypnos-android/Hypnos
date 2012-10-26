#include <linux/hypnos.h>
#include <linux/fsm.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/android_alarm.h>
#include <linux/pm_wakeup.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>

struct rtc_time next_alarm;
struct hypnos_update_time jut;
struct wake_lock wlock;
int flag;
unsigned long min_trans_time;
struct alarm alrm;

static void hypnos_record_time(struct work_struct* work){
	struct fsm* dev_fsm;
	struct rtc_time rtc_current_rtc_time;
	struct hypnos_update_time* hut;

	hut = container_of(work, struct hypnos_update_time, update);

	dev_fsm = hut->dev_fsm;
	hypnos_rtc_read_time(&rtc_current_rtc_time);
	rtc_tm_to_time(&rtc_current_rtc_time, &(dev_fsm->last_change));
	//pr_info("hypnost: updated time %lu\n", dev_fsm->last_change);
}

static void state_change(cmd change, char* argv, int argc){
	struct fsm* iterator;

	iterator = fsm_head;

	while(iterator){
		if(iterator->c == change){
			if(iterator->switch_state(argv, argc)){
				jut.dev_fsm = iterator;
				//pr_info("hypnost: initializing work\n");
				schedule_work(&jut.update);
				//pr_info("hypnost: scheduled work\n");
				return;
			}
		}
		iterator = iterator->next;
	}
}

void hypnos_dummy(struct alarm* alrm){
	//pr_info("hypnosm: hypnos_dummy called\n");
	wake_lock_timeout(&wlock, HZ*2);
}

void hypnos_rtc_set_alarm(struct rtc_wkalrm* rtc_alarm){
	unsigned long alarmTime;
	if(rtc_alarm->enabled == 1){
		memcpy(&next_alarm, &(rtc_alarm->time), sizeof(struct rtc_time));
		rtc_tm_to_time(&next_alarm, &alarmTime);
		//pr_info( "hypnos: alarm set for %lu\n", alarmTime);
	}
}

int hypnos_mitigate_timeout(struct fsm* dev_fsm, struct transition* trans){
	unsigned long rtc_alrm_time;
	unsigned long trans_time;

	if(trans->type == TIMEOUT){
		trans_time = trans->timeout + dev_fsm->last_change;
		rtc_tm_to_time(&next_alarm, &rtc_alrm_time );

		//pr_info("hypnosm: trans_time %lu tout: %lu, chng: %lu\n, alarm time %lu\n", 
				//trans_time, trans->timeout, dev_fsm->last_change, rtc_alrm_time);

		if(trans_time < rtc_alrm_time){
			if(!min_trans_time || trans_time < min_trans_time){
				min_trans_time = trans_time;
			}
			return 1;
		}
	}
	return 0;
}

int hypnos_mitigate_program(struct fsm* dev_fsm, struct transition* trans){
	return 1;
}

int hypnos_mitigate_external(struct fsm* dev_fsm, struct transition* trans){
	if(dev_fsm->dev){
		if(device_may_wakeup(dev_fsm->dev)){
			return 0;
		}
	}
	return 1;
}

void hypnos_resume_late(void){
	ktime_t wakeup_ktime;
	struct timespec wakeup_ts;
	struct rtc_time rtc_current_rtc_time;
	unsigned long curr;

	//pr_info("hypnosr: resuming, flag %d, min_trans_time %lu\n", flag, min_trans_time);

	hypnos_rtc_read_time(&rtc_current_rtc_time);
	rtc_tm_to_time(&rtc_current_rtc_time, &curr);
	//pr_info("hypnosr: current time %lu\n", curr);

	if(flag && min_trans_time > curr){


		wakeup_ts.tv_sec = min_trans_time;
		wakeup_ts.tv_nsec = 0;

		wakeup_ktime = timespec_to_ktime(wakeup_ts);

		alarm_start_range(&alrm, wakeup_ktime, wakeup_ktime);
		//pr_info("hypnosr: alarm set %lu\n", min_trans_time);
		min_trans_time = 0;
	}
}

int hypnos_suspend_late(void) {
	struct fsm* iterator;
	struct state* cur;
	struct transition* trans_iterator;
	int ret = 0;
	flag = 0;

	iterator = fsm_head;
	while(iterator){
		if(iterator->current_state != iterator->base_state){
			pr_info( "hypnos: current state != base_state for %x\n", iterator);
			cur = iterator->current_state;
			trans_iterator = cur->out;
			while(trans_iterator){
				switch(trans_iterator->type){
					case TIMEOUT:
						//pr_info( "hypnosm: timeout transition type");
						flag += hypnos_mitigate_timeout(iterator, trans_iterator);
						break;
					case PROGRAM:
						//pr_info( "hypnosm: program transition type");
						ret += hypnos_mitigate_program(iterator, trans_iterator);
						break;
					case EXTERNAL:
						//pr_info( "hypnosm: external transition type");
						ret += hypnos_mitigate_external(iterator, trans_iterator);
						break;
					default:
						pr_info( "hypnosm: unknown transition type %d", trans_iterator->type);
				}
				trans_iterator = trans_iterator->next_to_transition;
			}
		}
		else{
			pr_info( "hypnos: current state matches base_state for %x\n", iterator);
		}

		iterator = iterator->next;
	}
	ret = ret+flag;
	return ret;
}

long gettime(void){
	struct timeval tm;

	do_gettimeofday(&tm);
	return tm.tv_sec;
}

void hypnos_gpio_direction_output(unsigned gpio, int value){
	void* argv;
	int argc;
	int curr;

	argc = sizeof(gpio) + sizeof(value);
	argv = (void*)kmalloc(argc*sizeof(void), GFP_KERNEL);
	curr = 0;
	memcpy(argv, &gpio, sizeof(gpio));
	curr += sizeof(gpio);
	memcpy(argv + curr, &value, sizeof(value));

	state_change(GPIO_DIRECTION_OUTPUT, argv, argc);
}

void hypnos_gpio_set_value(unsigned gpio, int value){
	void* argv;
	int argc;
	int curr;

	argc = sizeof(gpio) + sizeof(value);
	argv = (void*)kmalloc(argc*sizeof(void), GFP_KERNEL);
	curr = 0;
	memcpy(argv, &gpio, sizeof(gpio));
	curr += sizeof(gpio);
	memcpy(argv + curr, &value, sizeof(value));

	state_change(GPIO_SET_VALUE, argv, argc);
}

static int __init hypnos_init(void){
	pr_info( "hypnos: hypnos_init called \n");
	INIT_WORK(&jut.update, hypnos_record_time);
	alarm_init(&alrm, ANDROID_ALARM_RTC_WAKEUP, hypnos_dummy);
	wake_lock_init(&wlock, WAKE_LOCK_SUSPEND, "hypnos_wl");
	min_trans_time = 0;
	flag = 0;
	return 0;
}

static void  __exit hypnos_exit(void){
	pr_info( "hypnos: hypnos_exit called \n");
}

core_initcall(hypnos_init);
module_exit(hypnos_exit);
