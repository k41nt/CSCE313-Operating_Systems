/*
    File: client.cpp

    Author: J. Higginbotham
    Department of Computer Science
    Texas A&M University
    Date  : 2016/05/21

    Based on original code by: Dr. R. Bettati, PhD
    Department of Computer Science
    Texas A&M University
    Date  : 2013/01/31
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */
    /* -- This might be a good place to put the size of
        of the patient response buffers -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*
    No additional includes are required
    to complete the assignment, but you're welcome to use
    any that you think would help.
*/
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <signal.h>
#include "reqchannel.h"
#include "bounded_buffer.h"
using namespace std;
/*
    This next file will need to be written from scratch, along with
    semaphore.h and (if you choose) their corresponding .cpp files.
 */

//#include "bounded_buffer.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*
    All *_params structs are optional,
    but they might help.
 */


struct PARAMS_stat {
	bounded_buffer<string> *buf; 
	vector<int> *count;                 
};

struct req {
	string name;                            
	bounded_buffer<string> *buf; 
};

struct PARAMS_handler {
	bounded_buffer<req> *request_buffer; 
	RequestChannel *chan;                  
	int tot_rq;                   
	int num_chan;                    
};

struct PARAMS_request {
	bounded_buffer<req> *request_buffer; 
	int num_rq;                       
	req r;                              
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
    void print(string s){
        pthread_mutex_lock(&console_lock);
        cout << s << endl;
        pthread_mutex_unlock(&console_lock);
    }
};

atomic_standard_output aso;


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
	PARAMS_request info = *(PARAMS_request *)arg;
	bounded_buffer<req> *request_buffer = info.request_buffer;
	req r = info.r; 
	for(int i=0; i< info.num_rq; i++) {
		request_buffer->push_back(r);
	}
	pthread_exit(NULL);
}

void* stat_thread_function(void* arg) {

	PARAMS_stat stat = *(PARAMS_stat *)arg;
	bounded_buffer<string> *buf = stat.buf;
	vector<int> *count = stat.count;
   	for(;;) {
		string response = buf->retrieve_front();
		if(response.compare("quit") == 0) 
			break;
		count->at(stoi(response) / 10) += 1;
	}
    pthread_exit(NULL);
}


