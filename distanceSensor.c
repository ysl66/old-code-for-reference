/*needs to be compiled with -lrt -lm option*/
#include <time.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "beagleboneLib.h"

timer_t timerID[10];
float sum;
int average; 

float getDistance(){
	int val;
	float distance;

	initAIN();
	val = ainVal();
	distance = 41.548*pow(((pow(1.786,2) * ((float)val/4095)) + 0.30221),-1.5281);
	printf("%f\n", distance);
	return distance;
}

static void timerHandler(int sig, siginfo_t *si, void *uc) {
	timer_t* tidp;
	tidp = si->si_value.sival_ptr;

	if (*tidp == timerID[0]) {
		sum = 0;
	}

	sum += getDistance();

	if (*tidp == timerID[9]) {	
		average = round(sum / 10.0); //get the average for all 10 measurements after last timer goes off
		printf("%i\n", average);
	}
}

void makeTimers (char* name, timer_t* timerID, int expire) {
	struct sigevent event;
	struct itimerspec itimer;
	struct sigaction action;
	int sigNo = SIGRTMIN; //raises signal when the timer expires

	//setup signal handler
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = timerHandler; //the action taken when timer expires
	sigemptyset(&action.sa_mask);
	if (sigaction(sigNo, &action, NULL) == -1) {
		printf("error setting up signal handling");
	}
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = sigNo;
	event.sigev_value.sival_ptr = timerID;
	timer_create(CLOCK_REALTIME, &event, timerID);

	itimer.it_interval.tv_sec = 0;
	//itimer.it_interval.tv_n = 0;
	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_nsec = expire * 1000000;
	timer_settime(*timerID, 0, &itimer, NULL);
}

int main() {
	int i;
	//create 10 timers
	while(1) {
		for (i = 0; i < 10; i++) {
			char name[20] = "timer #";
			char num[5];
			sprintf(num, "%i", i);
			strcat(name,num);
			
			makeTimers("timer", &timerID[i], 100); //a timer goes off every 0.01s --> data collected every 
		}
		sleep(3);
		int fd;
	    	char * sensor_fifo = "/tmp/my_fifo";
	    	char * error_Msg = "Error opening file, checking if file exists.\n";
	    	if( (fd = open(sensor_fifo, O_WRONLY)) <0){
			printf("%s", error_Msg);
	   	}
	    	char avgString[3];
	    	sprintf(avgString, "%i", average);


	    	//printf("%i", average);
		write(fd, avgString, sizeof(char*));
    		close(fd);
    		sleep(1);
	}
	return 0;	
}
