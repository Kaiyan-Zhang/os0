#include "pintos_thread.h"

void make_water();

struct reaction {
    int needed_h, used_h;
    struct lock mutex;
    struct condition now_o;
    struct condition now_h;
};

void reaction_init(struct reaction *reaction) {
    reaction->needed_h = 0;
    reaction->used_h = 0;
    lock_init(&reaction->mutex);
    cond_init(&reaction->now_o);
    cond_init(&reaction->now_h);
}

void reaction_h(struct reaction *reaction) {
    lock_acquire(&reaction->mutex);
    reaction->needed_h++;
    cond_signal(&reaction->now_h, &reaction->mutex);
    while (reaction->used_h == 0){
		cond_wait(&reaction->now_o, &reaction->mutex);
	}
    reaction->used_h--;
    lock_release(&reaction->mutex);
}

void reaction_o(struct reaction *reaction) {
    lock_acquire(&reaction->mutex);
    while (reaction->needed_h < 2){
		cond_wait(&reaction->now_h, &reaction->mutex);
	}
    reaction->used_h += 2;
    reaction->needed_h -= 2;
    cond_broadcast(&reaction->now_o, &reaction->mutex);
    make_water();
    lock_release(&reaction->mutex);
}