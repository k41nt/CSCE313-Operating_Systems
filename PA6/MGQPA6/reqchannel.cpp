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

#include "reqchannel.h"

int RequestChannel::chan_count = 0;
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

struct my_msgbuf {
	long mtype;
	char mtext[200];
};

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

	RequestChannel::chan_count++;
	key_t clientKey = ftok ("a.txt", RequestChannel::chan_count); // create a pseudo-random key
	key_t serverKey = ftok ("b.txt", RequestChannel::chan_count); // create a pseudo-random key


	serverID = msgget(clientKey, 0644 | IPC_CREAT); // create the msg queue
	clientID = msgget(serverKey, 0644 | IPC_CREAT); // create the msg queue

	if (serverID < 0){
		perror ("message queue error");	
	}

	if (clientID < 0){
		perror ("message queue error");	
	}
}

RequestChannel::~RequestChannel() {


	msgctl(serverID, IPC_RMID, NULL);
	msgctl(clientID, IPC_RMID, NULL);
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


	struct my_msgbuf server_buf;
	struct my_msgbuf client_buf;


	if (side_name=="SERVER"){
		//int serverID = msgget(serverKey, 0644| IPC_CREAT); // connect to the msg queueif (msqid == -1)
		if (serverID < 0){
			perror ("message queue error");		
		}
		int server_read_return_value = msgrcv(clientID, &server_buf, sizeof(server_buf.mtext), 0, 0);
    	if (server_read_return_value<= 0) {
        	perror("msgrcv");
        	exit(1);
    	}
		std::string s1 = server_buf.mtext;
		return s1;
	}

	if (side_name=="CLIENT"){
		//int clientID = msgget(clientKey, 0644| IPC_CREAT); // connect to the msg queueif (msqid == -1)
		if (clientID < 0){
			perror ("message queue error");
		}
		int client_read_return_value = msgrcv(serverID, &client_buf, sizeof(client_buf.mtext), 0, 0);
    	if (client_read_return_value<= 0) {
        	perror("msgrcv");
        	exit(1);
    	}
		std::string s2 = client_buf.mtext;
		return s2;
	}

}

int RequestChannel::cwrite(std::string _msg) {



	struct my_msgbuf server_buf;
	struct my_msgbuf client_buf;

	server_buf.mtype = 1;
	client_buf.mtype = 1;

	int server_write_return_value;
	int client_write_return_value;


	if (side_name=="SERVER"){
		//int serverID = msgget(serverKey, 0644 | IPC_CREAT); // create the msg queue
		const char * s1 = _msg.c_str();
		//std::cout << _msg << std::endl;
		strcpy(server_buf.mtext, s1);	
		server_write_return_value = msgsnd(serverID, &server_buf, strlen(s1)+1 , 0);
		return server_write_return_value;
	}

	if (side_name=="CLIENT"){
		//int clientID = msgget(clientKey, 0644 | IPC_CREAT); // create the msg queue
		const char * s2 = _msg.c_str();
		//std::cout << _msg << std::endl;
		strcpy(client_buf.mtext, s2);		
		client_write_return_value = msgsnd(clientID, &client_buf, strlen(s2)+1 , 0);
		return client_write_return_value;
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



