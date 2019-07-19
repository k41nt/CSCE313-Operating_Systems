#include <queue>
#include "semaphore.h"

template <class T> class bounded_buffer 
{
public:
	bounded_buffer(int _capacity) : full(0), empty(_capacity), mux(1){}
		
	void push_back(T str);
	T retrieve_front();
	int size();

private:
	Semaphore full, empty, mux;
	std::queue<T> b_buffer;
};




template <class T> void bounded_buffer<T>::push_back(T str)
{
	empty.P(); 
	mux.P(); 
	b_buffer.push(str);
	mux.V();
	full.V();
}


template <class T> T bounded_buffer<T>::retrieve_front()
{
	full.P();
	mux.P(); 
	T str = b_buffer.front();
	b_buffer.pop();
	mux.V();
	empty.V();
	return str;
}
