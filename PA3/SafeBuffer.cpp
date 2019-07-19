//
//  SafeBuffer.cpp
//
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#include "SafeBuffer.h"

SafeBuffer::SafeBuffer() {
	pthread_mutex_init(&m, NULL);
}

SafeBuffer::~SafeBuffer() {
	pthread_mutex_destroy(&m);
}

int SafeBuffer::size() {
    int size = myqueue.size();
	return size;
}

void SafeBuffer::push_back(std::string str) {
	pthread_mutex_lock(&m);
	myqueue.push(str);
	pthread_mutex_unlock(&m);
}

std::string SafeBuffer::retrieve_front() {
	pthread_mutex_lock(&m);
	std::string front = myqueue.front();
	myqueue.pop();
	pthread_mutex_unlock(&m);
	return front;
}
