/* UNIX tee command implementation using pthreads

   features: reads from a file specified as the first argument when running the code
             and writes to standard output and a specified file specified as the second
             argument when running the code.

   usage under Linux:
     gcc tee.c -o tee -lpthread
     ./tee file1 file2
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

FILE *fp, *fp2;

//Write content from buffer to file "tee.txt"
void* writeFile(int buffer){
  fputc(buffer, fp2);
  return NULL;
}

//Write content from buffer to standard output
void* printText(int buffer){
  printf("%c", buffer);
  return NULL;
}

int main(int argc, char *argv[]) {

    printf("\n");

  //Check that the user has specified two arguments: a fileto read from and a file to write to.
  if(argc != 3){
    printf("ERROR: You must specify a file to be read from and a file to write to\n");
    exit(0);
  }

  int buffer;
  fp = fopen(argv[1], "r");
  fp2 = fopen(argv[2], "w");
  pthread_t thread1;
  pthread_t thread2;

  //As long as we are not at the end of the file
  while (!feof(fp)) {

    //Read from file and place in buffer
    buffer = fgetc(fp);

    //Create thread to write to file
    pthread_create(&thread1, 0, writeFile, (void*)buffer);

    //Create thread to write to standard output
    pthread_create(&thread2, 0, printText, (void*)buffer);
  }

  printf("\n");

  fclose(fp);
  fclose(fp2);
}
