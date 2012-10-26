#ifndef STATE_H
#define STATE_H
#include<linux/transition.h>
struct fsm;
typedef struct state {
	// List of all the transitions that come to this state
	struct transition* in;

	// List of all the out transitions
	struct transition* out;

	// Current in mA
	unsigned power;

	// Another state in the FSM (nothing to do with edges, transitions)
	// Just to iterate the list of states in a FSM
	struct state* next;
}state;

extern struct state* search_state(struct fsm* f, unsigned power);
extern void print_state( struct state* s );
#endif
