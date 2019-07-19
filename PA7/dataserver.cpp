/* 
    File: dataserver.C
    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/16
    Dataserver main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <getopt.h>

#include "reqchannel.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

int MAXM = 255;

int backlog;


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string intToString(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to ss
   return ss.str();//return result
}




/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/


void process_hello(RequestChannel * requestchan, const string & _request) {
  requestchan->cwrite("hello to you too");
}

void process_data(RequestChannel * requestchan, const string &  _request) {
  usleep(1000 + (rand() % 5000));
  requestchan->cwrite(intToString(rand() % 100));
}


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(RequestChannel * requestchan, const string & _request) {
  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(requestchan, _request);
  }
  else if (_request.compare(0, 4, "data") == 0){
	process_data(requestchan, _request);
  }

}

void * connection_handler(void * arg){//RequestChannel * arg)
	char buf[1024];
    RequestChannel *chan = (RequestChannel*)arg;
    
	while(true) {
        recv(chan->read_socket(), buf, 1024,0);
        string request = buf;
        if (request.compare("quit") == 0) {
			chan->cwrite("bye");
            usleep(1000);
            break;
        }
		process_request(chan, request);
    }
	
    return 0;
}
/*

*/

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    const char* portNum = "2909"; //port for server
    string host = "localhost";
    
    int c = 0;
	int backlog;
    while((c = getopt(argc, argv, "p:b:")) != -1){
        switch(c){
            case 'b':
                backlog = atoi(optarg);
            case 'p':
                portNum = optarg;
                break;
            case '?':
            default:
				backlog = 10;
                cout<<"use the -p flag to change the port number.\n";
                exit(0);
        }
    }
    cout<<"Server: "<<host<<endl;
    cout<<"Port: "<<portNum<<endl;
    RequestChannel server(portNum, connection_handler, backlog);
}
