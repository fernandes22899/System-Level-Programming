#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "concurrentBuffer.h"


typedef struct Buffer{
  int front,back,size;
  unsigned int cap;
  pthread_mutex_t mu;
  pthread_cond_t cv;
  void ** queue;
} Buffer;

void *createConcurrentBuffer(unsigned int size){

  if(size == 0){
    return NULL;
  }
  Buffer * b = (Buffer *) malloc( sizeof( struct Buffer));

  if(b == NULL){
    return NULL;
  }
  b->size = 0;
  b->cap = size;
  b->front = 0;
  b->back = size - 1;
  b->queue = (void **) malloc( sizeof(void *) * size );

  if(b->queue == NULL){
    free(b->queue);
    return NULL;
  }

  for(int i = 0; i < size; i++){
    b->queue[i] = NULL;
  }

  if(pthread_mutex_init(&b->mu, NULL) != 0){
    printf("Error in mutex init\n");
  }
  if(pthread_cond_init(&b->cv, NULL) != 0){
    printf("Error in condition init\n");
  }

  return b;
}

void putConcurrentBuffer(void *handle, void *p){

  if(handle == NULL){
    fprintf(stderr, "NULL handle to putConcurrentBuffer()\n");
    exit(-1);
  }
  Buffer * b = handle;

  if(pthread_mutex_lock(&b->mu) != 0){
    printf("Error in mutex lock\n");
  }
  while(b->size == b->cap){
    if(pthread_cond_wait(&b->cv, &b->mu) != 0){
      printf("Error in condition wait\n");
    }
  }

  b->back = (b->back + 1) % b->cap;
  b->queue[b->back] = p;
  b->size = b->size + 1;

  if(pthread_cond_signal(&b->cv) != 0){
    printf("Error in condition signal\n");
  }
  if(pthread_mutex_unlock(&b->mu) != 0){
    printf("Error in mutex unlock\n");
  }

}

void *getConcurrentBuffer(void *handle){

  if(handle == NULL){
    fprintf(stderr, "NULL handle to putConcurrentBuffer()\n");
    exit(-1);
  }
  Buffer * b = handle;

  if(pthread_mutex_lock(&b->mu) != 0){
    printf("Error in mutex lock\n");
  }
  while(b->size == 0){
    if(pthread_cond_wait(&b->cv, &b->mu) != 0){
      printf("Error in condition wait\n");
    }
  }

  void * data = b->queue[b->front];
  b->front = (b->front + 1) % b->cap;
  b->size = b->size - 1;

  if(pthread_cond_signal(&b->cv) != 0){
    printf("Error in condition signal\n");
  }
  if(pthread_mutex_unlock(&b->mu) != 0){
    printf("Error in mutex unlock\n");
    return 0;
  }

  return data;
}

void deleteConcurrentBuffer(void *handle){

  if(handle == NULL){
    fprintf(stderr, "NULL handle to deleteConcurrentBuffer()\n");
    exit(-1);
  }
  Buffer * b = handle;

  free(b->queue);
  free(b);
}
