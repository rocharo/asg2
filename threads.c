/*
 * Bardia Keyoumarsi
 * Vincent Lantaca
 * Alexis Rocha
 *
 * CMPS 111
 * Assignement 2 - Thread scheduler
 * Lottery schedualing with timers
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <string.h> 
#include <sys/time.h> 

#define _XOPEN_SOURCE
#include <ucontext.h>

#define NUMBER_OF_THREADS 10
#define NUMBER_OF_TICKETS 1000

static ucontext_t ctx[NUMBER_OF_THREADS];

static int tickets[NUMBER_OF_TICKETS];
static int threadCount[NUMBER_OF_THREADS];

static void test_thread(void);
static int thread = 0;
static int ticketCount = 0;

void thread_exit(int);

static struct sigaction sa; 
static struct itimerval timer;

/*Yield to another thread
* yeild thread will schedule each thread, it will also 
* stop a thread change and restart the other threads 
* in essence it will swap threads.
*/
void thread_yield(int signum) {
    int old_thread = thread;
    
    //The scheduler picks a ticket from the array and swaps the threads
    thread = tickets[ticketCount++];
    swapcontext(&ctx[old_thread], &ctx[thread]);
}

/* The main fucntion will create 10 threads and assign ramdom tickets to 
* each thread. the main will also install a timer handler to signal 
* and dictate when the yeild fucntion will be called. thus it will also 
* configure a timer to expire after 100 msec and every 100 ms within each other. this
* will have the tickets be looped though each time
*/ 
int main(void) {
    printf("Main starting\n");
    
    printf("Main calling thread_create\n");

	// Create 10 threads
	int i;
	for (i = 0; i < NUMBER_OF_THREADS; i++){
                threadCount[i] = 0;
		thread_create(&test_thread);
		thread++;
	}
	thread = 0;
	
	// Assign tickets to threads
	for (i=0; i < NUMBER_OF_TICKETS; i++){
		tickets[i] = (rand() % NUMBER_OF_THREADS);
		threadCount[tickets[i]]++;
	}
    
    printf("Main returned from thread_create\n"); 

	/* Install timer_handler as the signal handler for SIGVTALRM.  */ 
	memset (&sa, 0, sizeof (sa)); 
	sa.sa_handler = &thread_yield; 
	sigaction (SIGVTALRM, &sa, NULL); 

	/* Configure the timer to expire after 100 msec...  */ 
	timer.it_value.tv_sec = 0; 
	timer.it_value.tv_usec = 100000; 
	/* ... and every 100 msec after that.  */ 
	timer.it_interval.tv_sec = 0; 
	timer.it_interval.tv_usec = 100000; 
	/* Start a virtual timer. It counts down whenever this process is 
	 executing.  */ 
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
	
	printf("Main returned from create_timer\n");

    // Loop, doing a little work then yielding to the other thread
    while(ticketCount<100);
	
	for (i = 0; i < NUMBER_OF_THREADS; i++)
		printf("Thread %d was allowed to run %d times.\n",
		       i, threadCount[i]);

    // We should never get here
    exit(0);
    
}

// This is the thread that gets started by thread_create
static void test_thread(void) {
    printf("In test_thread\n");

    // doing busy work
    while(1);
	
    thread_exit(0);
}

/* Create a thread will create a thread but and along with that 
* creat its context if it does not have any. this function will 
* also setup the stack for each threa but also deal with the thread 
* context when it exits
*/
int thread_create(int (*thread_function)(void)) {
    int newthread = 1+thread;
    
    printf("Thread %d in thread_create\n", thread);
    
    printf("Thread %d calling getcontext and makecontext\n", thread);

    // First, create a valid execution context the same as the current one
    getcontext(&ctx[newthread]);

    // Now set up its stack
    ctx[newthread].uc_stack.ss_sp = malloc(8192);
    ctx[newthread].uc_stack.ss_size = 8192;

    // This is the context that will run when this thread exits
    ctx[newthread].uc_link = &ctx[thread];

    // Now create the new context and specify what function it should run
    makecontext(&ctx[newthread], test_thread, 0);
    
    printf("Thread %d done with thread_create\n", thread);
}

// This doesn't do anything at present
void thread_exit(int status) {
    printf("Thread %d exiting\n", thread);
}
