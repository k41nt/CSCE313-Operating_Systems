
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <assert.h>
#include <fstream>
#include <numeric>
#include <vector>
#include "reqchannel.h"
#include "Bounded_buffer.h"

using namespace std;
/*
    This next file will need to be written from scratch, along with
    semaphore.h and (if you choose) their corresponding .cpp files.
 */



/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*
    All *_params structs are optional,
    but they might help.
	
	
 */
const char* port = "2909"; 
string hostName = "localhost";
 
 
struct request_thread_params {
    Bounded_buffer *request_Buffer;//Main Bounded buffer

	int *johnFrequencyCount;//frequncy count of John responses
	int *janeFrequencyCount;//frequncy count of Jane responses
	int *joeFrequencyCount;//frequncy count of Joe responses

	int *numRequests;//number of total requests
	int requestID;//ID of request

	RequestChannel *channel;//Request channel used


};

struct event_thread_params {//for event thread


	Bounded_buffer *john_Buffer;
	Bounded_buffer *joe_Buffer;
	Bounded_buffer *jane_Buffer;

	int n;//number of requests
	int w;//number of threads

	request_thread_params* buffLink;//daisy chains to request_thread_params, to use bounded buffer
};

struct stat_thread_params {
    std::vector<int> * johnFrequency;
	std::vector<int> * janeFrequency;
	std::vector<int> * joeFrequency;

	int* n;
	int rid;

	event_thread_params * workLink;//daisy chains to  worker_thread_params, to use request_thread_params
};

/*
    This class can be used to write to standard output
    in a multithreaded environment. It's primary purpose
    is printing debug messages while multiple threads
    are in execution.
 */
class atomic_standard_output {
    pthread_mutex_t console_lock;
public:
    atomic_standard_output() { pthread_mutex_init(&console_lock, NULL); }
    ~atomic_standard_output() { pthread_mutex_destroy(&console_lock); }
    void print(std::string s){
        pthread_mutex_lock(&console_lock);
        std::cout << s << std::endl;
        pthread_mutex_unlock(&console_lock);
    }
};

atomic_standard_output threadsafe_standard_output;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS */
/*--------------------------------------------------------------------------*/

string make_histogram_table(string name1, string name2,
        string name3, vector<int> *data1, vector<int> *data2,
        vector<int> *data3) {
	stringstream tablebuilder;
	tablebuilder << setw(25) << right << name1;
	tablebuilder << setw(15) << right << name2;
	tablebuilder << setw(15) << right << name3 << endl;
	for (int i = 0; i < data1->size(); ++i) {
		tablebuilder << setw(10) << left
		        << string(
		                to_string(i * 10) + "-"
		                        + to_string((i * 10) + 9));
		tablebuilder << setw(15) << right
		        << to_string(data1->at(i));
		tablebuilder << setw(15) << right
		        << to_string(data2->at(i));
		tablebuilder << setw(15) << right
		        << to_string(data3->at(i)) << endl;
	}
	tablebuilder << setw(10) << left << "Total";
	tablebuilder << setw(15) << right
	        << accumulate(data1->begin(), data1->end(), 0);
	tablebuilder << setw(15) << right
	        << accumulate(data2->begin(), data2->end(), 0);
	tablebuilder << setw(15) << right
	        << accumulate(data3->begin(), data3->end(), 0) << endl;

	return tablebuilder.str();
}

/*
    You'll need to fill these in.
*/
void* request_thread_function(void* arg) {
    request_thread_params p = *(request_thread_params*)arg;
	for(int i = 0; i < *p.numRequests; i++){
		Response* r = new Response("none", p.requestID, 0);
		switch(p.requestID){
			case 0:
				*(p.johnFrequencyCount) += 1;//future reference not ++, +=1
				r->serverResp = "data John Smith";
				r->requestID = 0;
				r->count = *p.johnFrequencyCount;
				break;
			case 1:
				*(p.janeFrequencyCount) += 1;
				r->serverResp = "data Jane Smith";
				r->requestID = 1;
				r->count = *p.janeFrequencyCount;
				break;
			case 2:
				*(p.joeFrequencyCount) += 1;
				r->serverResp = "data Joe Smith";
				r->requestID = 2;
				r->count = *p.joeFrequencyCount;
				break;
		}
		p.request_Buffer->addResponse(*r);
		delete r;
	}
	pthread_exit((void*) arg);

}

