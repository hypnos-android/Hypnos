#include<linux/state.h>
#include<linux/fsm.h>
#include<linux/module.h>

void print_state( struct state* s){
	pr_info("hypnosp: \t state %x\n", s);

	pr_info("hypnosp: \t \t power %u\n", s->power);

	pr_info("hypnosp: \t \t in trans \n");
	print_all_transitions(s->in, 0);

	pr_info("hypnosp: \t \t out trans\n");
	print_all_transitions(s->out, 1);
}
EXPORT_SYMBOL(print_state);

struct state* search_state(struct fsm* f, unsigned power){
	struct state* iterate;
	iterate = f->base_state;
	while(iterate){
		if(iterate->power == power)
			break;
		iterate = iterate->next;
	}
	if(iterate->power == power){
		return iterate;
	}
	return 0;
}
EXPORT_SYMBOL(search_state);
