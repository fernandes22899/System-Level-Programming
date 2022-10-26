// test for exception handler

#include <stdio.h>

void throwException(int);
void cancelCatchException(void);
int catchException(void);


void f1(){
  fprintf(stderr, "f1 entered\n");
  throwException(999);
  fprintf(stderr, "f1 exiting\n");
}

int main(){

  int caughtException;

  fprintf(stderr, "-----Entered main-----");

  if( (caughtException = catchException()) ){
    fprintf(stderr, "catch clause entered\n" );
    fprintf(stderr, "caught exception %d\n", caughtException );
    fprintf(stderr, "catch clause exiting\n" );
  }
  else{
    fprintf(stderr, "try block entered\n");
    f1();
    cancelCatchException();
    fprintf(stderr, "try block exiting\n");
  }

  fprintf(stderr, "-----Exiting main-----\n");
  return 0;
}
