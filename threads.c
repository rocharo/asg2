//
//  threads.c
//  
//
//

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

static void test_thread(void);
static int thread = 0;
void thread_exit(int);

int ticketCount = 0;

static struct sigaction sa; 
static struct itimerval timer;

// Yield to another thread
void thread_yield(int signum) {
    int old_thread = thread;
    
    // This is the scheduler, it is a bit primitive right now
    //thread = 1+thread;
	thread = tickets[ticketCount++];

    printf("Thread %d yielding to thread %d\n", old_thread, thread);
    printf("Thread %d calling swapcontext\n", old_thread);
    
    // This will stop us from running and restart the other thread
    swapcontext(&ctx[old_thread], &ctx[thread]);

    // The other thread yielded back to us
    printf("Thread %d back in thread_yield\n", thread);
}

// This is the main thread
// In a real program, it should probably start
// all of the threads and then wait for them to finish
// without doing any "real" work
int main(void) {
    printf("Main starting\n");
    
    printf("Main calling thread_create\n");

	// Creat 10 threads
	int i;
	for (i = 0; i < 10; i++){
		thread_create(&test_thread);
		thread++;
	}
	thread = 0;
	
	// Assign tickets to threads
	for (i=0; i < 1000; i++){
		tickets[i] = (rand() % 10);
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
    while(ticketCount<1000);

    // We should never get here
    exit(0);
    
}

// This is the thread that gets started by thread_create
static void test_thread(void) {
    printf("In test_thread\n");

    // Loop, doing a little work then yielding to the other thread
    while(1) {
        
        //printf("Test_thread just doing nothing\n");
        
        //thread_yield();
        
        //printf("Test_thread returned from thread_yield\n");
    }
    printf("Exiting thread...\n");
    thread_exit(0);
}

// Create a thread
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