void* event_handler(void* arg) {
	event_thread_params p = *(event_thread_params*)arg;
	fd_set rfds;

	int personID [p.w];//pid array

	int wCount = 0;//channel count
	int rCount = 0;//response count
	int maxFD = 0;//max fd of fdset
	int selectResult; //result of select function

	struct timeval tStruct = {0,10};//time struct for select function

	vector<RequestChannel*> channels;

	Response r = Response("",0,0);//dummy request


	for(int i = 0; i < p.w; i++){
		//create request channels
		RequestChannel* channel = new RequestChannel(hostName.c_str(), port);
		channels.push_back(channel);
		personID[i] = -1;
	}

	for(int i = 0; i < p.w; i++){
		//fill request channels with 1 request each
		r = p.buffLink->request_Buffer->getResponse();
		wCount++;
		personID[i] = r.requestID;
		channels[i]->cwrite(r.serverResp);
	}

	while(true){
		//evaluate requests
		FD_ZERO(&rfds);//initialize request channels
		for(int i = 0; i < p.w; i++){//resets fd_set max
			//cout << maxFD << endl;
			if(channels[i]->read_socket() > maxFD){
				maxFD = channels[i]->read_socket();
			}
			FD_SET(channels[i]->read_socket(), &rfds);//FD_SET command, needs to be done every iteration of while loop
		}
		
		selectResult = select(maxFD+1, &rfds, NULL, NULL,NULL);

		if(selectResult){//if select result is true
			for(int i = 0; i < p.w; i++){
				if(FD_ISSET(channels[i]->read_socket(), &rfds)){
					rCount++;
					string sResponse = channels[i]->cread();

					switch(personID[i]){
						case 0:
							p.john_Buffer->addResponse(Response(sResponse, 0, 0));
							break;
						case 1:
							p.jane_Buffer->addResponse(Response(sResponse, 1, 0));
							break;
						case 2:
							p.joe_Buffer->addResponse(Response(sResponse, 2, 0));
							break;
                    }

                    if(wCount < p.n*3){//different from response count conditional, writes server response to channel
	                    wCount++;
						r = p.buffLink->request_Buffer->getResponse();
	                    channels[i]->cwrite(r.serverResp);
	                    personID[i] = r.requestID;
                	}
				}
			}
		}
		if(rCount == p.n*3){
			break;
		}
	}

	for(int i = 0; i < p.w; i++){
		channels[i]->send_request("quit");
	}
	pthread_exit((void*) arg);
}

void* stat_thread_function(void* arg) {
    stat_thread_params p = *(stat_thread_params*)arg;
    
	Response r = Response("", -1, 0);
	for(int i = 0; i < *p.n; i++){
		if(p.rid == 0){
			r = p.workLink->john_Buffer->getResponse();
	 		p.johnFrequency->at(atoi(r.serverResp.c_str())/10) += 1;
	 	}
	 	else if(p.rid == 1){
			r = p.workLink->jane_Buffer->getResponse();
			p.janeFrequency->at(atoi(r.serverResp.c_str())/10) += 1;
		}
	 	else if(p.rid == 2){
			r = p.workLink->joe_Buffer->getResponse();
			p.joeFrequency->at(atoi(r.serverResp.c_str())/10) += 1;
	 	}
	}
	pthread_exit((void*) arg);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 10000; //default number of requests per "patient"
    int b = 50; //default size of request_buffer
    int w = 20; //default number of worker threads
	
	string temp = "";
    bool USE_ALTERNATE_FILE_OUTPUT = false;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:b:w:m:p:h")) != -1) {
        switch (opt) {
             case 'n':
                n = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 'w':
                w = atoi(optarg);
                break;
            case 'm':
                if(atoi(optarg) == 2) USE_ALTERNATE_FILE_OUTPUT = true;
                break;
            case 'p':
                port = optarg;
                break;
            case 'h':
                temp = optarg;
                if(temp != "")
                {
                    hostName = temp; 
                }
                break;
            case '?':
            default:
                std::cout << "This program can be invoked with the following flags:" << std::endl;
                std::cout << "-n [int]: number of requests per patient" << std::endl;
                std::cout << "-b [int]: size of request buffer" << std::endl;
                std::cout << "-w [int]: number of worker threads" << std::endl;
                std::cout << "-m 2: use output2.txt instead of output.txt for all file output" << std::endl;
                std::cout << "-h: print this message and quit" << std::endl;
                std::cout << "Example: ./client_solution -n 10000 -b 50 -w 120 -m 2" << std::endl;
                std::cout << "If a given flag is not used, a default value will be given" << std::endl;
                std::cout << "to its corresponding variable. If an illegal option is detected," << std::endl;
                std::cout << "behavior is the same as using the -h flag." << std::endl;
                exit(0);
        }
    }
	
	if (b < w){
		b = w;// increase buffer size to w   
	}


        ofstream ofs;
        if(USE_ALTERNATE_FILE_OUTPUT) ofs.open("output2.txt", ios::out | ios::app);
        else ofs.open("output.txt", ios::out | ios::app);

        std::cout << "n == " << n << std::endl;
        std::cout << "b == " << b << std::endl;
        std::cout << "w == " << w << std::endl;



