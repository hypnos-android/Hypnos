#include <linux/fsm.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/module.h>

struct fsm* fsm_head;
static struct state* get_base_state(void){
	struct state* base_state;
	base_state = (struct state*) kmalloc(sizeof(struct state), GFP_KERNEL);
	memset(base_state, 0, sizeof(struct state));
	return base_state;
}

struct fsm* new_fsm(cmd c){
	struct fsm* f;
	struct state* s;

	f = (struct fsm*) kmalloc(sizeof(struct fsm), GFP_KERNEL);
	s = get_base_state();

	memset(f, 0, sizeof(struct fsm));
	f->base_state = s;
	f->current_state = f->base_state;
	f->c = c;
	return f;
}
EXPORT_SYMBOL(new_fsm);

void print_all_fsm( void ){
	struct fsm* iterator;
	iterator = fsm_head;
	pr_info("hypnosp: PRINTING ALL REGISTERED FSMs\n");
	while(iterator){
		print_fsm(iterator);
		iterator = iterator->next;
	}
}
EXPORT_SYMBOL(print_all_fsm);

void print_fsm( struct fsm* f ){
	struct state* state_iterator;
	pr_info("hypnosp: PRINTING FSM\n");
	if(f){
		pr_info("hypnosp: PRINTING STATES of fsm %x\n", f);
		if(f->base_state){
			state_iterator = f->base_state;
			while(state_iterator){
				print_state(state_iterator);
				state_iterator = state_iterator->next;
			}
		} else{
			pr_info("hypnosp: fsm's base state is null, exiting\n");
		}
	} else{
		pr_info("hypnosp: fsm is null, exiting\n");
	}
}
EXPORT_SYMBOL(print_fsm);

int register_fsm (struct fsm* f){
	f->next = fsm_head;
	fsm_head = f;
	print_all_fsm();
	return 0;
}
EXPORT_SYMBOL(register_fsm);


int unregister_fsm (struct fsm* f){
	struct fsm* iterator;

	// Special case when f is fsm_head
	if(fsm_head == f){
		fsm_head = fsm_head->next;
		return 0;
	}

	iterator = fsm_head;
	while(iterator && iterator->next != f){
		iterator = iterator->next;
	}
	if(iterator){
		iterator->next = f->next;
		return 0;
	}
	else
		return NO_SUCH_FSM;
}
EXPORT_SYMBOL(unregister_fsm);

struct state* add_state( struct fsm* f, int power ){
	struct state* s;
	struct state* iterate;
	
	iterate = f->base_state;
	s = (struct state*) kmalloc(sizeof(struct state), GFP_KERNEL);
	memset(s, 0, sizeof(struct state));

	if( iterate->power == power ){
		pr_info("hypnos: Two states with same power!! %d\n" , power);
	}

	while( iterate->next ){
		if( iterate->power == power )
			pr_info("hypnos: Two states with same power!! %d\n" , power);
		iterate = iterate->next;
	}
	s->power = power;
	iterate->next = s;
	return s;
}
EXPORT_SYMBOL(add_state);
