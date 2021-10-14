These two files, and only these two files, should be submitted for grading.
No other files should need to be modified.
- caltrain.c
- reaction.c

```c
struct lock
struct condition
void lock_init(struct lock *lock);
void lock_acquire(struct lock *lock);
void lock_release(struct lock *lock);
void cond_init(struct condition *cond);
void cond_wait(struct condition *cond, struct lock *lock);
void cond_signal(struct condition *cond, struct lock *lock);
void cond_broadcast(struct condition *cond, struct lock *lock);
```

Compile using 'make'

To run a bunch of tests, you can try 'make run'.

The tests have a lot of randomness built in and aren't flawless (their
passing doesn't guarantee a perfect implementation).

You may want to run many times to get better assurance of your solution's sanity.
