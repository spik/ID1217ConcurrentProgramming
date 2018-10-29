/*Approximates pi

   features: Approximates pi by calculating the area of the upper-right quadrant
             of the unit circle and multiplying it by four.

   usage under Linux:
     gcc pi.c -o pi -lpthread -lm
     ./pi numberOfThreads

*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

//epsilon value defines how many times the program is recursivly run. If epsilon is small the number of subintervals are high
#define EPSILON 0.00000000001
#define MAXWORKERS 10

int numThreads;

//As pthread_create only takes one argument for the starting function, we need a struct to store multiple arguments
struct quadargs {

  double a, b, fa, fb, area, answer;

};

double f(double x);
void* start_quad(void *init);
double read_timer();

double start_time, end_time; /* start and end times */

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



//recursive adaptive quadrature procedure
//calculate the area of upper right quadrant with numerical integration
double quad(double a, double b, double fa, double fb, double area){

  double m, fm, leftarea, rightarea;

  //calculate the middle point (x) of the two limits
  m = (a+b)/2;

  //calculate the y-value of the middle point m
  fm = f(m);

  //calculate the left area
  leftarea = ((fa + fm)*(m-a))/2;

  //calculate the right area
  rightarea = ((fm + fb)*(b-m))/2;

  //check if absolute value of the difference between the two areas are larger than epsilon.
  if (fabs((leftarea + rightarea) - area) > EPSILON) {

    struct quadargs arg = {a, m, fa, fm, leftarea, 0.0};

    //run by creating new threads each recursive call ad
    //decrement the number fo threads each reqursive call and enter the if statement only when numThreads > 0
    if (numThreads-->0) {

      pthread_t thread1;

      //Create a thread that starts execution in start_quad function with arg as argument. This calculates the left area.
      pthread_create(&thread1, 0, start_quad, &arg);

      //Calculate the right area
      rightarea = quad(m, b, fm, fb, rightarea);
      pthread_join(thread1, NULL);

      //Save the answer in the struct
      leftarea = arg.answer;
   }

    else{
      //Calculate the area sequentially
      leftarea = quad(a, m, fa, fm, leftarea);
      rightarea = quad(m, b, fm, fb, rightarea);
    }
  }
  //return the calculated area
  return (leftarea + rightarea);
}

//Function for getten arguments from the struct and calling quad with those arguments
void* start_quad(void *init){

  struct quadargs *params = init;
  params->answer = quad(params->a, params->b, params->fa, params->fb, params->area);
  return NULL;
}

//calculate y value using pythagoras theorem
double f(double x){

  //f(x) = sqrt(1-x^2)
  return sqrt(1-pow(x, 2));
}

int main(int argc, char *argv[]) {

  //Get number of threads from argument 1
  numThreads = (argc > 1)? atoi(argv[1]) : MAXWORKERS;

  start_time = read_timer();

  //set values to get the area in the first quadrant of unit circle and multiply by four to get the area of the whole circle
  double pi = 4 * quad(0, 1, f(0), f(1), ((f(1)+f(0))*(1-0))/2);
  end_time = read_timer();

  //10 decimal accuracy
  printf("\n\nPi is approximately %.10f\n", pi);

  printf("\n\nThe execution time is %g sec\n", end_time - start_time);
  printf("\n\n");

  return 0;
}
