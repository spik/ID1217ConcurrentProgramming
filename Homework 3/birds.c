/*
=========================================================================================================
PROGRAM DESCRIPTION
=========================================================================================================
Simulates a parent bird feeding its babies by using semaphores.

   features: Given are n baby birds and one parent bird. The baby birds eat out of a common dish that
             initially contains W worms. Each baby bird repeatedly takes a worm, eats it, sleeps for
             a while, takes another worm, and so on. If the dish is empty, the baby bird who discovers
             the empty dish chirps real loud to awaken the parent bird. The parent bird flies off and
             gathers W more worms, puts them in the dish, and then waits for the dish to be empty again.
             This pattern repeats forever.

   usage under Linux:
     gcc birds.c -o birds -lpthread
     ./birds numBirds
=========================================================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define W 7
#define NUM_BIRDS 5

sem_t empty;
sem_t full;
sem_t mutex;
int worms = W;
int numBirds;

void *parent_bird(void *arg){

  while(1) {

    //Parent waits for the dish to be empty
    sem_wait(&empty);
    sem_wait(&mutex);
    printf("  Parent hears the baby's chirps and leaves to find worms\n");

    //Parent founds a random number if worms between 1 and W
    worms = 1+rand()%W;
    printf("  Parent found %d worms\n\n", worms);
    sem_post(&mutex);

    //Wake as many birds as the number of worms found
    for (int i = 0; i < worms; i++) {
      sem_post(&full);
    }
  }
}

void *baby_bird(void *arg){
  while(1) {

    usleep(1000*300);
    //Birds wait for the dish to be filled by parent. When this happens, the birds wake up.
    sem_wait(&full);

    //Grab a lock so that only one bird can eat at a time.
    sem_wait(&mutex);
    worms--;
    if (worms > 0) {
      printf("  - Baby bird %d ate worm and falls asleep, %d worms left in dish\n", (int)arg, worms);

      //Takes a while to eat the worm
      usleep(1000*300);

      //Release the lock so that the other birds can eat to
      sem_post(&mutex);

      //Sleep for a while (So that other birds can take the lock)
      usleep(1000*300);
    }
    else{
      //There's only one worm left. The bird that takes the last worm calls for the parent.
      printf("  - Bird %d ate the last worm, chirps to signal parent\n\n", (int)arg);

      //Release the lock
      sem_post(&mutex);

      //Send the signal to the parent that the dish is empty
      sem_post(&empty);
    }
  }
}

int main(int argc, char *argv[]){

  numBirds = (argc > 1)? atoi(argv[1]) : NUM_BIRDS;
  if (numBirds > NUM_BIRDS) numBirds = NUM_BIRDS;

  sem_init(&empty, 0, 0);
  sem_init(&full, 0, W);
  sem_init(&mutex, 0, 1);

  pthread_t parent;
  pthread_t baby[numBirds];

  pthread_create(&parent, NULL, (void *)parent_bird, NULL);

  printf("\n\n");

  for (int i = 0; i < numBirds; i++) {
    pthread_create(&baby[i], NULL, (void *)baby_bird, (void *)i);
  }

  pthread_join(parent, NULL);
  for (int j = 0; j < numBirds; j++) {
    pthread_join(baby[j], NULL);
  }

  return 0;
}
