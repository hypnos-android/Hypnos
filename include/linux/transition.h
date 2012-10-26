#ifndef TRANSITION_H
#define TRANSITION_H
#include <linux/state.h>
typedef enum {TIMEOUT, PROGRAM, EXTERNAL} transition_type;
struct transition {
	// State where the transition begins
	struct state* from;

	// State where the transition ends
	struct state* to;

	// Type of the transition
	transition_type type;

	// Timeout: only to be set, if the type is TIMEOUT
	unsigned long timeout;

	// Another transition with same source, just to iterate
	struct transition* next_from_transition;

	// Another transition with same destination, just to iterate
	struct transition* next_to_transition;
};

// Both the states must be properly initialized before calling this function
extern struct transition* add_transition( struct state* from, struct state* to, transition_type t);
extern void print_all_transitions(struct transition* t, int flag);
extern void print_transition(struct transition* t);
extern void set_timeout(struct transition* trans, int timeout);
#endif
