#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

class sem{
	int count;
	pthread_mutex_t mu;
	pthread_cond_t q;
public:
	sem(int val){
		pthread_mutex_init(&mu, NULL);
		pthread_cond_init(&q, NULL);
		count = val;
	};
	~sem(){
		pthread_mutex_destroy(&mu);
		pthread_cond_destroy(&q);
	};
	int p();//
	int v();//
};
