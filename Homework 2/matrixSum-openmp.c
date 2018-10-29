/*
=================================================================================================================
PROGRAM DESCRIPTION
=================================================================================================================
Matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c
     ./matrixSum-openmp size numWorkers
=================================================================================================================
PERFORMANCE MEASUREMENT:
=================================================================================================================
Speedup is calculated as follows: speedup = sequential execution time/parallel execution time.
The measurements are done on a computer with 8 processors.
-----------------------------------------------------------------------------------------------------------------
Matrix size: 5000                                   execution time
Number of threads          1            2           3           4           5           mean            speedup
1                     0.02674890   0.02688490   0.02653000   0.02692150   0.02702740   0.02688490          -
2                     0.01504790   0.01428280   0.01451590   0.01693500   0.01488700   0.01488700       1,805931
4                     0.00940127   0.00935110   0.01676290   0.00921839   0.00962786   0.00940127       2,859709
8                     0.01560380   0.01662610   0.02067240   0.01701030   0.01657140   0.01657140       1,622367
10                    0.01351710   0.01261550   0.01281340   0.01260030   0.01273940   0.01273940       2,110374
-----------------------------------------------------------------------------------------------------------------
Matrix size: 7500                                   execution time
Number of threads          1            2           3           4           5           mean            speedup
1                     0.06423520   0.06234850   0.06229220   0.06418490   0.06645220   0.06418490          -
2                     0.03420950   0.03308130   0.03420850   0.03266820   0.03312790   0.03312790       1,937488
4                     0.01822470   0.01868900   0.02139400   0.01926890   0.03672270   0.01926890       3,331010
8                     0.02914270   0.03628500   0.02555240   0.03840140   0.02113510   0.02914270       2,202434
10                    0.02044690   0.02084850   0.03016740   0.02767240   0.02106330   0.02106330       3,047239
-----------------------------------------------------------------------------------------------------------------
Matrix size: 10 000                                 execution time
Number of threads          1            2           3           4           5           mean            speedup
1                     0.10723800   0.10817600   0.10904100   0.11069700   0.10922800   0.10904100          -
2                     0.05674540   0.05681000   0.05605140   0.05571610   0.05650190   0.05605140       1,945375
4                     0.03190160   0.03197590   0.03104420   0.03081690   0.03164450   0.03164450       3,445812
8                     0.03593400   0.05334630   0.04336880   0.03535920   0.04159060   0.04159060       2,621770
10                    0.05109580   0.04995770   0.05166580   0.04925110   0.04119180   0.04995770       2,182667
-----------------------------------------------------------------------------------------------------------------
*/

#include <omp.h>

double start_time, end_time;

#include <stdio.h>
#include <stdlib.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */
//#define DEBUG

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];
void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j, max, min, maxX, maxY, minX, minY, total=0;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  min = max = matrix[0][0];

  start_time = omp_get_wtime();

  //i is private automatically as we use an omp parallel for
#pragma omp parallel for reduction (+:total) private(j)
    for (i = 0; i < size; i++){
      for (j = 0; j < size; j++){
        total += matrix[i][j];

        //Check of statement is true outside the loop so that we don't enter the critical section when we don't have to. This increases performance considerably.
        if (matrix[i][j] < min) {
          #pragma omp critical
          {
            if (matrix[i][j] < min) {
              min = matrix[i][j];
              minX= j;
              minY = i;
            }
          }
        }
        if (matrix[i][j] > max){
          #pragma omp critical
          {
            if (matrix[i][j] > max) {
              max = matrix[i][j];
              maxX = j;
              maxY = i;
            }
          }
        }
      }
    }

// implicit barrier

  end_time = omp_get_wtime();
  printf("\n");
    /* print results */
  printf("The total is %d\n", total);
  printf("The minimum is %d, x = %d, y = %d\n", min, minX, minY);
  printf("The minimum is %d, x = %d, y = %d\n", max, maxX, maxY);
  printf("The execution time is %g sec\n", end_time - start_time);
  printf("\n");
}
