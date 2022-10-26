#include <stdlib.h>
#include <stdio.h>

int main( int argc, char *argv[] )
{

  if( argc == 2 ){
    //argv[0] is name of program
    FILE *f = fopen( argv[1], "r" );

    if( f == NULL ){
      printf( "cannot open %s\n", argv[1] );
      exit(-1);
    }

    int gc;
    int count = 0;
    while( (gc = getc(f)) != EOF )
    {
      if( count%16 == 0 ){
        if( count != 0 )
          printf( "\n%08d", count );
        else
          printf( "%08d", count );
      }

      printf( " %02x", gc );

      count++;
      //gc = getc(f);
    }
    printf( "\n%08d\n", count );
  }
  else{
    printf( "Usage: lab1 filename\n" );
    exit(-1);
  }
  return 0;
}
