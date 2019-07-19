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
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>

#include <errno.h>

#include "reqchannel.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
/*

*/
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* PRIVATE METHODS FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/



RequestChannel::RequestChannel(const char* port_, void * (*connection_handler)(void *), int backlog){
	//server
	struct addrinfo *serv;
    struct addrinfo serverInput;
	struct sockaddr_storage their_addr; 
	socklen_t sin_size;
	
	char s[INET_ADDRSTRLEN];
    int addrval;
	
	
	memset(&serverInput, 0, sizeof serverInput);
    
	
	serverInput.ai_family = AF_UNSPEC;
    serverInput.ai_socktype = SOCK_STREAM;
    serverInput.ai_flags = AI_PASSIVE; 
	
	if((addrval = getaddrinfo(NULL, port_, &serverInput, &serv)) != 0){
       cout << "ERROR:" << gai_strerror(addrval) << endl;
    }
	if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1){
		cout << "ERROR: making socket\n";
    }
	if (::bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1){
		close(sockfd);
		cout << "ERROR: binding socket\n";
		exit(1);
	}
	
	freeaddrinfo(serv);
	
	if (listen(sockfd, backlog) == -1)
	{
		cout << "ERROR: listening\n";
        exit(1);
    }

	cout << "Waiting for connections...\n";
	
	char buf [1024];
	bool first = true; 
	while(true){
		sin_size = sizeof(their_addr);
		
		pthread_t threadID;
		
		other_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		
		int *newInt = new int(other_fd);
		
		if(other_fd == -1){
			continue;
		}
		else if(other_fd != -1 and first){
			first = false;
		}
		
		cout << *newInt << endl;
		
		pthread_create(&threadID, NULL, connection_handler, newInt);
	}
	
}
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/


RequestChannel::RequestChannel(const string server_name, const char* _port){
	//client
    struct addrinfo serverInput;
    struct addrinfo *res;

	memset(&serverInput, 0, sizeof(serverInput));
    serverInput.ai_family = AF_INET;
    serverInput.ai_socktype = SOCK_STREAM;    //tcp stream sockets
   
    int status;
	
	cout<<"connected to server "<< server_name.c_str()<<endl;
    if((status = getaddrinfo(server_name.c_str(), _port, &serverInput, &res)) != 0){
		cout << "ERROR:" << gai_strerror(status) << endl;
    }
    
    //creat socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd < 0){
        cout << "ERROR: making socket\n";
    }
    //socket connect
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) < 0){
       cout << "ERROR: binding socket\n";
    }
}

RequestChannel::~RequestChannel() {
  close(sockfd);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;


string RequestChannel::send_request(string _request){
    cwrite(_request);
    string s = cread();
    return s;
}

string RequestChannel::cread() {
  char buff[1024];
  if (recv(sockfd, buff, sizeof(buff), 0) < 0){											
	cout << "\nERROR: reading\n";
	return NULL;
  } 
  
	string s = buff;
	return s;
}

int RequestChannel::cwrite(string _msg) {

  if (_msg.length() >= MAX_MESSAGE) {
    cerr << "Message too long!\n";
    return -1;
  }


  const char * s = _msg.c_str();

  if(send(sockfd, s, strlen(s)+1, 0) == -1) {
	cout << "ERROR: writing\n";
  }
  
  return _msg.length();

}



