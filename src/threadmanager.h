#ifndef __THREADMANAGER_H__
#define __THREADMANAGER_H__

void tm_init();
void tm_destroy();
int tm_queue_thread(void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg);

#endif