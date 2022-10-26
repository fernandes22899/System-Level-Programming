#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "simVM.h"


typedef struct TLB {
  unsigned int data;
  unsigned int stamp;
} TLB;

typedef struct page {
  unsigned int data;
  unsigned int stamp;
  int mod;
} page;

typedef struct VMS {
  unsigned int sizeVM;
  unsigned int sizePM;
  unsigned int pageSize;
  unsigned int sizeTLB;
  int rep;
  int tlbRep;

  unsigned int * vm;
  page ** pt;
  TLB ** tlb;

  int ptIn;
  int tlbIn;

  int pageFaults;
  int tlbMisses;
  int diskWrites;
  int ts;

} VMS;


void *createVM(unsigned int sizeVM, unsigned int sizePM, unsigned int pageSize,
  unsigned int sizeTLB, char rep, char tlbRep){

  if (!(sizeVM > sizePM))
    return NULL;

  if (!(sizePM > 0))
    return NULL;

  if ((pageSize % 2) != 0)
    return NULL;

  if ((sizeVM * pageSize) > pow(2,32))
    return NULL;

  if (sizeTLB > sizePM)
    return NULL;

  if (sizeTLB <= 0)
    return NULL;

  int repINT = (int) rep;
  int tlbRepINT = (int) tlbRep;
  if (!(((repINT == 0) || (repINT == 1)) && ((tlbRepINT == 0) || (tlbRepINT == 1))))
    return NULL;

  VMS * vms = ( VMS * ) malloc( sizeof( struct VMS ) );
  if (vms == NULL)
    return NULL;

  vms->pageFaults = 0;
  vms->tlbMisses = 0;
  vms->diskWrites = 0;
  vms->ptIn = 0;
  vms->tlbIn = 0;
  vms->ts = 0;
  vms->sizeVM = sizeVM;
  vms->sizePM = sizePM;
  vms->pageSize = pageSize;
  vms->sizeTLB = sizeTLB;
  vms->rep = (int) rep;
  vms->tlbRep = (int) tlbRep;


  vms->vm = ( unsigned int * ) malloc( sizeof( unsigned int ) * (sizeVM * pageSize));
  if (vms->vm == NULL){
    free(vms->vm);
    return NULL;
  }

  for (int i = 0; i < sizeVM; i++){
    vms->vm[i] = i;
  }

  page * p;
  vms->pt = ( page ** ) malloc( sizeof( struct page ) * sizePM );
  if (vms->pt == NULL){
    free(vms->pt);
    return NULL;
  }
  for (int i = 0; i < sizePM; i++){
    p = ( page * ) malloc( sizeof( struct page ) );
    p->data = i;
    p->stamp = 0;
    p->mod = 0;
    vms->pt[i] = p;
  }

  TLB * tlb;
  vms->tlb = ( TLB ** ) malloc( sizeof( struct TLB ) * sizeTLB );
  if (vms->tlb == NULL){
    free(vms->tlb);
    return NULL;
  }
  for (int i = 0; i < sizeTLB; i++){
    tlb = ( TLB * ) malloc( sizeof( struct TLB ) );
    tlb->data = i;
    tlb->stamp = 0;
    vms->tlb[i] = tlb;
  }

  return vms;
}


void pageReplaceRR(void *handle, int mappedAddress, int structure){

  VMS * vms = handle;
  if (structure == 0){
    TLB * tlb;
    tlb = vms->tlb[vms->tlbIn];
    tlb->data = mappedAddress;
    vms->tlbIn++;

    if (vms->tlbIn == vms->sizeTLB)
      vms->tlbIn = 0;
  }
  else{
    page * p;
    p = vms->pt[vms->ptIn];
    p->data = mappedAddress;

    vms->ptIn++;
    if (vms->ptIn == vms->sizePM)
      vms->ptIn = 0;
  }
}


void repLRU(void *handle, int mappedAddress, int structure){

  VMS * vms = handle;
  int location = 0;
  if (structure == 0){
    TLB * tlb;
    TLB * tlbcomp;

    for (int i = 1; i < vms->sizeTLB; i++){
      tlb = vms->tlb[i];
      tlbcomp = vms->tlb[location];
      if (tlb->stamp < tlbcomp->stamp)
        location = i;
    }
    tlb = vms->tlb[location];
    tlb->stamp = vms->ts;
    tlb->data = mappedAddress;
  }
  else{
    page * p;
    page * pcc;
    for (int i = 1; i < vms->sizePM; i++){
      p = vms->pt[i];
      pcc = vms->pt[location];
      if (p->stamp < pcc->stamp){
        location = i;
      }
    }
    p = vms->pt[location];
    p->stamp = vms->ts;
    p->data = mappedAddress;
  }
}


