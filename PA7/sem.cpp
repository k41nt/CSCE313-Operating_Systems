#include "sem.h"

int sem::p(){
	int lockerr;
	int conderr;
	
	if(lockerr = pthread_mutex_lock(&mu) != 0)
	{
		return lockerr;
	}
	
	while(count <= 0){
		if(conderr = pthread_cond_wait(&q, &mu) != 0)
		{
			return conderr;
		}
	}
	--count;
	
	if(lockerr = pthread_mutex_unlock(&mu) != 0)
	{
		return lockerr;
	}
	
	return 0;
}

int sem::v(){
	int lockerr;
	int conderr;
	
	if(lockerr = pthread_mutex_lock(&mu) != 0)
	{
		return lockerr;
	}
	
	++count;
	
	if(conderr = pthread_cond_broadcast(&q) != 0)
	{
		return conderr;
	}
	
	if(lockerr = pthread_mutex_unlock(&mu) != 0)
	{
		return lockerr;
	}
	
	return 0;
}
