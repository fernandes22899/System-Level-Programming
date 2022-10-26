#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "defs.h"
#include "symtab.h"

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


int pc;
void * symtab;
int thru;
int errs;
int totInst;

void * opFormatSymtab;
void * opEncodingSymtab;
FILE * outFile;
int pcDuo;

int top20 = 524287;
int bot20 = -524288;
int top16 = 32767;
int bot16 = -32768;
uint32_t worA[1000000];


typedef struct symtabInfo {
  int exported;
  int imported;
  int defined;
  int referenced;
  int address;
  struct referenceAddress * refaddr;
} symtabInfo;

typedef struct referenceAddress {
  int address;
  struct referenceAddress * next;
} referenceAddress;


void createValidSymtab(void * symtab){
  char * opFormats[] = {"halt", "1",
                        "load", "5",
                        "store", "5",
                        "ldimm", "4",
                        "ldaddr", "5",
                        "ldind", "7",
                        "stind", "7",
                        "addf", "6",
                        "subf", "6",
                        "divf", "6",
                        "mulf", "6",
                        "addi", "6",
                        "subi", "6",
                        "divi", "6",
                        "muli", "6",
                        "call", "2",
                        "ret", "1",
                        "blt", "8",
                        "bgt", "8",
                        "beq", "8",
                        "jmp", "2",
                        "cmpxchg", "8",
                        "getpid", "3",
                        "getpn", "3",
                        "push", "3",
                        "pop", "3",
                        "export", "2",
                        "import", "2",
                        "word", "9",
                        "alloc", "9"};
  int opCount = 0;
  int formCount = 1;
  for (int i = 0; i < 30; i++){
    symtabInstall(symtab, opFormats[i + opCount], (void *) (long) atoi(opFormats[i + formCount]));
    opCount++;
    formCount++;
  }
}


void createOpcodeSymtab(void * symtab){
  char * opFormats[] = {"halt", "0",
                        "load", "1",
                        "store", "2",
                        "ldimm", "3",
                        "ldaddr", "4",
                        "ldind", "5",
                        "stind", "6",
                        "addf", "7",
                        "subf", "8",
                        "divf", "9",
                        "mulf", "10",
                        "addi", "11",
                        "subi", "12",
                        "divi", "13",
                        "muli", "14",
                        "call", "15",
                        "ret", "16",
                        "blt", "17",
                        "bgt", "18",
                        "beq", "19",
                        "jmp", "20",
                        "cmpxchg", "21",
                        "getpid", "22",
                        "getpn", "23",
                        "push", "24",
                        "pop", "25"};
  int opCount = 0;
  int encodingOffset = 1;
  for (int i = 0; i < 26; i++){
    symtabInstall(symtab, opFormats[i + opCount], (void *) (long) atoi(opFormats[i + encodingOffset]));
    opCount++;
    encodingOffset++;
  }
}

void printSymtab(void * symtab){
  void *iter = symtabCreateIterator(symtab);
  void *ret2;
  symtabInfo * ret;
  const char *sym = symtabNext(iter, &ret2);
  int counter = 0;
  const char * arr[100];
  while (sym != NULL){
    ret = ret2;
    if (ret->defined == 1){
      arr[counter] = sym;
      counter++;
    }
    sym = symtabNext(iter, &ret2);
  }

  symtabDeleteIterator(iter);

  symtabInfo * symbol;
  for (int i = 0; i < counter; i++){
    symbol = symtabLookup(symtab, arr[i]);
    printf("Symbol: %s", arr[i]);
    printf("\tInfo -> exported = %d\n", symbol->exported);
    printf("\t\timported = %d\n", symbol->imported);
    printf("\t\tdefined = %d\n", symbol->defined);
    printf("\t\treferenced = %d\n", symbol->referenced);
    printf("\t\taddress = %d\n\n", symbol->address);
  }
}

static void outVal(int value){
  putc(value & 0xFF, outFile);
  putc((value >> 8) & 0xFF, outFile);
  putc((value >> 16) & 0xFF, outFile);
  putc((value >> 24) & 0xFF, outFile);
}

void initAssemble(void){

  symtab = symtabCreate(10000);
  opFormatSymtab = symtabCreate(50);
  createValidSymtab(opFormatSymtab);

  opEncodingSymtab = symtabCreate(50);
  createOpcodeSymtab(opEncodingSymtab);

  pc = 0;
  pcDuo = 0;
  errs = 0;
  totInst = 0;
  thru = 1;
}


