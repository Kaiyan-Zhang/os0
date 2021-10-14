#include "pintos_thread.h"

/*
==== 先读懂它在封装些什么 ====

初始化这个锁
lock_init(struct lock *lock) __attribute__((unused));

上锁
lock_acquire(struct lock *lock) __attribute__((unused));

解锁
lock_release(struct lock *lock) __attribute__((unused));

初始化这个条件
cond_init(struct condition *cond) __attribute__((unused));

消费者在锁内等待一个条件满足
cond_wait(struct condition *cond, struct lock *lock) __attribute__((unused));

生产者在锁内通知另一个消费者条件可能已经满足
cond_signal(struct condition *cond, struct lock *lock) __attribute__((unused));

生产者在锁内通知所有消费者条件可能已经满足
cond_broadcast(struct condition *cond, struct lock *lock) __attribute__((unused));
*/

struct station {
	// 根据caltrain-runner.c,应该是有1000个乘客
    int waiting;
	int seats;
	int boarding;
	int boarded;
    struct lock lock;
    struct condition train_available;
    struct condition train_to_leave;
};

void station_init(struct station *station) {
	// main函数的开头
    station->waiting = 0;
    station->seats = 0;
    station->boarding = 0;
    station->boarded = 0;
    lock_init(&station->lock);
    cond_init(&station->train_available);
    cond_init(&station->train_to_leave);
}

void station_load_train(struct station *station, int count) {
	//或者线程
	//count大概描述的是free_seats
    lock_acquire(&station->lock);
    station->boarding = 0;
    station->boarded = 0;
    station->seats = count;
    cond_broadcast(&station->train_available, &station->lock);
    while (station->boarded < station->seats && (station->waiting > 0 || station->boarding > 0)){
        cond_wait(&station->train_to_leave, &station->lock);
	}
    station->seats = 0;
    station->boarded = 0;
    lock_release(&station->lock);
}

void station_wait_for_train(struct station *station) {
	//乘客线程
    lock_acquire(&station->lock);
    station->waiting++;
    while (station->boarding + station->boarded >= station->seats){
		cond_wait(&station->train_available, &station->lock);
	}
    station->boarding++;
    station->waiting--;
    lock_release(&station->lock);
}

void station_on_board(struct station *station) {
	// FILL ME IN
    lock_acquire(&station->lock);
    station->boarded++;
    station->boarding--;
    cond_signal(&station->train_to_leave, &station->lock);
    lock_release(&station->lock);
}
