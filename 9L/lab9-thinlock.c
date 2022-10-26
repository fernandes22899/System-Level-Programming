#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define MAX_T 14

int pthread_yield();

atomic_flag t;

int total = 0;
int n;

pthread_t pt[MAX_T];
int ln[MAX_T];
char * filename[MAX_T];

void sort(char *arr[], int size){
  int x,y;
  for(x = 0;x < size-1;x++){
    for(y = x+1;y < size;y++){
      if((strcmp(arr[x],arr[y])) > 0){
        char *temp = arr[x];
        arr[x] = arr[y];
        arr[y] = temp;
      }
    }
  }
}

void * work(void * en){
  long index = (long) en;

  FILE *fp = fopen(filename[index], "r");
  int c;
  int lc = 0;
  if(fp == NULL){
    ln[index] = lc;
    return 0;
  }

  for(c = getc(fp); c != EOF; c = getc(fp)){
    if(c == '\n'){
      lc++;
    }
  }

  while(1){
    if(atomic_flag_test_and_set(&t)){
      break;
    }
    if(pthread_yield()){
      printf("ERROR in pthread yield\n");
      exit(-1);
    }
  }

  total += lc;
  atomic_flag_clear(&t);

  ln[index] = lc;
  return 0;
  }

  int main(int argc, char * argv[]){
    n = argc - 1;
    if(n < 1){
      printf("No file names given\n");
      return 0;
    }
    if(n > MAX_T){
      printf("Too many file names given\n");
      return 0;
    }

    for(int x = 0; x < n;x++){
      filename[x] = argv[x+1];
    }

    sort(filename,n);
    atomic_flag_clear(&t);

    for(long i = 0; i < n; i++){
      if(pthread_create(&pt[i], NULL, work, (void *) i) != 0){
        printf("ERROR in create thread\n");
        return 0;
      }
    }

    for(int i = 0; i<n;i++){
      if(pthread_join(pt[i], NULL)){
        printf("ERROR in pthread join\n");
        return 0;
      }
    }

    for(int i=0;i<n;i++){
      printf("%s=%d\n", filename[i], ln[i]);
    }
    printf("Total Count=%d\n", total );
    return 0;
  }
