#include <stdio.h>
#include <stdlib.h>

#define INSUFFICIENT_MEMORY "malloc returned NULL in catchException\n"
#define INVALID_EXCEPTION_NUMBER "throwException: Invalid exception number %d\n"
#define UNHANDLED_EXCEPTION "unhandled exception %d\n"

void cancelCatchException(void);
void throwException(int);
int catchException(void);

extern long loadRBP();
extern long loadRBX();
extern long loadR12();
extern long loadR13();
extern long loadR14();
extern long loadR15();

extern void makeRBX();
extern void makeR12();
extern void makeR13();
extern void makeR14();
extern void makeR15();
extern void pare();


typedef struct snapshot
{
    long nextRBP;
    long prevRBP;
    long RIP;
    long RBX;
    long R12;
    long R13;
    long R14;
    long R15;

    struct snapshot * next;
} snapshot;

typedef struct Stack{
  struct snapshot * next;
}Stack;

Stack * stack;
int actual = 0;

int catchException(void)
{
    if( actual == 0 ){
      stack = (Stack *) malloc( sizeof( struct Stack ) );
      if(stack == NULL){
        fprintf(stderr, INSUFFICIENT_MEMORY );
        exit(-1);
      }
      actual = 1;
    }

    snapshot * sn = ( snapshot * ) malloc( sizeof( struct snapshot ) );
    if(sn == NULL){
      fprintf(stderr, INSUFFICIENT_MEMORY );
      exit(-1);
    }

    long rbp = loadRBP();
    long * lifeRBP = (long *) rbp;
    long prevrbp = *(lifeRBP);
    sn->nextRBP = rbp;
    sn->prevRBP = prevrbp;

    long rip = *( lifeRBP + 1 );
    sn->RIP = rip;

    sn->RBX = loadRBX();
    sn->R12 = loadR12();
    sn->R13 = loadR13();
    sn->R14 = loadR14();
    sn->R15 = loadR15();

    sn->next = stack->next;
    stack->next = sn;

    return 0;
}

void throwException( int exception )
{
    if( exception == 0 ){
        fprintf(stderr, INVALID_EXCEPTION_NUMBER, exception);
        exit(-1);
    }
    if( stack == 0 ){
        fprintf(stderr, UNHANDLED_EXCEPTION, exception);
        exit(-1);
    }
    else if(stack->next == NULL){
      fprintf(stderr, UNHANDLED_EXCEPTION, exception);
      exit(-1);
    }

    snapshot * sn = stack->next;
    stack->next = sn->next;
    long rbp = sn->nextRBP;

    *(long *)rbp = sn->prevRBP;
    *( ( (long *) rbp ) + 1 ) = sn->RIP;

    makeRBX(sn->RBX);
    makeR12(sn->R12);
    makeR13(sn->R13);
    makeR14(sn->R14);
    makeR15(sn->R15);
    pare( sn->nextRBP, exception );
}

void cancelCatchException(void)
{
  if(actual != 0){
    if(stack->next != NULL){
      snapshot * sn = stack->next;
      stack->next = sn->next;
      sn->next = NULL;
    }
  }
}
