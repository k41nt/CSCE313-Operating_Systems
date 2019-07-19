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

struct PARAMS_request {
	bounded_buffer* request_buffer;
	int num_req, num_workers, buffer_size;
	string message;
};

struct PARAMS_worker {
	RequestChannel* chan;
	bounded_buffer* request_buffer, *john_buffer, *jane_buffer, *joe_buffer;
};

struct PARAMS_stat {
	int num_req;
	vector<int>* count;
	bounded_buffer* buf;
};

struct Output {
    std::vector<int>* result_john; // Result Vector
    std::vector<int>* result_jane; // Result Vector
    std::vector<int>* result_joe; // Result Vector
    pthread_mutex_t* vector_lock; // Protect Vector Uploads
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

std::string make_histogram_table(std::string name1, std::string name2,
        std::string name3, std::vector<int> *data1, std::vector<int> *data2,
        std::vector<int> *data3) {
	std::stringstream tablebuilder;
	tablebuilder << std::setw(25) << std::right << name1;
	tablebuilder << std::setw(15) << std::right << name2;
	tablebuilder << std::setw(15) << std::right << name3 << std::endl;
	for (int i = 0; i < data1->size(); ++i) {
		tablebuilder << std::setw(10) << std::left
		        << std::string(
		                std::to_string(i * 10) + "-"
		                        + std::to_string((i * 10) + 9));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data1->at(i));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data2->at(i));
		tablebuilder << std::setw(15) << std::right
		        << std::to_string(data3->at(i)) << std::endl;
	}
	tablebuilder << std::setw(10) << std::left << "Total";
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data1->begin(), data1->end(), 0);
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data2->begin(), data2->end(), 0);
	tablebuilder << std::setw(15) << std::right
	        << accumulate(data3->begin(), data3->end(), 0) << std::endl;

	return tablebuilder.str();
}

/*
    You'll need to fill these in.
*/

void* request_thread_function(void* arg) {
	PARAMS_request* info = (PARAMS_request*) arg;

	for (int i = 0; i < info->num_req; i++) {
		info->request_buffer->push_back(info->message);
	}
	pthread_exit(NULL);
}

void* worker_thread_function(void* arg) {
	PARAMS_worker* w =(PARAMS_worker*) arg;

	while(true){
        std::string request = w->request_buffer->retrieve_front();
        std::string response = w->chan->send_request(request);

        if(request == "data John Smith"){
            w->john_buffer->push_back(response);
        }
        else if(request == "data Jane Smith"){
            w->jane_buffer->push_back(response);
        }
        else if(request == "data Joe Smith"){
            w->joe_buffer->push_back(response);
        }
        else if(request == "quit"){
            break;
        }
    }
    pthread_exit(NULL);
}

void* stat_thread_function(void* arg) {
	PARAMS_stat* stat = (PARAMS_stat*) arg;

	for(int i = 0; i < stat->num_req; i++){
        std::string response = stat->buf->retrieve_front();
        stat->count->at(stoi(response) / 10) += 1;
    }
    pthread_exit(NULL);
}


