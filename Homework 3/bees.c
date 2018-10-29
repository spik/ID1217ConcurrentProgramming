/*
=========================================================================================================
PROGRAM DESCRIPTION
=========================================================================================================
Simulates bees producing honey in a pot and a bear eating the honey using semaphores

   features: Given are n honeybees and a hungry bear. They share a pot of honey. The pot is initially
             empty; its capacity is H portions of honey. The bear sleeps until the pot is full, then
             eats all the honey and goes back to sleep. Each bee repeatedly gathers one portion of honey
             and puts it in the pot; the bee who fills the pot awakens the bear.

   usage under Linux:
     gcc bees.c -o bees -lpthread
     ./bees numBees
=========================================================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

//Maximum number of bees
#define N 10

//Maximum sixe of bowl
#define H 7

int numBees;

sem_t empty;
sem_t full;
sem_t mutex;
int honey = 0;

void *bees(void *arg){

  while(1) {

    usleep(1000*300);

    //Wait for the bowl to become empty
    sem_wait(&empty);

    //Grab the lock so that only one bee at the time can fill the bowl
    sem_wait(&mutex);

    //Put honey in bowl
    honey++;

    //If the bowl is full
    if (honey == H) {
      printf("  - Bee %d fills the bowl, signal bear to wake up\n\n", (int)arg);

      //Release the lock
      sem_post(&mutex);

      //Signal the bear to wake up
      sem_post(&full);
    }
    //The bowl is not full
    else{

      printf("  - Bee %d puts honey in the bowl, %d units of honey in bowl\n", (int)arg, honey);

      //Takes a while to put honey in the bowl
      usleep(1000*300);

      //Release lock so that another bee can fill the bowl
      sem_post(&mutex);

      //Go find more honey
      usleep(1000*300);
    }
  }
}

void *bear(void *arg){
  while(1) {

    //Wait for the bowl to become full
    sem_wait(&full);
    printf("  Bear wakes up\n");

    //As long as there is honey in the bowl
    while (honey > 0) {

      //Eat honey
      honey--;
      printf("  - Bear eats honey, %d units of honey left\n", honey);

      //It takes a while to eat (So that other bees can fill the bowl)
      usleep(1000*300);
    }
    //in this case the bowl is empty
    printf("  Bear has eaten all the honey and goes to sleep\n\n");

    //Signal all bees so that they can fill the bowl again
    for (size_t i = 0; i < H; i++) {
      sem_post(&empty);
    }
  }
}

int main(int argc, char *argv[]){

  numBees = (argc > 1)? atoi(argv[1]) : N;
  if (numBees > N) numBees = N;

  sem_init(&empty, 0, H);
  sem_init(&full, 0, 0);
  sem_init(&mutex, 0, 1);

  pthread_t bear_thread;
  pthread_t bee_thread[numBees];

  pthread_create(&bear_thread, NULL, (void *)bear, NULL);

  for (int i = 0; i < numBees; i++) {
    pthread_create(&bee_thread[i], NULL, (void *)bees, (void *)i);
  }

  pthread_join(bear_thread, NULL);
  for (int j = 0; j < numBees; j++) {
    pthread_join(bee_thread[j], NULL);
  }

  return 0;
}
