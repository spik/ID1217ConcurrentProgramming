/*
=========================================================================================================
PROGRAM DESCRIPTION
=========================================================================================================
Sorts an array of random numbers

   features: Uses the quicksort algorithms with OpenMP to sort an array
             with random numbers. The array is of the size specified when
             running the code.

   usage under Linux:
     gcc qsort_openmp.c -o qsort_openmp -fopenmp
     ./qsort_openmp size numWorkers
=========================================================================================================
PERFORMANCE MEASUREMENT:
=========================================================================================================
Speedup is calculated as follows: speedup = sequential execution time/parallel execution time.
The measurements are done on a computer with 8 processors.
---------------------------------------------------------------------------------------------------------
Array size: 500 000                          execution time
Number of threads         1         2         3         4         5         mean          speedup
1                     3.164020  3.167840  3.216410  3.207590  3.208410    3.207590           -
2                     1.692760  1.719740  1.713640  1.745850  1.729030    1.719740        1,719740
4                     0.950373  0.995934  0.968644  1.049860  0.952732    0.968644        3,311423
8                     0.722541  0.758013  0.729878  0.747084  0.747943    0.747084        4,293480
10                    0.721602  0.724693  0.739074  0.753084  0.739306    0.739074        4,340012
---------------------------------------------------------------------------------------------------------
Array size: 1 000 000                        execution time
Number of threads         1         2         3         4         5         mean          speedup
1                     12.52280  12.55310  12.67860  12.65280  12.65960    12.65280             -
2                     6.578210  6.551390  6.600700  6.585270  6.536190    6.578210        1,923441
4                     3.455520  3.555080  3.675010  3.637100  3.569260    3.569260        3,544936
8                     2.506640  2.556760  2.521150  2.530960  2.540740    2.530960        4,999209
10                    2.506110  2.549840  2.548130  2.538880  2.554990    2.548130        4,965523
---------------------------------------------------------------------------------------------------------
Array size: 1 300 000                        execution time
Number of threads         1         2         3         4         5         mean          speedup
1                     20.94730  21.24070  21.69930  21.72180  21.32190    21.32190           -
2                     10.93580  10.91280  10.98070  10.93770  10.96720    10.93770        1.949395
4                     5.735770  5.829250  5.776590  5.850970  5.914150    5.850970        3.644165
8                     4.130210  4.172310  4.225530  4.258060  4.270440    4.225530        5.045971
10                    4.301070  4.301200  4.328720  4.371010  4.384800    4.328720        4.925682
---------------------------------------------------------------------------------------------------------

*/
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXLENGTH 1300000
#define MAXWORKERS 10
//#define DEBUG

int length;
int numWorkers;

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

//Method for sorting. This is the part that can be parallelized
void sort(int *array, int left, int right){

  if (right > left) {

    //Partition the elements in the array. "j" is the index of the pivot element
    int j = partition(array, left, right);

    #pragma omp task
    {
      sort(array, left, j-1);
    }
    #pragma omp task
    {
      sort(array, j+1, right);
    }
  }
}

int main(int argc, char const *argv[]) {

  length = (argc > 1)? atoi(argv[1]) : MAXLENGTH;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;

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

  omp_set_num_threads(numWorkers);

  start_time = omp_get_wtime();

  #pragma omp parallel
  {
    //Only one thread should call sort first time.
    #pragma omp single nowait
    {
      sort(array, 0, length - 1);
    }
  }
  end_time = omp_get_wtime();

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
