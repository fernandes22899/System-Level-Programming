// simple unit test for symtabCreate, symtabInstall, symtabLookup
// and symtabDelete

#include <stdio.h>

#include "symtab.h"

int main()
{
  void *symtab = symtabCreate(10000);
  if (symtab == NULL)
  {
    fprintf(stderr, "symtabCreate failed!\n");
    return -1;
  }

  if (!symtabInstall(symtab, "MAX", (void *) (long) 42))
  {
    fprintf(stderr, "symtabInstall failed!\n");
    return -1;
  }

  if (((long) symtabLookup(symtab, "MAX")) != 42)
  {
    fprintf(stderr, "symtabLookup failed!\n");
    return -1;
  }

  symtabDelete(symtab);

  return 0;
}

