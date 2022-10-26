#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

int32_t f2i( int32_t f )
{
  //-------------int values---------------------
  uint32_t fu;
  uint32_t sign;        //First bit in sequence
  uint32_t exponent;    //8-bits after sign
  uint32_t significand; //23 bits after exponent
  uint32_t avalue;      //value of exponent - 127
  uint32_t answer = 0;      //int value

  //---------------Magic------------------------

  fu = f;
  sign = fu >> 31;             //extracting 1st bit
  exponent = (fu >> 23) & 0xFF;       //extracting 23 first bits
  significand = (fu & 0x7FFFFF) | 0x800000; //First 23 bits
  avalue = exponent - 127;    //Actual Value

  if( exponent == 0xFF ){    //if exponent all 1's then return max
    return 0x80000000;        //most negative number
  }
  else if((exponent >= 0) && (exponent < 127)){
    return 0;
  }
  else if( avalue >= 31 ){      //Actual Value greater than 31
    return 0x80000000;    //most negative number
  }

  if ( avalue > 23 ){
    significand = significand << (avalue - 23);
  }
  else{
    significand = significand >> (23 - avalue); // Discard decimal digits
  }

  if(sign == 0)
    answer = significand;
  else if(sign == 1)
    answer = -(significand);


  return answer;               //end of program
}
