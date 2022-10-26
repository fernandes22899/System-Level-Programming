//includes
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "concurrentBuffer.h"

//defined lengths
#define BUFFER_SIZE 10
#define MAX_LINE_LENGTH 1000

//work functions
static void *w1(void *);
static void *w2(void *);

//char length
int arr[44];
//mutex for update to global arr
pthread_mutex_t mu;

//structure of work info
typedef struct{
  FILE *fp;
  void *buff;
  char *fn;
} WORK;

int main(int argc, char *argv[]){

  if(argc < 2){
    fprintf(stderr, "Needs more than 1 file.\n");
    exit(-1);
  }

  //curbuffer creation
  void *curBuff = createConcurrentBuffer(BUFFER_SIZE);
  if(curBuff == NULL){
    fprintf(stderr, "Concurrent Buffer creation FAILED\n");
    exit(-1);
  }

  if(pthread_mutex_init(&mu, NULL) != 0){
    printf("Mutex init ERROR\n");
  }

  //1st thread
  pthread_t t1[argc - 1];
  for(int i = 0; i < (argc-1); i++){
    WORK *w = malloc(sizeof(WORK));//struct for thread 1
    if(w == NULL){
      fprintf(stderr, "WORK struct malloc FAILED\n");
      exit(-1);
    }
    w->fn = argv[i+1];
    w->buff = curBuff;

    //creating 1st thread
    if(pthread_create(&t1[i], NULL, w1, w) != 0){
      fprintf(stderr, "Thread 1 creation FAILED\n");
      exit(-1);
    }
  }

    pthread_t t2[argc - 1];
    for(int i = 0; i < (argc-1); i++){
      if(pthread_create(&t2[i], NULL, w2, curBuff) != 0){
        fprintf(stderr, "Thread 2 creation FAILED\n");
        exit(-1);
      }
    }

    //finishing 1st thread
    for(int i = 0; i < (argc-1);i++){
        if(pthread_join(t1[i], NULL)){
          fprintf(stderr, "Thread 1 join FAILED\n");
          exit(-1);
      }
    }

    //finishing 2nd thread
    for(int i = 0; i < (argc-1); i++){
        if(pthread_join(t2[i], NULL)){
          fprintf(stderr, "Thread 2 join FAILED\n");
          exit(-1);
      }
    }

    //histogram output
    for(int i = 0; i < 45; i++){
      printf("%d%d\n", i+ 6, arr[i]);
    }
}


static char *getLine(FILE *fp)
{
  char buffe[MAX_LINE_LENGTH];
  int gc = getc(fp);
  if(gc == EOF){
    return NULL;
  }
  int i = 0;

  while(gc != EOF && gc != '\n'){
    buffe[i] = gc;
    i += 1;

    if(i == MAX_LINE_LENGTH){
      fprintf(stderr, "Max line length reached\n", MAX_LINE_LENGTH);
      exit(-1);
    }
    gc = getc(fp);
  }
  if(gc == '\n'){
    buffe[i] = gc;
    i += 1;
  }
  buffe[i] = 0;

  char *m = malloc(i+1);
  if(m == NULL){
    fprintf(stderr, "getLine malloc FAILED\n");
    exit(-1);
  }
  strcpy(m, buffe);

  return m;
}

static void *w1(void *prod)
{
  //2st thread function
  WORK *w = prod;
  FILE *fp = fopen(w->fn, "r");
  if(fp == NULL){
    fprintf(stderr, "file read FAILED\n");
    exit(-1);
  }

  //fp for the struct WORK
  w->fp = fp;

  char *line;
  while((line = getLine(w->fp)) != NULL){
    printf("line = %s\n", line);
    putConcurrentBuffer(w->buff, line);
  }
  putConcurrentBuffer(w->buff,NULL);

  return NULL;
}

static void *w2(void *buff)
{
  //function for 2nd thread
  char *line;
  long y = 0;

  while((line = getConcurrentBuffer(buff)) != NULL){

    //lock thread
    if(pthread_mutex_lock(&mu) != 0){
      printf("mutex lock FAILED\n");
    }

    for(int i = 0; i < strlen(line); i++){
      if(isalpha(line[i]) != 0){
        y++;
      }
      else{
        if((y > 5) && (y < 51)){
          arr[y - 6] += 1;
        }
        y = 0;
      }
    }

    //unlock thread
    if(pthread_mutex_unlock(&mu) != 0){
      printf("mutex unlock ERROR\n");
    }
    free(line);//freeing line
  }
  return NULL;
}
