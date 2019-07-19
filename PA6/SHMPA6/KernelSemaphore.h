#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

class KernelSem{
	int semid;
	string fileName;
	int my_side;
public:
	KernelSem (short value, string pathName, int side)
	{	my_side= side;
		fileName=pathName;
		ofstream file;
		file.open(pathName, ofstream::app);
		file.close();
		key_t key = ftok (pathName.c_str(), 0);
		
		// initialize the sem value to an initial value = value
		// initially it was 0 meaning LOCKED
		struct sembuf sb = {0,value,0};
		semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);  // why IPC_CREAT, beyond the scope
		if (semid >=0){
			if ((semop(semid, &sb,1))==-1){
				perror("semop constructor");
				exit(-1);
			}
		} 
		else {
			while (semid<0){
				semid=semget(key,1,0666);
			}
		}

	}
	
	void P(){
		struct sembuf sb = {0, -1, 0};
		if (semop(semid, &sb, 1) == -1) {
			perror("semop P");
			exit(1);
		}
	}
	
	void V(){
		struct sembuf sb = {0, 1, 0};
		if (semop(semid, &sb, 1) == -1) {
			perror("semop V");
			exit(1);
		}
	}
	
	~KernelSem (){
		if (my_side >0){
		//union semun arg={0};
		if (semctl(semid, 0, IPC_RMID, 0) == -1) {
			perror("semctl");
			exit(1);}
		}
	}
};
