#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symtab.h"

typedef struct Node {
  char * symbol;
  void * data;
  struct Node * next;
} Node;

typedef struct Symbols {
  int size;
  Node ** table;
} Symbols;

typedef struct Iterator {
  Symbols * symboltable;
  unsigned int index;
  Node * node;
} Iterator;

static unsigned int hash(const char *str) {
  const unsigned int p = 16777619;
  unsigned int hash = 2166136261u;

  while(*str){
    hash = (hash ^ *str) * p;
    str += 1;
  }

  hash += hash << 13;
  hash ^= hash >> 7;
  hash += hash << 3;
  hash ^= hash >> 17;
  hash += hash << 5;
  return hash;
}

void *symtabCreate(int sizeHint){

  Symbols * s = ( Symbols * ) malloc( sizeof( struct Symbols ) );
  if (s == NULL)
    return NULL;

  s->size = sizeHint;
  s->table = ( Node ** ) malloc( sizeof( struct Node ) * sizeHint );
  if (s->table == NULL){
    free(s->table);
    return NULL;
  }
  for (int i = 0; i < sizeHint; i++)
    s->table[i] = NULL;

  return s;
}


void symtabDelete(void *symtabHandle){

  Symbols * s = symtabHandle;
  Node * victim;
  Node * next;

  for(int i = 0; i < s->size; i++){
    if(s->table[i] != NULL){
      victim = s->table[i];
      while(victim != NULL){
        next = victim->next;
        free(victim->symbol);
        free(victim);
        victim = next;
      }
    }
  }

  free(s->table);
  free(s);
}


static Node * symtabLookupHelper(void *symtabHandle, const char *symbol){

  Symbols * s = symtabHandle;
  unsigned int hash_key = hash(symbol);
	unsigned int bucket = hash_key % s->size;

  if (s->table[bucket] == NULL)
    return NULL;

  Node * temp = s->table[bucket];
  int compare;

  while(temp){
    compare = strcmp(temp->symbol, symbol);
    if(compare == 0)
      break;

    temp = temp->next;
  }
  if(temp == NULL){
    return NULL;
  }
  return temp;
}

int symtabInstall(void *symtabHandle, const char *symbol, void *data){

  Symbols * s = symtabHandle;
  Node * n = symtabLookupHelper(s, symbol);
  if (n != NULL){
    n->data = data;
    return 1;
  }

  Node * newnode = (Node *) malloc(sizeof(struct Node));
  if (newnode == NULL)
    return 0;

  newnode->symbol = (char *) malloc((sizeof(char) * strlen(symbol)) + 1);
  if (newnode->symbol == NULL)
    return 0;

  strcpy(newnode->symbol, symbol);
  newnode->data = data;

  unsigned int hash_key = hash(symbol);
	unsigned int bucket = hash_key % s->size;
  if (s->table[bucket] == NULL)
    newnode->next = NULL;
  else
    newnode->next = s->table[bucket];

  s->table[bucket] = newnode;
  return 1;
}

void *symtabLookup(void *symtabHandle, const char *symbol){

  Node * n = symtabLookupHelper(symtabHandle, symbol);
  if (n == NULL)
    return NULL;

  return n->data;
}

void *symtabCreateIterator(void *symtabHandle){

  Symbols * s = symtabHandle;
  Iterator * iter = (Iterator *) malloc(sizeof(struct Iterator));
  if (iter == NULL)
    return NULL;

  iter->symboltable = s;
  iter->node = NULL;

  for(int i = 0; i < s->size; i++){
    if(s->table[i] != NULL){
      iter->index = i;
      iter->node = s->table[i];
      break;
    }
  }
  return iter;
}


const char *symtabNext(void *iteratorHandle, void **returnData){

  Iterator * iter = iteratorHandle;
  if (iter->node == NULL)
    return NULL;

  Node * n = iter->node;
  char * symbol = n->symbol;
  *returnData = n->data;
  if (n->next != NULL)
    iter->node = n->next;

  else{
    Symbols * s = iter->symboltable;
    iter->node = NULL;
    for(int i = iter->index + 1; i < s->size; i++){
      if(s->table[i] != NULL){
        iter->node = s->table[i];
        iter->index = i;
        break;
      }
    }
  }
  return symbol;
}

void symtabDeleteIterator(void *iteratorHandle){
  free(iteratorHandle);
  return;
}
