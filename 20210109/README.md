姓名: 张开颜
学号: 20210109

通过这次实验我收获了很多, 也走了很多弯路。
我后来才意识到本次实验是多生产者多消费者模式,
而不是1生产者1消费者模式, 然后我才把最外面的while循环去掉。
本次实验最大的收获是巩固了mutex和cond的相关知识。

### 本次实验最大的经验
在写代码之前, 先用自然语言描述整个事件的过程, 
对于有限资源, 分配一个`mutex`, 
对于事件先后, 对每一个"通知"动作分配一个`cond`。
例如, 司机应该通知乘客可以上车了, 乘客也应该通知司机自己已经坐稳了。
用"通知"来完成同步操作是`mutex`和`semaphore`在代码编写上的主要区别。

### 本次实验最大的亮点
在做`reaction`实验的时候, 我意识到这也是一个多生产者多消费者的问题, 
因此我将氧原子看成"司机", 将氢原子看成"乘客", 将一个水分子看成"一辆车",
这样就将两道题当成了一道题来做。
具体可看代码中描述。

# 实验1: 托马斯小火车
```c
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

//火车线程, 只要乘客还有, 这样的线程就会不停地被创建
void station_load_train(struct station *station, int count) {
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

//乘客上车
void station_wait_for_train(struct station *station) {
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
```

# 实验2: 化学实验室
```c
#include "pintos_thread.h"

/*
 * 分析:
 * 将反应看成一辆车, 车上只有3个座位, o是司机, h是乘客
 * 一个司机带两个乘客
 * 
 * 过程:
 * 司机等待两个乘客 -> 乘客通知自己已经上车 -> 司机开车 -> 司机通知其他乘客等下一辆车
 * 
 * 代码:
 * 用两个cond模拟两个"通知"
 * 用一个mutex模拟车
 */

void make_water();

struct reaction {
	int seated_passenger; //已落座的乘客
	struct condition have_new_passenger; //有新的乘客上车了
	struct condition car_leaved;
	struct lock mutex;
};

void reaction_init(struct reaction *reaction) {
	reaction->seated_passenger = 0;
	cond_init(&reaction->have_new_passenger);
	cond_init(&reaction->car_leaved);
	lock_init(&reaction->mutex);
}

//将h视为乘客
void reaction_h(struct reaction *reaction) {
	lock_acquire(&reaction->mutex);
	//上车
	reaction->seated_passenger += 1;
	//通知司机: "我已经上车了"
	cond_signal(&reaction->have_new_passenger, &reaction->mutex);
	//等待开车
	cond_wait(&reaction->car_leaved, &reaction->mutex);
	//如果车开了, 则结束自己这个线程, 代表乘客"已死"
	lock_release(&reaction->mutex);
}

//将o视为司机
void reaction_o(struct reaction *reaction) {
	lock_acquire(&reaction->mutex);
	//司机等待2个乘客上车
	while(reaction->seated_passenger <= 1) cond_wait(&reaction->have_new_passenger, &reaction->mutex);
	//开车
	make_water();
	reaction->seated_passenger -= 2;
	//通知另外2个乘客: "你们等下一辆车吧"
	cond_signal(&reaction->car_leaved, &reaction->mutex);
	cond_signal(&reaction->car_leaved, &reaction->mutex);
	lock_release(&reaction->mutex);
}
```
