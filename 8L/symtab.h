//
// This is the interface for a generic symbol table. A table stores
// (symbol, data) pairs.
//
// A symbol is simply a C string (null-terminated char sequence).
//
// The data associated with a symbol is simply a void*.
//
// This implementation includes support for concurrent access to the symbol
// table. However, creation, deletion, and iteration are assumed to be
// single-threaded.
//
//

void *symtabCreate(int sizeHint, int lockCount, int useThinLocks);
  // Creates a symbol table.
  // If successful, returns a handle for the new table.
  // If memory cannot be allocated for the table, returns NULL.
  // The first parameter is a hint as to the expected number of (symbol, data)
  //   pairs to be stored in the table.
  // The second parameter specifies the number of locks to use to protect
  //   concurrent access to the table. The hash buckets will be divided
  //   into equal sized groups, with one group for each lock, where
  //   the lock protects its group of buckets. This parameter must not
  //   be greater than the first parameter.
  // The third parameter specifies whether "thin locks" should be used
  //   instead of a normal lock (mutex). If the parameter is 0, then
  //   normal locks are used; otherwise thin locks are used.
  //
  // This routine should only be used in single-threaded mode.
  //

void symtabDelete(void *symtabHandle);
  // Deletes a symbol table.
  // Reclaims all memory used by the table.
  // Note that the memory associate with data items is not reclaimed since
  //   the symbol table does not know the actual type of the data. It only
  //   manipulates pointers to the data.
  // Also note that no validation is made of the symbol table handle passed
  //   in. If not a valid handle, then the behavior is undefined (but
  //   probably bad).
  // This routine should only be used in single-threaded mode.
  //

int symtabUpdate(void *symtabHandle, const char *symbol,
  void *(*func)(void *, void*), void *arg);
  // Update or install a (symbol, data) pair in the table.
  // A lookup of the symbol is first done.
  // If the symbol was not previously installed, then the "func" argument is
  //   called with NULL for its first argument. The return value of this call
  //   is the data to be associated with the symbol in the table. Space is
  //   allocated and a copy is made of the symbol, and the (symbol, data)
  //   pair is then installed in the table.
  // If the symbol was previously installed, then the "func" argument is
  //   called with the symbol's data value for its first argument. The
  //   return value of this call will be used to replace the data value
  //   for the symbol.
  // Whenever the "func" argument is called, the "arg" argument is passed
  //   as its second parameter.
  // If successful, returns 1.
  // If memory cannot be allocated for a new symbol, then returns 0.
  // Note that no validation is made of the symbol table handle passed
  //   in. If not a valid handle, then the behavior is undefined (but
  //   probably bad).
  // This routine is designed to be used in multi-threaded mode. 


void *symtabLookup(void *symtabHandle, const char *symbol);
  // Return the data item stored with the given symbol.
  // If the symbol is found, return the associated data item.
  // If the symbol is not found, returns NULL.
  // Note that no validation is made of the symbol table handle passed
  //   in. If not a valid handle, then the behavior is undefined (but
  //   probably bad).
  // This routine is designed to be used in multi-threaded mode. 

void *symtabCreateIterator(void *symtabHandle);
  // Create an iterator for the contents of the symbol table.
  // If successful, a handle to the iterator is returned which can be
  // repeatedly passed to symtableNext to iterate over the contents
  // of the table.
  // If memory cannot be allocated for the iterator, returns NULL.
  // Note that no validation is made of the symbol table handle passed
  //   in. If not a valid handle, then the behavior is undefined (but
  //   probably bad).
  // This routine should only be used in single-threaded mode.
  //

const char *symtabNext(void *iteratorHandle, void **returnData);
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
  // This routine should only be used in single-threaded mode.
  //

void symtabDeleteIterator(void *iteratorHandle);
  // Delete the iterator indicated by the only parameter.
  // Reclaims all memory used by the iterator.
  // Note that no validation is made of the iterator table handle passed
  //   in. If not a valid handle, then the behavior is undefined (but
  //   probably bad).
  // This routine should only be used in single-threaded mode.
  //