void* event_handler_function(void* arg) {
	PARAMS_handler x = *(PARAMS_handler *)arg;
	bounded_buffer<req> *request_buffer = x.request_buffer;
	RequestChannel *chan = x.chan;

	RequestChannel *c[x.num_chan];
	for(int i=0; i<x.num_chan; i++) {
		string str = chan->send_request("newthread");
		c[i] = new RequestChannel(str, RequestChannel::CLIENT_SIDE);
	}

	bounded_buffer<string> *res_buf[x.num_chan];
	fd_set rfd;
	int max=0;
	int result=0;
	struct timeval timeval = {0, 10};
	int req_sent;
	int req_recv;
	for(int i=0; i<x.num_chan; i++) {
		req r = request_buffer->retrieve_front();
		req_sent++;
		res_buf[i] = r.buf;
		string request = "data " + r.name;
		c[i]->cwrite(request);
	}
	while(req_recv < x.tot_rq) {
		FD_ZERO(&rfd);
		for(int i=0; i<x.num_chan; i++) {
			if(c[i]->read_fd() > max) {
				max = c[i]->read_fd();
			}
			FD_SET(c[i]->read_fd(), &rfd);
		}
		result = select(max+1, &rfd, NULL, NULL, &timeval);
		if(result) {
			for(int i=0; i<x.num_chan; i++) {
				if(FD_ISSET(c[i]->read_fd(), &rfd)) {
					req_recv++;
					string response = c[i]->cread();
					res_buf[i]->push_back(response);

					if(req_sent < x.tot_rq) {
						req r = request_buffer->retrieve_front();
						string request = "data " + r.name;
						req_sent++;
						c[i]->cwrite(request);
						res_buf[i] = r.buf;
					}
				}
			}
		}
		if(req_recv == x.tot_rq)
			break;
	}
	for(int i=0; i<x.num_chan; i++) 
		c[i]->send_request("quit");
	pthread_exit(NULL);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int n = 10; //default number of requests per "patient"
    int b = 50; //default size of request_buffer
    int w = 10; //default number of worker threads
    bool USE_ALTERNATE_FILE_OUTPUT = false;
    int opt = 0;
    while ((opt = getopt(argc, argv, "n:b:w:m:h")) != -1) {
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
            case 'h':
            default:
                cout << "This program can be invoked with the following flags:" << endl;
                cout << "-n [int]: number of requests per patient" << endl;
                cout << "-b [int]: size of req buffer" << endl;
                cout << "-w [int]: number of worker threads" << endl;
                cout << "-m 2: use output2.txt instead of output.txt for all file output" << endl;
                cout << "-h: print this message and quit" << endl;
                cout << "Example: ./client_solution -n 10000 -b 50 -w 120 -m 2" << endl;
                cout << "If a given flag is not used, a default value will be given" << endl;
                cout << "to its corresponding variable. If an illegal option is detected," << endl;
                cout << "behavior is the same as using the -h flag." << endl;
                exit(0);
        }
    }
    
    int pid = fork();
    if(pid == 0) {
        ofstream ofs;
        if(USE_ALTERNATE_FILE_OUTPUT) ofs.open("output2.txt", ios::out | ios::app);
        else ofs.open("output.txt", ios::out | ios::app);
        
        cout << "n == " << n << endl;
        cout << "b == " << b << endl;
        cout << "w == " << w << endl;
        
        cout << "CLIENT STARTED:" << endl;
        cout << "Establishing control channel... " << flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl;
        
        /*
            This time you're up a creek.
            What goes in this section of the code?
            Hint: it looks a bit like what went here 
            in PA3, but only a *little* bit.
        */


	vector<int> john_frequency_count(10, 0);
	vector<int> jane_frequency_count(10, 0);
	vector<int> joe_frequency_count(10, 0);

    // Vector Lock
    pthread_mutex_t lock; 
    pthread_mutex_init(&lock,NULL);

	struct timeval start_time;
    struct timeval end_time;
    int64_t start_usecs;
    int64_t finish_usecs;

    gettimeofday(&start_time, NULL); //start time
	
	pthread_t rthread_id[3];
	pthread_t stat_thread_id[3];
	pthread_t handler_thread_id;

	bounded_buffer<req> request_buffer(b);
	bounded_buffer<string> john_buffer(b);
	bounded_buffer<string> jane_buffer(b);
	bounded_buffer<string> joe_buffer(b);

	req requests[] = {{"John Smith", &john_buffer}, {"Jane Smith", &jane_buffer},{"Joe Smith",  &joe_buffer}};
	
	PARAMS_request PARAMS_request[] = {{&request_buffer, n, requests[0]},{&request_buffer, n, requests[1]},{&request_buffer, n, requests[2]}};
	
	for(int i=0; i<3; i++)
		pthread_create(&rthread_id[i], 0, &request_thread_function, (void *)&PARAMS_request[i]);

	PARAMS_handler PARAMS_handler = {&request_buffer, chan, n*3, w};

	pthread_create(&handler_thread_id, 0, &event_handler_function, (void *)&PARAMS_handler);

	PARAMS_stat PARAMS_stat[] = {{&john_buffer, &john_frequency_count}, {&jane_buffer, &jane_frequency_count},{&joe_buffer,  &joe_frequency_count}};

	for(int i=0; i<3; i++)
		pthread_create(&stat_thread_id[i], 0, &stat_thread_function, (void *)&PARAMS_stat[i]);

	for(int i=0; i<3; i++)
		pthread_join(rthread_id[i], NULL);

	pthread_join(handler_thread_id, NULL);

	john_buffer.push_back("quit");
	jane_buffer.push_back("quit");
	joe_buffer.push_back("quit");

	for(int i=0; i<3; i++)
			pthread_join(stat_thread_id[i], NULL);

	    gettimeofday(&end_time, NULL); //end timer

		ofs.close();
	    cout << "Results for n == " << n << ", w == " << w << ", b == " << b  << endl;
	    string histogram_table = make_histogram_table("data John","data Jane","data Joe",&john_frequency_count,&jane_frequency_count, &joe_frequency_count);
	    cout << histogram_table << endl;
	    double total_time = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);        
	    cout << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << endl;
	    cout << "Sleeping..." << endl;
	    usleep(10000);
	    chan->send_request("quit");


        //This is to output the time results into time_results.txt for easy time collection
        ofstream myfile;
        myfile.open ("test_results.txt",std::ios_base::app);
        myfile << "Results for n == " << n << ", w == " << w << ", b == " << b  << std::endl;
        myfile << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << std::endl<< std::endl;
    }
    else if(pid != 0) execl("dataserver", NULL);
}
