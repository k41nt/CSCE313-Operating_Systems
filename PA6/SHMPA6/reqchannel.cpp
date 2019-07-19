/* 
    File: requestchannel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/11

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#include "reqchannel.h"

int RequestChannel::chan_count = 0;
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

const bool VERBOSE = false;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* PRIVATE METHODS FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

RequestChannel::RequestChannel(const std::string _name, const Side _side) :
my_name(_name), my_side(_side), side_name((_side == RequestChannel::SERVER_SIDE) ? "SERVER" : "CLIENT")
{
	full_1 = new KernelSem (0,my_name+"full1.txt", my_side);
	full_2 = new KernelSem (0,my_name+"full2.txt",my_side);
	empty_1 = new KernelSem (1,my_name+"empty1.txt",my_side);
	empty_2 = new KernelSem (1,my_name+"empty2.txt",my_side);

	RequestChannel::chan_count++;
	key_t clientKey = ftok ("a.txt", RequestChannel::chan_count); // create a pseudo-random key
	key_t serverKey = ftok ("b.txt", RequestChannel::chan_count); // create a pseudo-random key

	serverID = shmget(clientKey, SHM_SIZE, 0644 | IPC_CREAT); // connecting to the segment
	clientID = shmget(serverKey, SHM_SIZE, 0644 | IPC_CREAT); // connecting to the segment

	client_data = (char*) shmat(clientID, (void*) 0, 0);
	server_data = (char*) ((unsigned long)client_data + ((unsigned long) SHM_SIZE));// this is the bonus: cast the server_data to the same memory segment of client_data

	if (serverID < 0){
		perror ("shmget error");	
	}

	if (clientID < 0){
		perror ("shmget error");	
	}
}


RequestChannel::~RequestChannel() {
	shmdt(client_data);
	shmdt(server_data);
	delete full_1;
	delete empty_1;
	delete full_2;
	delete empty_2;
	shmctl(clientID,IPC_RMID,NULL);
	shmctl(serverID,IPC_RMID,NULL);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;

std::string RequestChannel::send_request(std::string _request) {
	pthread_mutex_lock(&send_request_lock);
	if(cwrite(_request) < 0) {
		pthread_mutex_unlock(&send_request_lock);
		return "ERROR";
	}
	std::string s = cread();
	pthread_mutex_unlock(&send_request_lock);
	return s;
}

std::string RequestChannel::cread() {

	if (side_name=="SERVER"){
		full_1->P();
		const char* temp1 = client_data;
		//std::cout<<"SERVER: Read from segment: "<< temp1<<std::endl;
		empty_1->V();
		return temp1;
	}

	if (side_name=="CLIENT"){
		full_2->P();
		const char* temp2 = server_data;
		//std::cout<<"CLIENT: Read from segment: "<< temp2<<std::endl<<std::endl;
		empty_2->V();
		return temp2;
	}

}

int RequestChannel::cwrite(std::string _msg) {
	if (side_name=="SERVER"){
		empty_2->P();
		strcpy(server_data, _msg.c_str());	
		//std::cout<<"SERVER: Write to segment: "<<server_data<<std::endl;
		full_2->V();
	}

	if (side_name=="CLIENT"){
		empty_1->P();
		strcpy(client_data, _msg.c_str());	
		//std::cout<<"CLIENT: Write to segment: "<<client_data<<std::endl;
		full_1->V();
	}

}

/*--------------------------------------------------------------------------*/
/* ACCESS THE NAME OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

std::string RequestChannel::name() {
	return my_name;
}

/*--------------------------------------------------------------------------*/
/* ACCESS FILE DESCRIPTORS OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/

int RequestChannel::read_fd() {
	return rfd;
}

int RequestChannel::write_fd() {
	return wfd;
}



