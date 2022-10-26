#include <stdio.h>
#include <stdlib.h>

#define ERR_TWO_ARGUMENTS "Requires 2 arguments, input file and output file"
#define ERR_INVALID_FILE "Cannot open file:"
#define ERR_CONTINUATION_BYTE "Missing or Unexpected Continuation byte at offset:"
#define ERR_OVERLONG_ENCODING "Overlong Encoding at offset:"
#define ERR_NONCHARACTER "The sequence decodes to a noncharacter: start byte at offset:"
#define ERR_RESERVED_SURROGATES "The sequence decodes to a value reserved for surrogates: start byte at offset:"
#define ERR_OUT_OF_RANGE "The sequence decodes to a value greater than 0x10FFFF: start byte at offset:"
#define ERR_INCOMPLETE_CODE_UNIT "Incomplete UTF-16 code unit at offset:"
#define ERR_UNPAIRED_SURROGATE "Unpaired surrogate at offset:"
// followed by the 2 or 3 bytes that you see at the start of the file in hex (if there are at least 2 or 3 bytes)
#define ERR_MISSING_BOM "Missing BOM"
//default error if all other specific error cases fail
#define ERR_INCORRECT_START_BYTE "Incorrect start byte at offset:"

int main( int argc, char *argv[] )
{

  FILE *inputfile;
  FILE *outputfile;

  inputfile = fopen( argv[1], "r" );
  outputfile = fopen( argv[2], "w" );

  //=======Checking for Errors========
  if( argc != 3 )//Checking for enough arguments
  {
    fprintf( stderr, ERR_TWO_ARGUMENTS );
    exit(-1);
  }

  if( inputfile == NULL )//checking if reading file opens
  {
    fprintf( stderr, ERR_INVALID_FILE );
    exit(-1);
  }
  if( outputfile == NULL )//checking if output file works
  {
    fprintf( stderr, ERR_INVALID_FILE );
    exit(-1);
  }
  if( argv[1] == argv[2] )//checking if files are the same
  {
    fprintf( stderr, ERR_INVALID_FILE );
    exit(-1);
  }

  //===========Bit check=============
  unsigned int Bom = getc( inputfile );
  unsigned int Bom2 = getc( inputfile );
  unsigned int Bom3 = getc( inputfile );

  //checking for correct Bom's
  if( (Bom != 0xEF) || (Bom2 != 0xBB) || (Bom3 != 0xBF) )
  {
    fprintf( stderr, ERR_MISSING_BOM);
    exit(-1);
  }

  //-------Adding BOMS---------
  putc( 0xFE, outputfile );
  putc( 0xFF, outputfile );

  unsigned int byte = getc( inputfile );
  while( byte != EOF )
  {
    if( byte >> 7 == 0 )//if byte begins with 0
    {
      putc( 0, outputfile);
      putc( byte, outputfile );
    }
    else if( byte >> 5 == 6 )//if byte begins with 110, so 2 bytes
    {
      unsigned int byteTwo = getc( inputfile );
      unsigned int unicode;

      byte = byte & 0x1F; //byte 11111
      byte = byte << 6;
      byteTwo = byteTwo & 0x3F;//byte 111111

      unicode = byte | byteTwo;

      putc( unicode >> 8, outputfile );
      putc( unicode & 0x00FF, outputfile );
    }
    else if( byte >> 4 == 14 )//if byte begins with 1110, so 3 bytes
    {
      unsigned int byteTwo = getc( inputfile );
      unsigned int byteThree = getc( inputfile );
      unsigned int unicode;
      unsigned int ad;

      byte = byte & 0x0F;//byte 1111
      byteTwo = byteTwo & 0x3F;//byte 111111
      byteThree = byteThree & 0x3F;

      unicode = byte << 12;
      ad = byteTwo << 6;

      unicode = unicode | ad;
      unicode = unicode | byteThree;

      putc( unicode >> 8, outputfile );
      putc( unicode, outputfile );
    }
    else if( byte >> 3 == 29 )//if byte begins with 11110
    {
      unsigned int byteTwo = getc( inputfile );
      unsigned int byteThree = getc( inputfile );
      unsigned int byteFour = getc( inputfile );
      unsigned int unicode;
      unsigned int begin;
      unsigned int end;

      byte = (byte & 0x07) << 18;//111
      byteTwo = (byteTwo & 0x3F) << 12;//111111
      byteThree = (byteThree & 0x3F) << 6;
      byteFour = byteFour & 0x3F;

      unicode = byte | byteTwo | byteThree | byteFour;
      unicode = unicode - 0x10000;

      begin = (unicode >> 10) & 0x3FF;
      begin = begin + 0xD800;

      end = unicode & 0x0003FF;
      end = end + 0xDC00;

      putc( begin >> 8, outputfile );
      putc( begin & 0x00FF, outputfile );
      putc( end >> 8, outputfile );
      putc( end & 0x00FF, outputfile );
    }
    byte = getc( inputfile );
  }

  fclose(outputfile);
  fclose(inputfile);
  return 0;
}
