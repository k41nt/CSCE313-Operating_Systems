#include <mutex>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <string>
#include "semaphore.h"

#ifndef _bounded_buffer_H_
#define _bounded_buffer_H_


class bounded_buffer {
	Semaphore *full, *empty, *mux;
	queue<std::string> b_buffer;

public:
    bounded_buffer(int _capacity);
    ~bounded_buffer();
    void push_back(std::string str);
    std::string retrieve_front();
    //int size();
};

#endif /* bounded_buffer_h */