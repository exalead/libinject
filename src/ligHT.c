/******************************************************************************
 *
 *                               The NG Project
 *
 *                          Light HashTable (LigHT)
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "ligHT.h"


#define ACQUIRE_READ                                                          \
  if (mt) {                                                                   \
    pthread_rwlock_rdlock(&table->lock);                                      \
  }
#define ACQUIRE_WRITE                                                         \
  if (mt) {                                                                   \
    pthread_rwlock_wrlock(&table->lock);                                      \
  }
#define RELEASE                                                               \
  if (mt) {                                                                   \
    pthread_rwlock_unlock(&table->lock);                                      \
  }

#define FREE_ENTRY(entry)                                                     \
  if (entry && entry->key != -1) {                                            \
    if (table->free) {                                                        \
      table->free(entry->data);                                               \
    }                                                                         \
    entry->data = NULL;                                                       \
    entry->key  = -1;                                                         \
    --table->length;                                                          \
  }

#define GET_OFFSET(key) ((unsigned int)(key))%(table->capacity);

/** An entry of the LigHT
 */
struct LigHTEntry {
  int   key;  /**< The key of the entry */
  void* data; /**< Pointer to the associated data */
};

/** A LigHT.
 */
struct LigHT {
  size_t      capacity;   /**< Capacity of the table */
  size_t      length;     /**< Number of elements in the table */

  LigHTEntry* data;       /**< UserData */
  LigHTDataFree* free;    /**< Callback to call to free user data */

  pthread_rwlock_t lock;  /**< RWLock for multithread access */
};


/** Change the capacity of the LigHT.
 *
 * @param table  The table
 * @param newCap The new capacity of the table
 * @param mt     If true, access to the table are mutexed.
 * @return success.
 */
static bool LigHT_grow(LigHT* table, size_t newCap, bool mt) {
  LigHTEntry* newPtr = NULL;
  ACQUIRE_WRITE
  if (table->capacity < newCap) {
    newPtr = (LigHTEntry*)realloc(table->data, newCap * sizeof(LigHTEntry));
    if (newPtr || newCap == 0) {
      table->data     = newPtr;
      if (newPtr) {
        int i;
        for (i = table->capacity ; i < (int)newCap ; ++i) {
          table->data[i].key  = -1;
          table->data[i].data = NULL;
        }
        for (i = 0 ; i < (int)table->capacity ; ++i) {
          unsigned int pos = i;
          unsigned int key = newPtr[pos].key;
          void* data = newPtr[pos].data;
          while (newPtr[pos].key != -1) {
            unsigned int next = (key % newCap);
            if (next == pos) {
              break;
            }
            key  = newPtr[next].key;
            data = newPtr[next].data;
            newPtr[next].key  = newPtr[pos].key;
            newPtr[next].data = newPtr[pos].data;
            pos = next;
          }
        }
      }
      table->capacity = newCap;
    }
  } else {
    newPtr = table->data;
  }
  RELEASE
  return (newPtr || newCap == 0);
}

LigHT* LigHT_init(size_t initialCap, LigHTDataFree* freeCB) {
  LigHT* table;
  table = (LigHT*)malloc(sizeof(LigHT));
  if (!table) {
    return NULL;
  }
  table->length   = 0;
  table->data     = NULL;
  table->capacity = 0;
  table->free     = freeCB;
  pthread_rwlock_init(&table->lock, NULL);

  (void)LigHT_grow(table, initialCap, false);

  return table;
}

void LigHT_destroy(LigHT* table) {
  LigHT_clear(table);
  free(table->data);
  free(table);
}

/** Find the entry corresponding to the given key.
 *
 * @param table The HastTable
 * @param key   The key to look for
 * @param force If true, and the key is not present,
 *              the function will return a pointer to the next
 *              free entry (if exists) instead of NULL.
 * @return NULL if not found, or a pointer to the corresponding entry.
 */
static inline LigHTEntry* LigHT_findEntry(LigHT* table, int key, bool force) {
  off_t offset, expected;

  expected = offset = GET_OFFSET(key);
  do {
    LigHTEntry* pos;
    pos = table->data + offset;
    if (pos->key == key) {
      return pos;
    } else if (pos->key == -1) {
      return force ? pos : NULL;
    }
    if (++offset == (off_t)table->capacity) {
      offset = 0;
    }
  } while (expected != offset);
  return NULL;
}

bool LigHT_put(LigHT* table, int key, void* data, bool replace, bool mt) {
  LigHTEntry* entry;
  ACQUIRE_WRITE
  {
    entry = LigHT_findEntry(table, key, true);
    if (entry && entry->key == key && !replace) {
      entry = NULL;
    } else if (entry) {
      FREE_ENTRY(entry);
      entry->key  = key;
      entry->data = data;
      ++table->length;
    }
  }
  RELEASE
  return entry != NULL;
}

bool LigHT_contains(LigHT* table, int key, bool mt) {
  LigHTEntry* entry;
  ACQUIRE_READ
  {
    entry = LigHT_findEntry(table, key, false);
  }
  RELEASE
  return entry != NULL;
}

void* LigHT_get(LigHT* table, int key, bool mt) {
  LigHTEntry* entry;
  ACQUIRE_READ
  {
    entry = LigHT_findEntry(table, key, false);
  }
  RELEASE
  return entry == NULL ? NULL : entry->data;
}

bool LigHT_remove(LigHT* table, int key, bool mt) {
  LigHTEntry* entry;
  ACQUIRE_WRITE
  {
    entry = LigHT_findEntry(table, key, false);
    FREE_ENTRY(entry);
  }
  RELEASE
  return entry != NULL;
}

void LigHT_clear(LigHT* table) {
  off_t i;
  for (i = 0 ; i < (off_t)table->length ; ++i) {
    LigHTEntry* entry = table->data + i;
    FREE_ENTRY(entry);
  }
}
