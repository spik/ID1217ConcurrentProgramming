/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
//ifndef = if the following is NOT defined
#ifndef _REENTRANT
#define _REENTRANT
#define DEBUG
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
//Waits for all workers to arrive. If they have, wake all sleeping threads, else put threads to sleep
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

struct position{
  int value;
  int xPos;
  int yPos;
};

struct position mins[MAXWORKERS];
struct position maxs[MAXWORKERS];

void *Worker(void *);

/* read command line, initialize, and create threads */
//argc contains the number of arguments passed to the program
//argv is an array that contains those arguments.
int main(int argc, char *argv[]) {

  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;

  //declare a thread for the worker on place "MAXWORKERS" in the array "workerid"
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  //The  pthread_attr_init()  function  initializes  the  thread  attributes  object pointed to by attr with default attribute values.
  pthread_attr_init(&attr);

  /*PTHREAD_SCOPE_SYSTEM
    The  thread  competes for resources with all other threads in all processes on the system that are in the
    same scheduling allocation domain (a group of one or more processors).  PTHREAD_SCOPE_SYSTEM threads  are
    scheduled relative to one another according to their scheduling policy and priority.*/
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  //Check if we have more than one command line argument. If we do, set size to argument 1, else set size to MAXSIZE
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;

  //Check if we have more than two command line arguments. If we do, set number of workers to argument 2, else set size to MAXWORKERS
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  //One strip is the part of the matrix that the worker should go through. For example, of the size of the matrix is 20 and we have 10 workers, each worker should go through one tenth of the array
  stripSize = size/numWorkers;

  /* initialize the matrix */
  //Fill the matrix with random values between 0 and 99
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
  //If DEBUG is defined, print the matrix
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)

    //Create a thread for the current worker that will start execution in the "Worker" routine with the argument (void *) l
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;

//If debug is defined
#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  //The first row for the perticular thread is the id multiplied by the size of a strip.
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /* sum values in my strip */
  total = 0;
  struct position minPosition;
  struct position maxPosition;
  minPosition.value = 99;
  maxPosition.value = 0;
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      if (matrix[i][j] < minPosition.value) {
        minPosition.value = matrix[i][j];
        minPosition.xPos = j;
        minPosition.yPos = i;
      }
      if (matrix[i][j] > maxPosition.value) {
        maxPosition.value = matrix[i][j];
        maxPosition.xPos = j;
        maxPosition.yPos = i;
      }
    }
  }

  sums[myid] = total;
  mins[myid] = minPosition;
  maxs[myid] = maxPosition;
  Barrier();

  if (myid == 0) {

    total = 0;
    for (i = 0; i < numWorkers; i++){
      total += sums[i];
    }

    int currentMin = 99;
    int currentMax = 0;
    int xPosMin;
    int yPosMin;
    int xPosMax;
    int yPosMax;

    for (int j = 0; j < numWorkers; j++) {
      if(mins[j].value < currentMin){
        currentMin = mins[j].value;
        xPosMin = mins[j].xPos;
        yPosMin = mins[j].yPos;
      }
      if(maxs[j].value > currentMax){
        currentMax = maxs[j].value;
        xPosMax = maxs[j].xPos;
        yPosMax = maxs[j].yPos;
      }
    }

    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", total);
    printf("the minimum is %d at position x = %d, y = %d\n", currentMin, xPosMin, yPosMin);
    printf("the maximum is %d at position x = %d, y = %d\n", currentMax, xPosMax, yPosMax);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}
