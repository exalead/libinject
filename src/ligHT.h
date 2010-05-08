/******************************************************************************/
/*                          libinject                                         */
/*                                                                            */
/*  Redistribution and use in source and binary forms, with or without        */
/*  modification, are permitted provided that the following conditions        */
/*  are met:                                                                  */
/*                                                                            */
/*  1. Redistributions of source code must retain the above copyright         */
/*     notice, this list of conditions and the following disclaimer.          */
/*  2. Redistributions in binary form must reproduce the above copyright      */
/*     notice, this list of conditions and the following disclaimer in the    */
/*     documentation and/or other materials provided with the distribution.   */
/*  3. The names of its contributors may not be used to endorse or promote    */
/*     products derived from this software without specific prior written     */
/*     permission.                                                            */
/*                                                                            */
/*  THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS   */
/*  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED         */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/*  DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY         */
/*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/*  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/*  POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                            */
/*   Copyright (c) 2007-2010 Exalead S.A.                                     */
/******************************************************************************/

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