void assemble(char *label, INSTR instr){

  if (thru == 1){
    if (label != NULL){
      symtabInfo * tempSymb = symtabLookup(symtab, label);
      if (tempSymb == NULL){
        symtabInfo * info = ( symtabInfo * ) malloc( sizeof( struct symtabInfo ) );
        info->exported = 0;
        info->imported = 0;
        info->defined = 1;
        info->referenced = 0;
        info->address = pc;
        info->refaddr = NULL;
        symtabInstall(symtab, label, (void *) info);
      }
      else{
        if (tempSymb->defined == 1){
          error(ERROR_LABEL_DEFINED);
          errs++;
        }
        else{
          tempSymb->defined = 1;
          tempSymb->address = pc;
        }
      }
    }

    if (instr.format != 0){
      int invalidOpcode = 0;

      if (symtabLookup(opFormatSymtab, instr.opcode) == NULL){
        error(ERROR_OPCODE_UNKNOWN);
        errs++;
        invalidOpcode = 1;
      }

      if (!invalidOpcode){
        void * format = symtabLookup(opFormatSymtab, instr.opcode);
        if (format != (void *) (long) instr.format){
          error(ERROR_OPERAND_FORMAT);
          errs++;
          invalidOpcode = 1;
        }
      }

      if (!invalidOpcode){
        if ((instr.format == 2) || (instr.format == 5) || (instr.format == 8)){
          char * opcode = instr.opcode;
          char * curSymbol;
          if (instr.format == 2)
            curSymbol = instr.u.format2.addr;
          if (instr.format == 5)
            curSymbol = instr.u.format5.addr;
          if (instr.format == 8)
            curSymbol = instr.u.format8.addr;

          symtabInfo * curInfo = symtabLookup(symtab, curSymbol);
          if (curInfo == NULL){
            symtabInfo * newInfo = ( symtabInfo * ) malloc( sizeof( struct symtabInfo ) );
            newInfo->exported = 0;
            newInfo->imported = 0;
            newInfo->defined = 0;
            newInfo->referenced = 0;
            newInfo->address = 0xFFFF;
            newInfo->refaddr = NULL;
            symtabInstall(symtab, curSymbol, (void *) newInfo);
          }
          curInfo = symtabLookup(symtab, curSymbol);
          if (strcmp(opcode, "import") == 0){
            if (curInfo->imported == 1){
              error(ERROR_MULTIPLE_IMPORT, instr.u.format2.addr);
              errs++;
            }
            else
              curInfo->imported = 1;
          }
          else if (strcmp(opcode, "export") == 0){
            if (curInfo->exported == 1){
              error(ERROR_MULTIPLE_EXPORT, instr.u.format2.addr);
              errs++;
            }
            else
              curInfo->exported = 1;
          }
          else{
            curInfo->referenced = 1;
            if (curInfo->defined == 0)
              curInfo->address = pc;
            if (curInfo->refaddr == NULL){
              curInfo->refaddr = ( referenceAddress * ) malloc( sizeof( struct referenceAddress ) ) ;
              curInfo->refaddr->address = pc;
              curInfo->refaddr->next = NULL;
            }
            else{
              referenceAddress * newaddr = ( referenceAddress * ) malloc( sizeof( struct referenceAddress ) ) ;
              newaddr->address = pc;
              newaddr->next = curInfo->refaddr;
              curInfo->refaddr = newaddr;
            }
          }
        }

        if (strcmp(instr.opcode, "alloc") == 0){
          if (instr.u.format9.constant <= 0){
            error(ERROR_CONSTANT_ZERO);
            errs++;
          }
        }
        if (instr.format == 4){
          if ((instr.u.format4.constant > top20) || (instr.u.format4.constant < bot20)){
            error(ERROR_CONSTANT_INVALID, instr.u.format4.constant);
            errs++;
          }
        }
        if (instr.format == 7){
          if (instr.u.format7.offset > 0xFFFF){
            error(ERROR_OFFSET_INVALID, instr.u.format7.offset);
            errs++;
          }
        }
      }
    }

    if (instr.format != 0){
      if ((strcmp(instr.opcode, "export") != 0) && (strcmp(instr.opcode, "import") != 0)){
        if ((strcmp(instr.opcode, "alloc") == 0) && (instr.u.format9.constant > 0)){
          pc += instr.u.format9.constant;
          totInst += instr.u.format9.constant;
        }
        else{
          pc++;
          totInst++;
        }
      }
    }
  }

  else{
    if (instr.format != 0){
      if((strcmp(instr.opcode, "import") != 0) && (strcmp(instr.opcode, "export") != 0)){
        int alloc = 0;
        uint32_t opcode;
        void * data = symtabLookup(opEncodingSymtab, instr.opcode);
        opcode = *((uint32_t *) &data);
        uint32_t word;
        if (instr.format == 1){
          word = opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 2){
          symtabInfo * instrInfo = symtabLookup(symtab, instr.u.format2.addr);
          int32_t actAdd = 0;
          if (instrInfo->defined == 1){
            uint32_t saveAdd = instrInfo->address;
            actAdd = saveAdd - (pcDuo + 1);
          }
          actAdd = actAdd & 0xFFFFF;
          word = (actAdd << 12) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 3){
          uint32_t reg = instr.u.format3.reg;
          word = (reg << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 4){
          uint32_t reg = instr.u.format4.reg;
          uint32_t constant = instr.u.format4.constant;
          word = (constant << 12) | (reg << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 5){
          uint32_t reg = instr.u.format5.reg;
          symtabInfo * instrInfo = symtabLookup(symtab, instr.u.format5.addr);
          int32_t actAdd = 0;
          if (instrInfo->defined == 1){
            uint32_t saveAdd = instrInfo->address;
            actAdd = saveAdd - (pcDuo + 1);
          }
          actAdd = actAdd & 0xFFFFF;
          word = (actAdd << 12) | (reg << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 6){
          uint32_t reg1 = instr.u.format6.reg1;
          uint32_t reg2 = instr.u.format6.reg2;
          word = (reg2 << 12) | (reg1 << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 7){
          uint32_t reg1 = instr.u.format7.reg1;
          uint32_t reg2 = instr.u.format7.reg2;
          uint32_t offset = instr.u.format7.offset;
          word = (offset << 16) | (reg2 << 12) | (reg1 << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else if (instr.format == 8){
          uint32_t reg1 = instr.u.format8.reg1;
          uint32_t reg2 = instr.u.format8.reg2;
          symtabInfo * instrInfo = symtabLookup(symtab, instr.u.format8.addr);
          int32_t actAdd = 0;
          if (instrInfo->defined == 1){
            uint32_t saveAdd = instrInfo->address;
            actAdd = saveAdd - (pcDuo + 1);
          }
          actAdd = actAdd & 0xFFFF;
          word = (actAdd << 16) | (reg2 << 12) | (reg1 << 8) | opcode;
          word = word & 0xFFFFFFFF;
          worA[pcDuo] = word;
        }
        else{
          if (strcmp(instr.opcode,"alloc") == 0){
            alloc = 1;
            word = 0;
            word = word & 0xFFFFFFFF;
            for (int i = 0; i < instr.u.format9.constant; i++){
              worA[pcDuo] = word;
              pcDuo++;
            }
          }
          else{
            word = instr.u.format9.constant;
            word = word & 0xFFFFFFFF;
            worA[pcDuo] = word;
          }
        }
        if (alloc == 0)
          pcDuo++;
      }
    }

    if (pcDuo == pc){
      for (int w = 0; w < pcDuo; w++){
        outVal(worA[w]);
      }
    }
  }
}

void sort(const char *arr[], int size){
  int x, y;
  for (x = 0; x < size - 1; x++){
    for (y = x + 1; y < size; y++){
      if ((strcmp(arr[x], arr[y])) > 0){
        const char *tmp = arr[x];
        arr[x] = arr[y];
        arr[y] = tmp;
      }
    }
  }
}

static void outVals(char value[]){
  int counter = 0;
  while (counter < 16){
    if (counter < strlen(value))
      putc(value[counter] & 0xFF, outFile);
    else
      putc(0 & 0xFF, outFile);

    counter++;
  }
}

void deallocateSymInfo(void *symtabHandle){

  void *iter = symtabCreateIterator(symtab);
  void *ret2;
  symtabInfo * ret;
  const char *sym = symtabNext(iter, &ret2);
  while (sym != NULL){
    ret = ret2;
    free(ret);
    sym = symtabNext(iter, &ret2);
  }

  symtabDeleteIterator(iter);
}


int betweenPasses(FILE *outf){

  if (totInst > 1048576){
    error(ERROR_PROGRAM_SIZE);
    errs++;
  }

  void *iter = symtabCreateIterator(symtab);
  void *ret2;
  symtabInfo * ret;
  const char *sym = symtabNext(iter, &ret2);
  int defCounter = 0;
  int countZ = 0;
  int insymCounter = 0;
  int outsymCounter = 0;

  const char * hunArr[100];
  const char * tonArr[100];
  const char * inpArr[100];
  const char * outArr[100];

  while (sym != NULL){
    ret = ret2;
    if (ret->defined == 1){
      hunArr[defCounter] = sym;
      defCounter++;
      if (ret->exported == 1){
        inpArr[insymCounter] = sym;
        insymCounter++;
      }
    }
    else{
      if (ret->referenced == 1){
        outArr[outsymCounter] = sym;
        outsymCounter++;
      }
    }
    tonArr[countZ] = sym;
    countZ++;
    sym = symtabNext(iter, &ret2);
  }

  symtabDeleteIterator(iter);
  sort(hunArr, defCounter);

  symtabInfo * tempSymb;
  int32_t actAdd;
  uint32_t saveAdd;

  for (int i = 0; i < countZ; i++){
    tempSymb = symtabLookup(symtab, tonArr[i]);
    if (tempSymb->imported == 1){
      if (tempSymb->exported == 1){
        error(ERROR_SYMBOL_IMPORT_EXPORT, tonArr[i]);
        errs++;
      }
      if (tempSymb->defined == 1){
        error(ERROR_SYMBOL_IMPORT_DEFINED, tonArr[i]);
        errs++;
      }
      if (tempSymb->referenced == 0){
        error(ERROR_SYMBOL_IMPORT_NO_REFERENCE, tonArr[i]);
        errs++;
      }
      if (strlen(tonArr[i]) > 16){
        error(ERROR_SYMBOL_IMPORT_SIZE, tonArr[i]);
        errs++;
      }
    }
    if (tempSymb->exported == 1){
      if (strlen(tonArr[i]) > 16){
        error(ERROR_SYMBOL_EXPORT_SIZE, tonArr[i]);
        errs++;
      }
      if (tempSymb->defined == 0){
        error(ERROR_SYMBOL_EXPORT_NO_DEFINITION, tonArr[i]);
        errs++;
      }
    }
    if ((tempSymb->referenced == 1) && (tempSymb->imported == 0) && (tempSymb->defined == 0)){
      error(ERROR_LABEL_REFERENCE_NOT_FOUND, tonArr[i]);
      errs++;
    }

    if ((tempSymb->defined == 1) && (tempSymb->referenced == 1)){
      referenceAddress * temprefaddr = tempSymb->refaddr;
      saveAdd = tempSymb->address;
      while (temprefaddr != NULL){
        actAdd = saveAdd - (temprefaddr->address + 1);
        if ((actAdd > top20) || (actAdd < bot20)){
          error(ERROR_LABEL_SIZE20, tonArr[i], temprefaddr->address + 1);
          errs++;
        }
        temprefaddr = temprefaddr->next;
      }
    }
  }

  outFile = outf;
  symtabInfo * tsym;
  int refcount;
  int totEx = 0;

  for (int y = 0; y < outsymCounter; y++){
    refcount = -1;
    tsym = symtabLookup(symtab, outArr[y]);
    referenceAddress * testaddr = tsym->refaddr;
    while (testaddr != NULL){
      refcount++;
      testaddr = testaddr->next;
    }
    totEx += refcount;
  }

  int insymbols = insymCounter * 5;
  int outsymbols = (outsymCounter * 5) + (totEx * 5);
  int objwords = totInst;

  outVal(insymbols);
  outVal(outsymbols);
  outVal(objwords);
  sort(inpArr, insymCounter);

  if (insymCounter > 0){
    char insymbolcharArr[16];
    symtabInfo * tempInSymbol;
    for (int j = 0; j < insymCounter; j++){
      tempInSymbol = symtabLookup(symtab, inpArr[j]);
      strcpy(insymbolcharArr, inpArr[j]);
      outVals(insymbolcharArr);
      outVal(tempInSymbol->address);
    }
  }

  sort(outArr, outsymCounter);

  if (outsymCounter > 0){
    char sumOutArr[16];
    symtabInfo * sumTempOut;
    for (int k = 0; k < outsymCounter; k++){
      sumTempOut = symtabLookup(symtab, outArr[k]);
      referenceAddress * tempaddr = sumTempOut->refaddr;
      while (tempaddr != NULL){
        strcpy(sumOutArr, outArr[k]);
        outVals(sumOutArr);
        outVal(tempaddr->address);
        tempaddr = tempaddr->next;
      }
    }
  }

  symtabInfo * symbol;
  for (int i = 0; i < defCounter; i++){
    symbol = symtabLookup(symtab, hunArr[i]);
    printf("%s %d\n", hunArr[i], symbol->address);
  }
  thru = 2;

  return errs;
}
