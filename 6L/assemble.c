/*
 * assemble.c - handles the details of assembly for the asx20 assembler
 *
 *              This currently just contains stubs with debugging output.
 */
 //IMPORTS
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "symtab.h"

//GIVEN ERROR HANDLING
#define ERROR_PROGRAM_SIZE "Program consumes more than 2^20 words"
#define ERROR_LABEL_DEFINED "Label %s already defined"
#define ERROR_OPCODE_UNKNOWN "Unknown Opcode %s"
#define ERROR_OPERAND_FORMAT "Opcode does not match the given operands"
#define ERROR_CONSTANT_ZERO "Constant must be greater than zero"
#define ERROR_CONSTANT_INVALID "Constant %d will not fit into 20 bits"
#define ERROR_OFFSET_INVALID "Offset %d will not fit into 16 bits"
#define ERROR_MULTIPLE_EXPORT "Symbol %s exported more than once"
#define ERROR_MULTIPLE_IMPORT "Symbol %s imported more than once"
#define ERROR_LABEL_REFERENCE_NOT_FOUND "Label %s is referenced but not defined or imported"
#define ERROR_SYMBOL_IMPORT_EXPORT "Symbol %s is both imported and exported"
#define ERROR_SYMBOL_IMPORT_DEFINED "Symbol %s is both imported and defined"
#define ERROR_SYMBOL_IMPORT_NO_REFERENCE "Symbol %s is imported but not referenced"
#define ERROR_SYMBOL_EXPORT_NO_DEFINITION "Symbol %s is exported but not defined"
#define ERROR_SYMBOL_IMPORT_SIZE "Symbol %s is imported and longer than 16 characters"
#define ERROR_SYMBOL_EXPORT_SIZE "Symbol %s is exported and longer than 16 characters"
#define ERROR_LABEL_SIZE16 "Reference to label %s at address %d won't fit in 16 bits"
#define ERROR_LABEL_SIZE20 "Reference to label %s at address %d won't fit in 20 bits"

// enable debugging printout
#define DEBUG 1
//----------Varibles Declared--------
static int prog = 0;
static int co = 0;
static char* SymIn[2222];
static char* SymOut[2222];
static uint32_t uIn;
static uint32_t uOut;
void *createSymIn;
void *createSymOut;
void *create;
int pcIn[2222];
int pcOut[2222];

//STRUCTURE of symbol table
typedef struct sym
{
  void *s;
  int c;
} sym;
static sym *struc[2222]; // usage of data structure

// this is called once so that the assembler can initialize any internal
// data structures.
void initAssemble(void)
{
  #if DEBUG
    fprintf(stderr, "initAssemble called\n");
  #endif
  create = symtabCreate(2222); //internal data structures initalization
  createSymIn = symtabCreate(2222);
  createSymOut = symtabCreate(2222);
}

// this is the "guts" of the assembler and is called for each line
// of the input that contains a label, instruction or directive
//
// note that there may be both a label and an instruction or directive
// present on a line
//
// note that the assembler directives "export" and "import" have structure
// identical to instructions with format 2, so that format is used for them
//
// for the directives "word" and "alloc" a special format, format 9, is used
//
// see defs.h for the details on how each instruction format is represented
// in the INSTR struct.
//
void assemble(char *label, INSTR instr)
{
#if DEBUG
  fprintf(stderr, "assemble called:\n");
  if (label)
  {
    fprintf(stderr, "  label is %s\n", label);
  }
  if (instr.format != 0)
  {
    fprintf(stderr, "  instruction is %s", instr.opcode);
    switch(instr.format)
    {
      case 1:
        fprintf(stderr, "\n");
        break;
      case 2:
        fprintf(stderr, " %s\n", instr.u.format2.addr);
        break;
      case 3:
        fprintf(stderr, " r%d\n", instr.u.format3.reg);
        break;
      case 4:
        fprintf(stderr, " r%d,%d\n", instr.u.format4.reg,
          instr.u.format4.constant);
        break;
      case 5:
        fprintf(stderr, " r%d,%s\n", instr.u.format5.reg,
          instr.u.format5.addr);
        break;
      case 6:
        fprintf(stderr, " r%d,r%d\n", instr.u.format6.reg1,
          instr.u.format6.reg2);
        break;
      case 7:
        fprintf(stderr, " r%d,%d(r%d)\n", instr.u.format7.reg1,
          instr.u.format7.offset, instr.u.format7.reg2);
        break;
      case 8:
        fprintf(stderr, " r%d,r%d,%s\n", instr.u.format8.reg1,
          instr.u.format8.reg2, instr.u.format8.addr);
        break;
      case 9:
        fprintf(stderr, " %d\n", instr.u.format9.constant);
        break;
      default:
        bug("unexpected instruction format (%d) in assemble", instr.format);
        break;
    }
  }
#endif

  if(symtabLookup(create,label) != NULL)
    error(ERROR_LABEL_DEFINED,label);//check if clear
  else{
    symtabInstall(create,label,(long)prog);
    co++;
  }

  if(instr.format != 0)
  {
    if(instr.opcode && strcmp(instr.opcode, "alloc") == 0)
      prog = prog + instr.u.format9.constant;
    else if(instr.opcode && strcmp(instr.opcode, "import") != 0
        && strcmp(instr.opcode, "export") != 0)
      prog++;
    if(instr.opcode && strcmp(instr.opcode, "ldaddr") == 0)
      if(instr.format != 5)
        error(ERROR_OPERAND_FORMAT);
  }

  if(instr.opcode && strcmp(instr.opcode,"export") == 0)
  {
    sym *mal = malloc(sizeof(sym));
    if(instr.format == 2)
    {
      SymIn[uIn] = instr.u.format2.addr;
      symtabInstall(createSymIn, instr.u.format2.addr, symtabLookup(create,instr.u.format2.addr));

      struc[uIn] = mal;
      uIn++;
    }
  }
  else if(instr.opcode && strcmp(instr.opcode, "import") == 0){
    if(instr.format == 2){
      SymOut[uOut] = instr.u.format2.addr;
      uOut++;
    }
  }
}

