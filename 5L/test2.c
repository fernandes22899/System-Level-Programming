// simple unit test for symtabCreate, symtabInstall, symtabCreateIterator,
// symtabNext, symtabDelete and symtabDeleteIterator

#include <stdio.h>
#include <string.h>

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

  void *iter = symtabCreateIterator(symtab);
  if (iter == NULL)
  {
    fprintf(stderr, "symtabCreateIterator failed!\n");
    return -1;
  }

  void *ret;
  const char *sym = symtabNext(iter, &ret);
  if (strcmp(sym, "MAX") || (((long) ret) != ((long) 42)))
  {
    fprintf(stderr, "symtabNext failed!\n");
    return -1;
  }

  symtabDeleteIterator(iter);
  symtabDelete(symtab);

  return 0;
}

