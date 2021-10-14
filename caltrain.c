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
    int waiting; //正在等车的人
	int boarding;//正在上车但是还没坐稳的人...这也算本题特色了吧
	int seated;//已经坐稳的人
	int free_seats;//车上的空位置
    struct lock mutex;
    struct condition you_may_board;
    struct condition train_may_leave;
};

void station_init(struct station *station) {
    station->waiting = 0;
    station->free_seats = 0;
    station->boarding = 0;
    station->seated = 0;
    lock_init(&station->mutex);
    cond_init(&station->you_may_board);
    cond_init(&station->train_may_leave);
}

void station_load_train(struct station *station, int count) {
	//火车线程
    lock_acquire(&station->mutex);
	//描述上车之前的状态
    station->boarding = 0;
    station->seated = 0;
    station->free_seats = count;
	//通知所有乘客车来了, 它们都有同等的机会能上车
    cond_broadcast(&station->you_may_board, &station->mutex);
	//直到没有乘客上车了,或者才能开车
    while (station->seated < station->free_seats && (station->waiting > 0 || station->boarding > 0)){
        cond_wait(&station->train_may_leave, &station->mutex);
	}
	//火车开车了
    station->free_seats = 0;
    station->seated = 0;
    lock_release(&station->mutex);
}

void station_wait_for_train(struct station *station) {
	//乘客上车
    lock_acquire(&station->mutex);
    station->waiting++;
	//只有火车来了才能上车
    while (station->boarding + station->seated >= station->free_seats){
		cond_wait(&station->you_may_board, &station->mutex);
	}
    station->boarding++;
    station->waiting--;
    lock_release(&station->mutex);
}

void station_on_board(struct station *station) {
	//乘客在车上找座位
    lock_acquire(&station->mutex);
    station->seated++;
    station->boarding--;
	//通知火车"我已经坐稳了"
    cond_signal(&station->train_may_leave, &station->mutex);
    lock_release(&station->mutex);
}
