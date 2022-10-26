#include <stdlib.h>
#include <string.h>
#include "symtab.h"
//
// This is the interface for a generic symbol table. A table stores
// (symbol, data) pairs.
//
// A symbol is simply a C string (null-terminated char sequence).
//
// The data associated with a symbol is simply a void*.
//

// modified FNV hash (see
//http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash)
static unsigned int hash(const char *str)
{
   const unsigned int p = 16777619;
   unsigned int hash = 2166136261u;
   while (*str)
   {
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

typedef struct{
  char* symbol;
  void* data;
  struct entryNode* next;
} entryNode;

typedef struct symbolTab{
  int length;
  void** address;
} symbolTab;

typedef struct{
  void* table;
  unsigned int curVal;
  void* curEnt;
} iterator;

/*
Implement a helper function to do an internal lookup of a symbol in a given table, returning a
pointer to the linked struct containing the symbol, if one is found. The function should be passed
the control struct for a hash table and a const char* pointer to the symbol. (The const keyword
indicates that the function should not change the characters in the string representing the symbol.)
The function should use the hash function to "hash" the symbol to an integer value and then
compute the modulus of that integer and the table's length. The symbol is then searched for on the
linked list at that level in the table. Use the strcmp function to compare symbol strings. (Be
careful, strcmp returns zero when the strings are identical.) If the symbol is found, then return a
pointer to the struct that contains the symbol. If the symbol is not found, then return NULL.
*/
static entryNode *lookup(symbolTab *symtabHandle, const char *symbol)
{
  unsigned int hashSymbol = hash(symbol);
  unsigned int hashHandle = hashSymbol % symtabHandle->length;
  entryNode *n = (entryNode*)symtabHandle->address[hashHandle];
  if(n == NULL)
    return NULL;

  while( n != NULL ){
    if( strcmp(n->symbol, symbol) == 0 )
      return n;
    else
      n = (entryNode*)n->next;
  }
  return NULL;
}

// Creates a symbol table.
// If successful, returns a handle for the new table.
// If memory cannot be allocated for the table, returns NULL.
// The parameter is a hint as to the expected number of (symbol, data)
//   pairs to be stored in the table.
void *symtabCreate(int sizeHint)
{
  symbolTab *hash = malloc(sizeof(symbolTab));
  if( hash == NULL )
    return NULL;

  hash->length = sizeHint;
  hash->address = malloc(sizeof(hash)*sizeHint);
  if(hash->address == NULL){
    free(hash);
    return NULL;
  }
  for(int i = 0; i < hash->length; i++){
    hash->address[i] = NULL;
  }
  return hash;
}

// Deletes a symbol table.
// Reclaims all memory used by the table.
// Note that the memory associate with data items is not reclaimed since
//   the symbol table does not know the actual type of the data. It only
//   manipulates pointers to the data.
// Also note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void symtabDelete(void *symtabHandle)
{
  symbolTab *del = symtabHandle;
  for(int i = 0; i < del->length; i++){
    free(del->address[i]);
  }
  free(del);
}

// Install a (symbol, data) pair in the table.
// If the symbol is already installed in the table, then the data is
//   overwritten.
// If the symbol is not already installed, then space is allocated and
//   a copy is made of the symbol, and the (symbol, data) pair is then
//   installed in the table.
// If successful, returns 1.
// If memory cannot be allocated for a new symbol, then returns 0.
// Note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
int symtabInstall(void *symtabHandle, const char *symbol, void *data)
{
  symbolTab *hashCon = symtabHandle;
  entryNode *look = lookup(hashCon, symbol);

  if( look != NULL )
    look->data = data;
  else
  {
    entryNode *inst = malloc(sizeof(entryNode));
    if( inst == NULL )
      return 0;

    inst->symbol = malloc(sizeof(char) * (strlen(symbol) + 1));
    if( inst->symbol == NULL )
      return 0;

    strcpy(inst->symbol, symbol);
    inst->data = data;

    unsigned int hashSymbol = hash(symbol);
    hashSymbol = hashSymbol % hashCon->length;

    if( hashCon->address[hashSymbol] != NULL ){
      inst->next = hashCon->address[hashSymbol];
      hashCon->address[hashSymbol] = inst;
    }
    else
      hashCon->address[hashSymbol] = inst;

  }
  return 1;
}

// Return the data item stored with the given symbol.
// If the symbol is found, return the associated data item.
// If the symbol is not found, returns NULL.
// Note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void *symtabLookup(void *symtabHandle, const char *symbol)
{
  entryNode* lookSym = lookup(symtabHandle, symbol);
  if(lookSym == NULL)
    return NULL;

  return lookSym->data;
}

// Create an iterator for the contents of the symbol table.
// If successful, a handle to the iterator is returned which can be
// repeatedly passed to symtabNext to iterate over the contents
// of the table.
// If memory cannot be allocated for the iterator, returns NULL.
// Note that no validation is made of the symbol table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void *symtabCreateIterator(void *symtabHandle)
{
  symbolTab *hash = symtabHandle;
  iterator *iter = malloc(sizeof(iterator));

  if(iter == NULL)
    return NULL;

  iter->table = hash;
  //int level = 0;
  for(int i = 0; i < hash->length; i++){
    if(hash->address[i] != NULL){
      iter->curVal = i;
      iter->curEnt = hash->address[i];
      break;
    }
  }

  return iter;
}

// Returns the next (symbol, data) pair for the iterator.
// The symbol is returned as the return value and the data item
// is placed in the location indicated by the second parameter.
// If the whole table has already been traversed then NULL is
//   returned and the location indicated by the second paramter
//   is not modified.
// Note that no validation is made of the iterator table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
// Also note that if there has been a symbtabInstall call since the
//   iterator was created, the behavior is undefined (but probably
//   benign).
const char *symtabNext(void *iteratorHandle, void **returnData)
{
  iterator *iter = iteratorHandle;

  if(iter->curEnt == NULL)
    return NULL;

  symbolTab *con = iter->table;
  entryNode *ent = iter->curEnt;
  *returnData = ent->data;

  if( ent->next != NULL ){
    iter->curEnt = ent->next;
    return ent->symbol;
  }
  else{
    for(int i = iter->curVal + 1; i < con->length; i++){
      if(con->address[i] != NULL){
        iter->curVal = i;
        iter->curEnt = con->address[i];
        return ent->symbol;
      }
    }
    iter->curEnt = NULL;
  }
  return ent->symbol;
}

// Delete the iterator indicated by the only parameter.
// Reclaims all memory used by the iterator.
// Note that no validation is made of the iterator table handle passed
//   in. If not a valid handle, then the behavior is undefined (but
//   probably bad).
void symtabDeleteIterator(void *iteratorHandle)
{
  free(iteratorHandle);
}
