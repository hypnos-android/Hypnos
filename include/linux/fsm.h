#ifndef FSM_H
#define FSM_H
#include<linux/state.h>
#include<linux/kernel.h>
#define NO_SUCH_FSM 1
typedef enum {GPIO_SET_VALUE, GPIO_DIRECTION_OUTPUT, MMC_WRITEL, AKM, CLK, CONFIG_GPIO_TABLE} cmd;

struct fsm{
	// To access all the states
	struct state* base_state;

	// Keep track of current state
	struct state* current_state;

	// Command to switch the state eg: GPIO_SET_VALUE
	cmd c;

	// Ask the device to make a transition because the command was called
	int (*switch_state)(char* argv, int argc);

	// Pointer to the device to check if it is enabled to wakeup
	struct device* dev;

	//Pointer to the next FSM, just to iterate
	struct fsm* next;

	// Time when the power state was last changed
	unsigned long last_change;
};
extern struct fsm* fsm_head;

extern struct fsm* new_fsm(cmd c);
extern void print_fsm( struct fsm* );
extern void print_all_fsm( void );

extern int register_fsm( struct fsm* );
extern int unregister_fsm( struct fsm* );

extern struct state* add_state( struct fsm*, int power );
#endif