// this is called between passes and provides the assembler the file
// pointer to use for outputing the object file
//
// it returns the number of errors seen on pass1
//
int betweenPasses(FILE *outf)
{
  fopen(outf, "w"); //open outf and write
  #if DEBUG
    fprintf(stderr, "betweenPasses called\n");
  #endif
 //local varibles declared
  void *data;
  void *iter = symtabCreateIterator(create);
  void *sy = symtabNext(iter,&data);
  void *coArr[co];
  void *progCArr[co];
  bool wh = true;
  bool in = true;
  bool out = true;
  int pcInT[2222];
  int pcOutT[2222];

  for(int i = 0; i < co; i++){
    coArr[i] = sy;
    progCArr[i] = data;

    sy = symtabNext(iter,&data);
  }

  while(wh)
  {
    wh = false;
    for(int i = 0; i < co; i++)
      for(int j=i+1;j < co; j++)
        if(strcmp(coArr[j-1],coArr[j]) > 0)
        {
          void *fo = coArr[j];
          void *foT = progCArr[j];
          coArr[j] = coArr[j-1];
          coArr[j-1] = fo;
          progCArr[j] = progCArr[j-1];
          progCArr[j-1] = foT;
          wh = true;
        }
  }

  for(int i = 0; i < co; i++)
    printf("%s %ld\n", coArr[i], progCArr[i]);

  for(int i = 0; i < uIn; i++)//INSYMBOL
    pcIn[i] = symtabLookup(create,pcIn[i]);

  if(uOut > 0){//OUTSYMBOL
    for(int i = 0;i < uOut; i++)
      pcOut[i] = symtabLookup(create,pcOut[i]);
  }

  //Sorting of OUTSYMBOLS AND INSYMBOLS
  while(in)
  {
    in = false;
    for(int i = 0; i < uIn; i++)
      for(int j=i+1;j < uIn; j++)
        if(strcmp(pcIn[j-1],pcIn[j]) > 0)
        {
          void *fo = pcIn[j];
          void *foT = pcInT[j];
          pcIn[j] = pcIn[j-1];
          pcIn[j-1] = fo;
          pcInT[j] = pcInT[j-1];
          pcInT[j-1] = foT;
          in = true;
        }
  }

  while(out)
  {
    out = false;
    for(int i = 0; i < uOut; i++)
      for(int j=i+1;j < uOut; j++)
        if(strcmp(pcOut[j-1],pcOut[j]) > 0)
        {
          void *fo = pcOut[j];
          void *foT = pcOutT[j];
          pcOut[j] = pcOut[j-1];
          pcOut[j-1] = fo;
          pcOutT[j] = pcOutT[j-1];
          pcOutT[j-1] = foT;
          out = true;
        }
  }

  for(int i = 0; i < uIn; i++)
  {
    struc[i]->s = pcIn[i];
    struc[i]->c = symtabLookup(create, struc[i]->s);
  }

  uIn = uIn*5;
  uOut = uOut*5; //5 is number of words

  //Begin writing into file
  //INSYMBOL
  putc(uIn,outf);
  putc(uIn>>8, outf);
  putc(uIn>>16, outf);
  putc(uIn>>24, outf);

  //OUTSYMBOL
  putc(uOut,outf);
  putc(uOut>>8, outf);
  putc(uOut>>16, outf);
  putc(uOut>>24, outf);

  //OBJECT
  putc(prog,outf);
  putc(prog>>8, outf);
  putc(prog>>16, outf);
  putc(prog>>24, outf);

  for(int i = 0; i < uIn/5;i++)
  {
    int strucLen = strlen(struc[i]->s);
    for(int j = 0; j < strucLen; j++){
      char *t[strlen(struc)];
      strcpy(t,strucLen);
      putc(t[j],outf);
    }

    //Variants of file length
    if(strucLen == 1){
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
    }
    else if(strucLen == 2){
      putc(0, outf);
      putc(0, outf);
    }
    else if(strucLen == 3){
      putc(0, outf);
    }
    else if(strucLen == 4){
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);
      putc(0, outf);

      putc((long)struc[i]->c, outf);
      putc((long)struc[i]->c >> 8, outf);
      putc((long)struc[i]->c >> 16, outf);
      putc((long)struc[i]->c >> 24, outf);
    }
  }


  return 0;
}
