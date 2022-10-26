#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_T 14

pthread_mutex_t mu;
pthread_cond_t cv;

int cnt;
int total;
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

void * work(void * ent){
  long index = (long) ent;
  FILE *fp = fopen(filename[index], "r");
  int c;
  int lc = 0;

  if(fp == NULL){
    ln[index] = lc;
    return 0;
  }

  for(c = getc(fp);c != EOF;c = getc(fp)){
    if(c == '\n')
      lc++;
  }

  if(pthread_mutex_lock(&mu) != 0){
    printf("ERROR in mutex lock\n");
  }

  total += lc;
  if(++cnt == n){
    if(pthread_cond_signal(&cv) != 0){
      printf("ERROR in condition signal\n");
      return 0;
    }
  }
  if(pthread_mutex_unlock(&mu) != 0){
    printf("ERROR in mutex unlock");
    return 0;
  }

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

  for(int x = 0;x<n;x++){
    filename[x] = argv[x+1];
  }
  sort(filename, n);

  if(pthread_mutex_init(&mu,NULL) != 0){
    printf("ERROR in mutex init\n");
  }

  if(pthread_cond_init(&cv,NULL) != 0){
    printf("ERROR in condition init\n");
  }

  cnt = 0;
  total = 0;

  for(long i = 0;i<n;i++){
    if(pthread_create(&pt[i],NULL,work,(void *) i) != 0){
      printf("ERROR in create\n");
      return 0;
    }
  }

  while(cnt != n){
    if(pthread_cond_wait(&cv, &mu) != 0){
      printf("ERROR in condition parent\n");
    }
  }

  for(int x = 0; x<n;x++){
    printf("%s=%d\n",filename[x],ln[x]);
  }
  printf("Total Count=%d\n", total);
  return 0;

}
