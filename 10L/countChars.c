//
// simple test for the concurrent buffer
//
// create two threads which communicate through the buffer
//
// the main thread opens a file (name given on commandline) and
// passes the FILE* to the first thread through the parameter to
// the first thread's work function
//
// the first thread reads lines from the file, passing each line via
// the buffer to the second thread.
//
// the second thread sums the number of characters in the lines
//
// at EOF the first thread sends a NULL pointer to the buffer and
// terminates
//
// when the second thread receives the NULL pointer it returns the
// sum through the void* return value of its work function
//
// the main thread uses thread join to wait for the two threads to
// finish (the join also cleans up the memory for the threads)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "concurrentBuffer.h"

// allow this to be overridden at compile time
// e.g. gcc -DBUFFER_SIZE=1 ...
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 5
#endif

// the work functions for the threads
static void *work1(void *);
static void *work2(void *);

// struct to be used to pass information to the first thread
typedef struct {
  FILE *fp;
  void *buffer;
} INFO;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: countChars file\n");
    exit(-1);
  }

  // open the file to be read
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL)
  {
    fprintf(stderr, "can't open %s\n", argv[1]);
    exit(-1);
  }

  // create the concurrent buffer
  void *cBuffer = createConcurrentBuffer(BUFFER_SIZE);
  if (cBuffer == NULL)
  {
    fprintf(stderr, "can't create the concurrent buffer\n");
    exit(-1);
  }

  // create struct to pass info to first thread
  INFO *ip = malloc(sizeof(INFO));
  if (ip == NULL)
  {
    fprintf(stderr, "can't malloc the INFO struct\n");
    exit(-1);
  }
  ip->fp = fp;
  ip->buffer = cBuffer;

  // create the first thread
  pthread_t thread1;
  if (pthread_create(&thread1, NULL, work1, ip) != 0)
  {
    fprintf(stderr, "can't create thread 1\n");
    exit(-1);
  }

  // create the second thread
  pthread_t thread2;
  if (pthread_create(&thread2, NULL, work2, cBuffer) != 0)
  {
    fprintf(stderr, "can't create thread 2\n");
    exit(-1);
  }

  // wait for the first thread to finish
  if (pthread_join(thread1, NULL))
  {
    fprintf(stderr, "join for thread 1 fails\n");
    exit(-1);
  }

  // wait for the second thread to finish
  void *ret;
  if (pthread_join(thread2, &ret))
  {
    fprintf(stderr, "join for thread 2 fails\n");
    exit(-1);
  }
  long sum = (long) ret;

  // free the INFO struct
  free(ip);

  // close the file
  fclose(fp);

  // free the concurrent buffer
  deleteConcurrentBuffer(cBuffer);

  printf("%ld\n", sum);
}

// get a line from an opened file
//   returns NULL at EOF
//   assumes a line is less than MAX_LINE_LENGTH characters long
//   mallocs space for the return value
//   line returned includes the newline on the end
//   last line might not include newline if last line is terminated by EOF
//   in this case one more call to readLine is required for NULL to be returned
#define MAX_LINE_LENGTH 1024
static char *getLine(FILE *fp)
{
  char buf[MAX_LINE_LENGTH];
  int i = 0;
  int c = getc(fp);
  if (c == EOF) return NULL;
  while (c != EOF && c != '\n')
  {
    buf[i] = c;
    i += 1;
    if (i == MAX_LINE_LENGTH)
    {
      fprintf(stderr, "maximum line length (%d) exceeded\n", MAX_LINE_LENGTH);
      exit(-1);
    }
    c = getc(fp);
  }
  if (c == '\n')
  {
    buf[i] = c;
    i += 1;
  }
  buf[i] = 0;
  char *s = malloc(i+1);
  if (s == NULL)
  {
    fprintf(stderr, "malloc failed in getLine\n");
    exit(-1);
  }
  strcpy(s, buf);
  return s;
}

// work function for the first thread (the producer)
static void *work1(void *info)
{
  INFO *ip = info;

  // read the lines in the file
  char *line;
  while ((line = getLine(ip->fp)) != NULL)
  {
    putConcurrentBuffer(ip->buffer, line);
  }

  // put a NULL to indicate EOF
  putConcurrentBuffer(ip->buffer, NULL);

  return NULL;
}

// work function for the second thread (the consumer)
static void *work2(void *buffer)
{
  // character count
  long sum = 0;

  // get lines from the buffer
  // and sum the number of characters in the line
  char *line;
  while ((line = getConcurrentBuffer(buffer)) != NULL)
  {
    sum += strlen(line);
    free(line);
  }

  // return the character count via the void* return value
  return (void *) sum;
}
