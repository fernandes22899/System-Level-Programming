#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symtab.h"

int main()
{

  void *symtab = symtabCreate(4444);

  if(symtab == NULL){
    fprintf(stderr, "Create Failure\n" );
    return -1;
  }

  if( !symtabInstall(symtab, "MAX", (void *)(long)48) ){
    fprintf(stderr, "Install Failure\n" );
    return -1;
  }

  void *symIter = symtabCreateIterator(symtab);
  if(symIter == NULL){
    fprintf(stderr, "Create Iterator Failure\n" );
    return -1;
  }

  void *n;
  const char *sym = symtabNext(symIter, &n);
  if (strcmp(sym, "MAX") || (((long)n) != ((long)48)))
  {
      fprintf(stderr, "Symtab Next Failure\n");
      return -1;
  }

  symtabDeleteIterator(symIter);

  symtabDelete(symtab);

  return 0;
}
