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