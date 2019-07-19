//
//  bounded_buffer.cpp
//  
//
//  Created by Joshua Higginbotham on 11/4/15.
//
//

#include "bounded_buffer.h"
#include <string>
#include <queue>

using namespace std;

bounded_buffer::bounded_buffer(int _capacity) {
	mux = new Semaphore(1);
	full = new Semaphore(0);
	empty = new Semaphore(_capacity);
}
bounded_buffer::~bounded_buffer() {
	delete full, empty;
}

void bounded_buffer::push_back(string str) {
	empty->P();
	mux->P();
	b_buffer.push(str);
	mux->V();
	full->V();
}

string bounded_buffer::retrieve_front() {
	full->P();
	mux->P();
	string str = b_buffer.front();
	b_buffer.pop();
	mux->V();
	empty->V();
	return str;
}

