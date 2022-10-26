#include <stdio.h>
#include <stdint.h>

long getFP();

int frameCount(){
    long fp = getFP();

    int c = 0;
    long fpNew;
    long fpOld = fp;

    while((fpNew = *((long *) fpOld)) > fpOld){
      fpOld  = fpNew;
      c++;
    }

    return c;
}
