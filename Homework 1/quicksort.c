/*Sorts an array of random numbers

   features: Uses the quicksort algorithms with threads to sort an array
             with random numbers. The array is of the size specified when
             running the code.

   usage under Linux:
     gcc quicksort.c -o quicksort -lpthread
     ./quicksort size

*/
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXLENGTH 1000000
//#define DEBUG

int length;

//Because pthread_create only send one argument to the specified function,
//a structh as to be created to contain the necessary arguments
struct sort_args{
  int *array;
  int left;
  int right;
};

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized ){
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */

//Swap two elemnets in an array
void swap(int array[], int i, int j){
  int temp = array[i];
  array[i] = array[j];
  array[j] = temp;
}

//Method for partitioning
int partition (int *array, int left, int right){
  int i = left, j = right + 1;
  while(1){

    /*Note: need a do while loop as this will cause i++/j-- to be executed at least once before checking if the condition is met */

    //while the element in position i is less than the pivot element, increment i
    do i++; while(array[i] <= array[left] && i < right);

    //While the element in position j is greater than the pivot element, decrement j
    do j--; while (array[j] > array[left]);

    //Check if pointers cross. In that case, we are done with the partitioning => break from the loop
    if (i >= j) break;

    //If the pointers hasn't crossed yet, swap their positions
    swap(array, i, j);
  }
  //swap the pivot element with j.
  swap(array, left, j);

  //return index
  return j;
}

int numOfThreads = 3000;

/*
void serial_sort(int *array, int left, int right){

  if (right > left) {
    int j = partition(array, left, right);
    serial_sort(array, left, j-1);
    serial_sort(array, j+1, right);
  }
}*/

//Method for calling sort with the initial values
void *start_sort(void *init){

  struct sort_args *start = init;
  sort(start->array, start->left, start->right);

  return NULL;
}
//Method for sorting. This is the part that can be parallelized
void sort(int *array, int left, int right){
  if (right > left) {

    //Partition the elements in the array. "j" is the index of the pivot element
    int j = partition(array, left, right);

    if (numOfThreads-- > 0) {

      struct sort_args args = {array, left, j-1};

      pthread_t thread;

      //sort the left part of the array, i.e. the part to the left side of the pivot j
      pthread_create(&thread, 0, start_sort, &args);

      //sort(array, left, j-1);

      //sort the right part of the array, i.e. the part to the rght side of the pivot j
      sort(array, j+1, right);

      pthread_join(thread, NULL);
    }
    else{
      sort(array, left, j-1);
      sort(array, j+1, right);
    }
  }
}

int main(int argc, char const *argv[]) {

  length = (argc > 1)? atoi(argv[1]) : MAXLENGTH;

  int *array = calloc(length, sizeof(int));

  //Fill the array with random values between 0 and 99
  for (int i = 0; i < length; i++) {
    array[i] = rand()%99;
  }

  //Print array before sort
  #ifdef DEBUG
  printf("\n");
  for (int i = 0; i < length; i++) {
    printf("\t%d", array[i]);
  }
  printf("\n\n");
  #endif

  start_time = read_timer();

  /*struct sort_args init = {array, 0, length - 1};
  start_sort(&init);*/
  sort(array, 0, length - 1);

  end_time = read_timer();

  //Print array after sort
  #ifdef DEBUG
  for (int i = 0; i < length; i++) {
    printf("\t%d", array[i]);
  }
  #endif

  printf("\n\n");
  printf("The execution time is %g sec\n", end_time - start_time);

  printf("\n");

  free(array);

  return 0;
}
