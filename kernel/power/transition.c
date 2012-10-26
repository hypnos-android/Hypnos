#include <linux/transition.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/module.h>

// If flag = 0, print all the in transitions
// If flag = 1, print all the out transitions
void print_all_transitions(struct transition* t, int flag){
	while(t){
		print_transition(t);
		if(flag){
			t = t->next_to_transition;
		} else{
			t = t->next_from_transition;
		}
	}
}
EXPORT_SYMBOL(print_all_transitions);

void print_transition( struct transition* t){
	if(t){
		pr_info("hypnosp: \t\t\t transition %x\n", t);

		pr_info("hypnosp: \t\t\t\t transition type %d\n", t->type);

		pr_info("hypnosp: \t\t\t\t transition from %x\n", t->from);

		pr_info("hypnosp: \t\t\t\t transition to %x\n", t->to);
	} else{
		pr_info("hypnosp: transition is null, exiting\n");
	}
}
EXPORT_SYMBOL(print_transition);

struct transition* add_transition( struct state* from, struct state* to, transition_type t){
	struct transition* trans;

	trans = (struct transition*) kmalloc(sizeof(struct transition), GFP_KERNEL);
	memset(trans, 0, sizeof(struct transition));
	if(from && to){
		trans->from = from;
		trans->to = to;
		trans->type = t;

		trans->next_from_transition = from->out;
		from->out =	trans;

		trans->next_to_transition = to->in;
		to->in = trans;
	}

	return trans;
}
EXPORT_SYMBOL(add_transition);

void set_timeout( struct transition* trans, int timeout ){
	if(trans){
		if(trans->type == TIMEOUT ){
			//pr_info("hypnos: timeout set %d on transition %x\n", timeout, trans);
			trans->timeout = timeout;
		} else{
			pr_info("hypnos: attempt to set timeout on transition of type exitting %d\n", trans->type);
		}
	}
}
EXPORT_SYMBOL(set_timeout);