////////////////////////////////////////////////////////////
/////////////CRITICAL SECTION///////////////////////////////
////////////////////////////////////////////////////////////

        int johnCount;
		int janeCount;
		int joeCount;
		pthread_t requestThreads[3];
		pthread_t eventThread;
		pthread_t statThreads[3];

		Bounded_buffer *request_buffer = new Bounded_buffer(b);
		Bounded_buffer *john_buffer = new Bounded_buffer(b);
		Bounded_buffer *jane_buffer = new Bounded_buffer(b);
		Bounded_buffer *joe_buffer = new Bounded_buffer(b);
		
		std::vector<int> john_frequency_count(10, 0);
        std::vector<int> jane_frequency_count(10, 0);
        std::vector<int> joe_frequency_count(10, 0);
		
        //john request parameters
		request_thread_params* johnPARAM = new request_thread_params;
		johnPARAM->request_Buffer = request_buffer;
		johnPARAM->johnFrequencyCount = &johnCount;
		johnPARAM->numRequests = &n;
		johnPARAM->requestID = 0;
        //jane request parameters
		request_thread_params* janePARAM = new request_thread_params;
		janePARAM->request_Buffer = request_buffer;
		janePARAM->janeFrequencyCount = &janeCount;
		janePARAM->numRequests = &n;
		janePARAM->requestID = 1;
        //joe request parameters
		request_thread_params* joePARAM = new request_thread_params;
		joePARAM->request_Buffer = request_buffer;
		joePARAM->joeFrequencyCount = &joeCount;
		joePARAM->numRequests = &n;
		joePARAM->requestID = 2;
		////Worker thread parameters
		event_thread_params* ep = new event_thread_params;
		ep->john_Buffer = john_buffer;
		ep->jane_Buffer = jane_buffer;
		ep->joe_Buffer = joe_buffer;
		ep->n = n;
		ep->w = w;
		
		//buffer link
		request_thread_params link;
		link.request_Buffer = request_buffer;
		//link.channel = chan;
		ep->buffLink = &link;

		event_thread_params sLink;
        sLink.john_Buffer = john_buffer;
        sLink.jane_Buffer = jane_buffer;
        sLink.joe_Buffer = joe_buffer;
		
		//stat thread parameters
		stat_thread_params * johnStat = new stat_thread_params;
		johnStat->johnFrequency = &john_frequency_count;
		johnStat->rid = 0;
		johnStat->workLink = &sLink;
		johnStat->n = &n;

		stat_thread_params * janeStat = new stat_thread_params;
		janeStat->janeFrequency = &jane_frequency_count;
		janeStat->rid = 1;
		janeStat->workLink = &sLink;
		janeStat->n = &n;

		stat_thread_params * joeStat = new stat_thread_params;
		joeStat->joeFrequency = &joe_frequency_count;
		joeStat->rid = 2;
		joeStat->workLink = &sLink;
		joeStat->n = &n;

		struct timeval start_time;
		struct timeval end_time;
    	int64_t start_usecs;
    	int64_t finish_usecs;
		gettimeofday(&start_time, NULL); //start timer

		////request threads
		pthread_create(&requestThreads[0], NULL, request_thread_function, (void*)johnPARAM);
		pthread_create(&requestThreads[1], NULL, request_thread_function, (void*)janePARAM);
		pthread_create(&requestThreads[2], NULL, request_thread_function, (void*)joePARAM);

		//event handler thread
		pthread_create(&eventThread, NULL, event_handler, (void*) ep);

		//create statistics threads
		pthread_create(&statThreads[0], NULL, stat_thread_function, (void*)johnStat);
		pthread_create(&statThreads[1], NULL, stat_thread_function, (void*)janeStat);
		pthread_create(&statThreads[2], NULL, stat_thread_function, (void*)joeStat);

		//join request threads
		pthread_join (requestThreads[0], NULL);
		pthread_join (requestThreads[1], NULL);
		pthread_join (requestThreads[2], NULL);
	
		//push quit
		Response quitResponse = Response("quit", -5, -5);
		for (int i = 0; i < w; i++) {
            request_buffer->addResponse(quitResponse);
        }
        
		//join event thread
		pthread_join (eventThread, NULL);
		//join stat threads
		pthread_join (statThreads[0], NULL);
		pthread_join (statThreads[1], NULL);
		pthread_join (statThreads[2], NULL);
		
		gettimeofday(&end_time, NULL); //end timer
		ofs.close();


	    cout << "Results for n == " << n << ", w == " << w << ", b == " << b  << endl;
	    string histogram_table = make_histogram_table("data John","data Jane","data Joe",&john_frequency_count,&jane_frequency_count, &joe_frequency_count);
	    cout << histogram_table << endl;
	    double total_time = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);        
	    cout << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << endl;
	    cout << "Sleeping..." << endl;
	    usleep(10000);

        //This is to output the time results into time_results.txt for easy time collection
        ofstream myfile;
        myfile.open ("test_results2.txt",std::ios_base::app);
        myfile << "Results for n == " << n << ", w == " << w << ", b == " << b  << std::endl;
        myfile << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << std::endl<< std::endl;
       
        usleep(10000);

}
