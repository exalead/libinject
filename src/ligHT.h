/******************************************************************************
 *
 *                               The NG Project
 *
 *                            Light HashTable (LigHT)
 *
 *                       Copyright (c) 2007 Exalead S.A.
 *
 *****************************************************************************/

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <sys/types.h>

/** @defgroup LigHT Light Table
 *
 * LigHT is a very light storage table. It's not yet full-featured (collision are
 * not correctly handled), but it provides all the function to write an access
 * elements. Stored data can be associated to automatic callbacks used to
 * autotically free the data. @{
 */

/** An entry of the LigHT table
 */
typedef struct LigHTEntry LigHTEntry;

/** A LigHT table.
 */
typedef struct LigHT LigHT;

/** Callback to clear an entry.
 */
typedef void (LigHTDataFree)(void* entry);

/** Build a new LigHT.
 *
 * The new table is allocated and returned. It must be freed using LigHT_destroy
 *
 * @param initialCap Capacity of the table.
 * @param freeCB     Callback to remove the data (NULL if none).
 * @return A new LigHT
 */
LigHT* LigHT_init(size_t initialCap, LigHTDataFree* freeCB);

/** Free a LigHT.
 *
 * Remove all the elements of the LigHT and free the LigHT.
 *
 * @param table The table to destroy.
 */
void   LigHT_destroy(LigHT* table);

/** Check if the key is present in the table.
 *
 * @param table The table.
 * @param key   The key to look for
 * @param mt    If true, the function must lock its access to the table
 * @return the presence of the key.
 */
bool   LigHT_contains(LigHT* table, int key, bool mt);

/** Get the element associated with the given key.
 *
 * @param table The table
 * @param key   The key to look for
 * @param mt    If true, the function must lock its access to the table
 * @return The value associated with the key, or NULL
 */
void*  LigHT_get(LigHT* table, int key, bool mt);

/** Add an entry corresponding to the given key.
 *
 * If the key is already in the hashtable and the replace flag as not been set,
 * the function will fail.
 *
 * @param table   The HastTable
 * @param key     The key to write
 * @param data    The value associated to the key
 * @param replace If true, replace the existent value if defined
 * @param mt      If true, the function must lock its access to the table
 * @return true if success, false if no available space has been found or if
 *         the key is already in the table.
 */
bool   LigHT_put(LigHT* table, int key, void* data, bool replace, bool mt);

/** Remove an entry from the table.
 *
 * @param table The table.
 * @param key   The key to remove
 * @param mt    If true, the function must lock its access to the table
 * @return true if the key was in the table.
 */
bool   LigHT_remove(LigHT* table, int key, bool mt);

/** Clear the table.
 *
 * @param table The table
 */
void   LigHT_clear(LigHT* table);

/** @} */

#endif