// Screen Refresh Handler
void screen_refresh(int sig, siginfo_t *si, void *uc)
{
    
    
    Output* out = (Output*) si->si_value.sival_ptr;
    
    pthread_mutex_lock(out->vector_lock);
    
    std::string histogram_table = make_histogram_table("John Smith",
        "Jane Smith", "Joe Smith", out->result_john,
        out->result_jane, out->result_joe);

    pthread_mutex_unlock(out->vector_lock);

    system("clear");
    
    std::cout << histogram_table << std::endl;
    
    
    
    //signal(sig, SIG_IGN);
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
                std::cout << "This program can be invoked with the following flags:" << std::endl;
                std::cout << "-n [int]: number of requests per patient" << std::endl;
                std::cout << "-b [int]: maximum number of requests that will be allowed in the request buffer" << std::endl;
                std::cout << "-w [int]: number of worker threads" << std::endl;
                std::cout << "-m: use output2.txt instead of output.txt for all file output" << std::endl; //purely for convenience, you may find it helpful since you have to make two graphs instead of one, and they require different data
                std::cout << "-h: print this message and quit" << std::endl;
                std::cout << "Example: ./client_solution -n 10000 -b 50 -w 120 -m" << std::endl;
                std::cout << "If a given flag is not used, a default value will be given" << std::endl;
                std::cout << "to its corresponding variable. If an illegal option is detected," << std::endl;
                std::cout << "behavior is the same as using the -h flag." << std::endl;
                exit(0);
        }
    }

    int pid = fork();
    if (pid == 0) {
    	ofstream ofs;
        if(USE_ALTERNATE_FILE_OUTPUT) ofs.open("output2.txt", ios::out | ios::app);
        else ofs.open("output.txt", ios::out | ios::app);
        
        std::cout << "n = " << n << std::endl;
        std::cout << "b = " << b << std::endl;
        std::cout << "w = " << w << std::endl;
        
        std::cout << "CLIENT STARTED:" << std::endl;
        std::cout << "Establishing control channel... " << std::flush;
        RequestChannel *chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
        std::cout << "done." << std::endl;

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
        
        ///////////////BONUS////////////////////////////////////
        // Screen Refresh Signal Timer
        timer_t timerid;
        struct sigevent sev;
        struct itimerspec its;
        long long freq_nanosecs;
        sigset_t mask;
        struct sigaction sa;
        
        
        // Handler for Timer
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = screen_refresh;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGALRM,&sa,NULL);
        
        // Block Timer Signal
        sigemptyset(&mask);
        sigaddset(&mask,SIGALRM);
        sigprocmask(SIG_SETMASK,&mask,NULL);
        
        // Output Variable
        Output out;
        out.result_john = &john_frequency_count;
        out.result_jane = &jane_frequency_count;
        out.result_joe = &joe_frequency_count;
        out.vector_lock = &lock;
        
        // Create Timer
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGALRM;
        sev.sigev_value.sival_ptr = &out;//timerid;
        timer_create(CLOCK_REALTIME, &sev, &timerid);
        
        freq_nanosecs = 2000000000; // 2 Second Timer
        its.it_value.tv_sec = freq_nanosecs / 1000000000;
        its.it_value.tv_nsec = freq_nanosecs % 1000000000;
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
        
        
        // ENABLE/DISABLE REFRESH HERE
        timer_settime(timerid, 0, &its, NULL);
        
        sigprocmask(SIG_UNBLOCK,&mask, NULL);
        ////////////////////////////////////////////////////////////////////////

        
        struct timeval start_time;
        struct timeval end_time;
        int64_t start_usecs;
        int64_t finish_usecs;

        gettimeofday(&start_time, NULL); //start time

        PARAMS_request requests;
        PARAMS_request john_data, jane_data, joe_data;
        PARAMS_worker wi[w];
        PARAMS_stat john_stat, jane_stat, joe_stat;
        bounded_buffer* request_buffer = new bounded_buffer(b);
        bounded_buffer* john_buffer = new bounded_buffer(b);
        bounded_buffer* jane_buffer = new bounded_buffer(b); 
        bounded_buffer* joe_buffer = new bounded_buffer(b); 
        RequestChannel* worker_channel[w];
        pthread_t workers[w];
        pthread_t john_st, jane_st, joe_st;

        requests.num_req = n;
        requests.num_workers = w;
        requests.buffer_size = b;

        john_data = requests;
        jane_data = requests;
        joe_data = requests;

        john_data.request_buffer = request_buffer;
        jane_data.request_buffer = request_buffer;
        joe_data.request_buffer = request_buffer;

        john_data.message = "data John Smith";
        jane_data.message = "data Jane Smith";
        joe_data.message = "data Joe Smith";

        pthread_t John, Jane, Joe;

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        pthread_create(&John, &attr, request_thread_function, &john_data);
        pthread_create(&Jane, &attr, request_thread_function, &jane_data);
        pthread_create(&Joe, &attr, request_thread_function, &joe_data);


        for(int i = 0; i < w; i++){
            std::string str = chan->send_request("newthread");
            RequestChannel* temp = new RequestChannel(str, RequestChannel::CLIENT_SIDE);
            worker_channel[i] = temp;
        }

        for(int i = 0; i < w; i++){
        	wi[i].chan = worker_channel[i];
            wi[i].request_buffer = request_buffer;
            wi[i].john_buffer = john_buffer;
            wi[i].jane_buffer = jane_buffer;
            wi[i].joe_buffer = joe_buffer;
        }

        for (int i = 0; i < w; i++){
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&workers[i], &attr, worker_thread_function, &wi[i]);
        }

        john_stat.count = &john_frequency_count;
        jane_stat.count = &jane_frequency_count;
        joe_stat.count = &joe_frequency_count;

        john_stat.num_req = n;
        jane_stat.num_req = n;
        joe_stat.num_req = n;

        john_stat.buf = john_buffer;
        jane_stat.buf = jane_buffer;
        joe_stat.buf = joe_buffer;

        pthread_create(&john_st, &attr, stat_thread_function, &john_stat);
        pthread_create(&jane_st, &attr, stat_thread_function, &jane_stat);
        pthread_create(&joe_st, &attr, stat_thread_function, &joe_stat);

        pthread_join(John, NULL);
        pthread_join(Jane, NULL);
        pthread_join(Joe, NULL);

        for(int i = 0; i < w; i++){
            request_buffer->push_back("quit");
        }

        for (int i = 0; i < w; i++){
            pthread_join(workers[i], NULL);
        }

        pthread_join(john_st, NULL);
        pthread_join(jane_st, NULL);
        pthread_join(joe_st, NULL);

        gettimeofday(&end_time, NULL); //end timer

        delete request_buffer, john_buffer, jane_buffer, joe_buffer;

        for(int i = 0; i < w; i++){
            delete worker_channel[i];
        }

        ofs.close();
        std::cout << "Results for n == " << n << ", w == " << w << ", b == " << b  << std::endl;
        string histogram_table = make_histogram_table("data John","data Jane","data Joe",&john_frequency_count,&jane_frequency_count, &joe_frequency_count);
        cout << histogram_table << endl;
        std::cout << "Sleeping..." << std::endl;
        usleep(10000);
        chan->send_request("quit");
        

        double total_time = (end_time.tv_sec - start_time.tv_sec)*1000000 + (end_time.tv_usec - start_time.tv_usec);        
        std::cout << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << std::endl;

        //This is to output the time results into time_results.txt for easy time collection
        ofstream myfile;
        myfile.open ("time_results2.txt",std::ios_base::app);
        myfile << "Results for n == " << n << ", w == " << w << ", b == " << b  << std::endl;
        myfile << "Time to completion: " << setprecision(7)<<total_time/1000000 << " secs" << std::endl<< std::endl;
        system ("exec rm *full*.txt"); // delete the key files created
        system ("exec rm *empty*.txt");// delete the key files created
        system ("exec ./kill_ipcs.sh");// remove all ipcs left over, just for safe

    }
    else if(pid != 0) execl("dataserver", NULL);
}