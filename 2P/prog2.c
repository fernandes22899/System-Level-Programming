//
// f2i: convert IEEE single-precision floating-point to
// 32-bit two's complement integer.
//
// i2f: convert 32-bit two's complement integer to IEEE
// single-precision floating point.
//
// Remember when you implement these functions you may not use the
// float or double types. Everything must be done using only integer
// types.

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// f2i
//   Parameter: 32-bit value that should be interpreted as an IEEE
//     single-precision floating-point value.
//   Return value: 32-bit two's complement integer value produced by
//     converting the parameter to integer.
//   Note: This routine should duplicate what the Intel 64 hardware
//     does when converting floats to ints. Be sure to consider NaNs,
//     denormalized numbers, infinities, overflow, etc.
int32_t f2i( uint32_t in )
{
  //-------------int values---------------------
  uint32_t fu;
  uint32_t sign;        //First bit in sequence
  uint32_t exponent;    //8-bits after sign
  uint32_t significand; //23 bits after exponent
  uint32_t avalue;      //value of exponent - 127
  uint32_t answer = 0;      //int value

  //---------------Magic------------------------

  fu = in;
  sign = fu >> 31;             //extracting 1st bit
  exponent = (fu >> 23) & 0xFF;       //extracting 23 first bits
  significand = (fu & 0x7FFFFF) | 0x800000; //First 23 bits
  avalue = exponent - 127;    //Actual Value

  if( exponent == 0xFF )    //if exponent all 1's then return max
    return 0x80000000;        //most negative number
  else if( (exponent >= 0) && (exponent < 127) )
    return 0;
  else if( avalue >= 31 )     //Actual Value greater than 31
    return 0x80000000;    //most negative number


  if ( avalue > 23 )
    significand = significand << (avalue - 23);
  else
    significand = significand >> (23 - avalue); // Discard decimal digits


  if(sign == 0)
    answer = significand;
  else if(sign == 1)
    answer = -(significand);


  return answer;               //end of program
}

// i2f
//   Parameter: 32-bit two's complement integer value.
//   Return value: 32-bit value that should be interpreted as an IEEE
//     single-precision floating-point value produced converting the
//     parameter to floating point.
//   Note: This routine should duplicate what the Intel 64 hardware
//     does when converting ints to floats.
uint32_t i2f( int32_t in )
{
  if(in == 0){ //If 0, return 0
    return 0;
  }

//Initiate variables used
  int32_t fp = 0;;
  uint32_t sign;
  uint32_t significand;
  uint32_t avalue; // aka actual exponent
  uint32_t calcin;
  uint32_t ex; //aka storede exponent
  uint32_t temp;
  uint32_t gbit; //aka guardbits
  uint32_t temp2;
  int count = 0;
  int sbit; //aka stickybit


  if( in >= 0 ){ //positive number
    sign = 0;
  }
  else{         //negative number
    sign = 1;
  }

  if( sign == 1 ){
    in = ~in;
    in += 1;
  }

  calcin = in;
  while( calcin != 1 ){ //Get actual expononet (aka avalue)
    calcin = calcin >> 1;
    count += 1;
  }
  avalue = count;

  ex = avalue + 127; //stored exponent = actual exponent + 127

  if( avalue > 23 ){
    significand = (in >> (avalue - 23)) & 0x7FFFFF;
    if( (avalue == 24) || (avalue == 25) ){
      sbit = 0;
    }
    else{
      temp = in << (32 - (avalue - 23) + 2);
      temp = temp & 0xFFFFFFFF;
      temp = temp >> (32 - (avalue - 23) + 2);
      if( temp == 0 ){
        sbit = 0;
      }
      else{
        sbit = 1;
      }
    }

    if( avalue < 25 ){
      gbit = (in & 0x01) << 1;
    }
    else{
      temp2 = in >> (avalue - 25);
      gbit = temp2 & 0x03;
    }

    if( gbit == 2 ){
      if( sbit == 0 ){
        if( (significand & 0x000001) == 1 ){
          if( significand == 0x7FFFFF ){
            significand = 0 << 23;
            ex += 1;
          }
          else{
            significand += 1;
          }
        }
      }
      else if( sbit == 1 ){
        if( significand == 0x7FFFFF ){
          significand = 0 << 23;
          ex += 1;
        }
        else{
          significand += 1;
        }
      }
    }
    else if( gbit == 3 ){
      if( significand == 0x7FFFFF ){
        significand = 0 << 23;
        ex += 1;
      }
      else{
        significand += 1;
      }
    }
  }
  else{
    significand = (in << (23 - avalue) & 0x7FFFFF);
  }

  fp = (sign << 31) | (ex << 23) | (significand);

  return fp; // just a stub for now; always return all zero bits
}

// s2d
//   Parameter: 32-bit value that should be interpreted as an IEEE
//     single-precision floating-point value.
//   Return value: 64-bit value that should be interpreted as an IEEE
//     double-precision floating-point value produced by converting the
//	   single-precision value supplied.

uint64_t s2d( uint32_t in )
{
  uint64_t dp;
  uint64_t sign;
  uint64_t exponent;
  uint64_t significand;
  uint32_t avalue;
  uint64_t dp64;

  dp = 0;
  sign = in >> 31;
  exponent = (in >> 23) & 0xFF;
  significand = in & 0x7FFFFF;
  avalue = exponent - 127;
  dp64 = avalue + 1023;

  if( in == 0 || in == 0x80000000 ) //0
    return (sign << 63);
  else if( (in == 0x7F800000) == 0x7F800000 ) //NaN
    dp = ((sign << 63) | 0x7FF8000000000000 | (significand << 29));
  else if( in == 0x7F800000 || in == 0xFF800000 ) //Infinity
    dp = (sign << 63) | 0x7FF0000000000000;
  else
    dp = ((sign << 63) | (dp64 << 52) | (significand << 29));


	return dp; // just a stub for now; always return zero
}