int readAdjust(void *handle, unsigned int address, int f){

  VMS * vms = handle;
  page * p;
  TLB * tlb;
  vms->ts++;
  int mappedAddress = address / vms->pageSize;

  for (int i = 0; i < vms->sizeTLB; i++){
    tlb = vms->tlb[i];
    if (tlb->data == mappedAddress){
      tlb->stamp = vms->ts;

      for (int j = 0; j < vms->sizePM; j++){
        p = vms->pt[j];
        if (p->data == mappedAddress)
          p->stamp = vms->ts;
      }
      if (f == 1)
        return (float) vms->vm[address];

      return vms->vm[address];
    }
  }
  vms->tlbMisses++;

  for (int i = 0; i < vms->sizePM; i++){
    p = vms->pt[i];
    if (p->data == mappedAddress){
      p->stamp = vms->ts;
      if (vms->tlbRep == 0)
        pageReplaceRR(handle, mappedAddress, 0);
      else
        repLRU(handle, mappedAddress, 0);

      if (f == 1)
        return (float) vms->vm[address];

      return vms->vm[address];
    }
  }

  vms->pageFaults++;

  int location = 0;
  page * pcc;
  for (int i = 1; i < vms->sizePM; i++){
    p = vms->pt[i];
    pcc = vms->pt[location];
    if (p->stamp < pcc->stamp)
      location = i;
  }

  p = vms->pt[location];
  if (p->mod == 1){
    vms->diskWrites++;
    p->mod = 0;
  }

  if (vms->tlbRep == 0)
    pageReplaceRR(handle, mappedAddress, 0);
  else
    repLRU(handle, mappedAddress, 0);

  if (vms->rep == 0)
    pageReplaceRR(handle, mappedAddress, 1);
  else
    repLRU(handle, mappedAddress, 1);

  if (f == 1)
    return (float) vms->vm[address];

  return vms->vm[address];
}

void writeHelper(void *handle, unsigned int address, int value){

  VMS * vms = handle;
  vms->ts++;
  page * p;
  TLB * tlb;
  vms->vm[address] = value;
  int mappedAddress = address / vms->pageSize;

  for (int i = 0; i < vms->sizeTLB; i++){
    tlb = vms->tlb[i];

    if (tlb->data == mappedAddress){
      tlb->stamp = vms->ts;

      for (int j = 0; j < vms->sizePM; j++){
        p = vms->pt[j];
        if (p->data == mappedAddress){
          p->mod = 1;
          p->stamp = vms->ts;
        }
      }
      return;
    }
  }
  vms->tlbMisses++;

  for (int i = 0; i < vms->sizePM; i++){
    p = vms->pt[i];
    if (p->data == mappedAddress){
      p->mod = 1;
      p->stamp = vms->ts;

      if (vms->tlbRep == 0)
        pageReplaceRR(handle, mappedAddress, 0);
      else
        repLRU(handle, mappedAddress, 0);

      return;
    }
  }

  vms->pageFaults++;

  int location = 0;
  page * pcc;
  for (int i = 1; i < vms->sizePM; i++){
    p = vms->pt[i];
    pcc = vms->pt[location];
    if (p->stamp < pcc->stamp)
      location = i;
  }
  vms->ptIn = location;

  p = vms->pt[location];
  if (p->mod == 1){
    vms->diskWrites++;
    p->mod = 0;
  }

  if (vms->tlbRep == 0)
    pageReplaceRR(handle, mappedAddress, 0);
  else
    repLRU(handle, mappedAddress, 0);

  if (vms->rep == 0)
    pageReplaceRR(handle, mappedAddress, 1);
  else
    repLRU(handle, mappedAddress, 1);


  for (int i = 0; i < vms->sizePM; i++){
    p = vms->pt[i];
    if (p->data == mappedAddress)
      p->mod = 1;
  }
}


int readInt(void *handle, unsigned int address){
  return readAdjust(handle, address, 0);
}

float readFloat(void *handle, unsigned int address){
  return readAdjust(handle, address, 1);
}

void writeInt(void *handle, unsigned int address, int value){
  writeHelper(handle, address, value);
}

void writeFloat(void *handle, unsigned int address, float value){
  writeHelper(handle, address, (int) value);
}

void printStatistics(void *handle){
  VMS * vms = handle;
  printf("Number of page faults: %d\n", vms->pageFaults);
  printf("Number of TLB misses: %d\n", vms->tlbMisses);
  printf("Number of disk writes: %d\n", vms->diskWrites);
}


void cleanupVM(void *handle){
  VMS * vms = handle;
  free(vms->vm);
  for(int i = 0; i < vms->sizePM; i++)
    free(vms->pt[i]);
  free(vms->pt);
  for(int i = 0; i < vms->sizeTLB; i++)
    free(vms->tlb[i]);
  free(vms->tlb);
  free(vms);
}
