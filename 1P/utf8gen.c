// generate all one-byte UTF-8 cases

#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "utf8-1"

int main()
{
  FILE *fp = fopen(FILE_NAME, "w");
  if (fp == NULL)
  {
    fprintf(stderr, "cannot open %s\n", FILE_NAME);
    exit(-1);
  }

  // put out BOM
  putc(0xEF, fp);
  putc(0xBB, fp);
  putc(0xBF, fp);

  int i;

  for (i = 0; i <= 0x7f; i++)
  {
    putc(i, fp);
  }
  fclose(fp);
}
