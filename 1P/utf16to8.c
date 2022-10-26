#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] )
{

    FILE *inputfile;
    FILE *outputfile;

    //=======Checking for Errors=======
    if( argc != 3 )//Checking for enough arguments
    {
      fprintf( stderr, "Not enough arguments!\n" );
      exit(-1);
    }

    inputfile = fopen( argv[1], "r" );
    if( inputfile == NULL )//checking if reading file opens
    {
      fprintf( stderr, "Reading file cannot open!\n" );
      exit(-1);
    }

    outputfile = fopen( argv[2], "w" );
    if( outputfile == NULL )//checking if output file works
    {
      fprintf( stderr, "Writing file cannot open!\n" );
      exit(-1);
    }

    if( argv[1] == argv[2] )//checking if files are the same
    {
      fprintf( stderr, "Writing file cannot open!\n" );
      exit(-1);
    }

    //===========Bit check=============

    unsigned int Bom = getc( inputfile );
    unsigned int Bom2 = getc( inputfile );

    //checking for correct Bom's
    if( Bom != 0xFE || Bom2 != 0xFF )
    {
      fprintf( stderr, "File header does not match up!\n");
      exit(-1);
    }

    //-------Adding BOMS---------
    putc( 0xEF, outputfile );
    putc( 0xBB, outputfile );
    putc( 0xBF, outputfile );

    unsigned int byte = getc( inputfile );

    while( byte != EOF )
    {
      if( byte == 0 )
      {
        unsigned int byte2 = getc( inputfile );
        putc( byte2, outputfile );
      }
      else if(  )
      {

      }
    }

    fclose(inputfile);
    fclose(outputfile);
    return 0;
}
