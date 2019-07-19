#include <iostream>
#include <pthread.h>
#include <stdio.h>

using namespace std;

class Semaphore {
	int count;
	pthread_mutex_t mux;
    pthread_cond_t q;
public:
	Semaphore(int c)
		:count(c) {
			pthread_mutex_init(&mux, NULL);
        	pthread_cond_init(&q, NULL);
		}

	~Semaphore() {
		pthread_mutex_destroy(&mux);
        pthread_cond_destroy(&q);
	}

	void P() {
		pthread_mutex_lock(&mux);
		count--;
		if (count < 0) {
			pthread_cond_wait(&q, &mux);
		}
		pthread_mutex_unlock(&mux);
	}

	void V() {
		pthread_mutex_lock(&mux);
		count++;
		if (count <= 0) {
			pthread_cond_signal(&q);
		}
		pthread_mutex_unlock(&mux);
	}
};
