#ifndef _DEPOOLSET_H
#define _DEPOOLSET_H
/*-------------------------------------------------------------------------
 * drawElements Memory Pool Library
 * --------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Memory pool set class.
 *//*--------------------------------------------------------------------*/

#include "deDefs.h"
#include "deMemPool.h"
#include "dePoolArray.h"
#include "deInt32.h"

#include <string.h> /* memset() */

enum
{
	DE_SET_ELEMENTS_PER_SLOT	= 4
};

DE_BEGIN_EXTERN_C

void	dePoolSet_selfTest		(void);

DE_END_EXTERN_C

/*--------------------------------------------------------------------*//*!
 * \brief Declare a template pool set class interface.
 * \param TYPENAME	Type name of the declared set.
 * \param KEYTYPE	Type of the key.
 *
 * This macro declares the interface for a set. For the implementation of
 * the set, see DE_IMPLEMENT_POOL_HASH. Usually this macro is put into the
 * header file and the implementation macro is put in some .c file.
 *
 * \todo [petri] Detailed description.
 *
 * The functions for operating the set are:
 * \todo [petri] Figure out how to comment these in Doxygen-style.
 *
 * \code
 * Set*     Set_create            (deMemPool* pool);
 * int      Set_getNumElements    (const Set* array);
 * deBool   Set_exists            (const Set* array, Key key);
 * deBool   Set_insert            (Set* array, Key key);
 * void     Set_delete            (Set* array, Key key);
 * \endcode
*//*--------------------------------------------------------------------*/
#define DE_DECLARE_POOL_SET(TYPENAME, KEYTYPE)		\
\
typedef struct TYPENAME##Slot_s TYPENAME##Slot;    \
\
struct TYPENAME##Slot_s \
{    \
	int				numUsed; \
	TYPENAME##Slot*	nextSlot; \
	KEYTYPE			keys[DE_SET_ELEMENTS_PER_SLOT]; \
}; \
\
typedef struct TYPENAME##_s    \
{    \
	deMemPool*			pool;    \
	int					numElements;    \
\
	int					slotTableSize;    \
	TYPENAME##Slot**	slotTable;		\
	TYPENAME##Slot*		slotFreeList;		\
} TYPENAME;    \
\
typedef struct TYPENAME##Iter_s \
{	\
	const TYPENAME*			hash;			\
	int						curSlotIndex;	\
	const TYPENAME##Slot*	curSlot;		\
	int						curElemIndex;	\
} TYPENAME##Iter;	\
\
TYPENAME*	TYPENAME##_create		(deMemPool* pool);    \
void		TYPENAME##_reset		(TYPENAME* set);    \
deBool		TYPENAME##_reserve		(TYPENAME* set, int capacity);    \
deBool		TYPENAME##_exists		(const TYPENAME* set, KEYTYPE key);    \
deBool		TYPENAME##_insert		(TYPENAME* set, KEYTYPE key);    \
void		TYPENAME##_delete		(TYPENAME* set, KEYTYPE key);    \
\
DE_INLINE int		TYPENAME##_getNumElements	(const TYPENAME* set)							DE_UNUSED_FUNCTION;	\
DE_INLINE void		TYPENAME##Iter_init			(const TYPENAME* hash, TYPENAME##Iter* iter)	DE_UNUSED_FUNCTION;	\
DE_INLINE deBool	TYPENAME##Iter_hasItem		(const TYPENAME##Iter* iter)					DE_UNUSED_FUNCTION;	\
DE_INLINE void		TYPENAME##Iter_next			(TYPENAME##Iter* iter)							DE_UNUSED_FUNCTION;	\
DE_INLINE KEYTYPE	TYPENAME##Iter_getKey		(const TYPENAME##Iter* iter)					DE_UNUSED_FUNCTION;	\
DE_INLINE deBool	TYPENAME##_safeInsert		(TYPENAME* set, KEYTYPE key)					DE_UNUSED_FUNCTION;	\
DE_INLINE void		TYPENAME##_safeDelete		(TYPENAME* set, KEYTYPE key)					DE_UNUSED_FUNCTION;	\
\
DE_INLINE int TYPENAME##_getNumElements (const TYPENAME* set)    \
{    \
	return set->numElements;    \
}    \
\
DE_INLINE void TYPENAME##Iter_init (const TYPENAME* hash, TYPENAME##Iter* iter)    \
{	\
	iter->hash			= hash;		\
	iter->curSlotIndex	= 0;		\
	iter->curSlot		= DE_NULL;	\
	iter->curElemIndex	= 0;		\
	if (TYPENAME##_getNumElements(hash) > 0)		\
	{												\
		int slotTableSize	= hash->slotTableSize;	\
		int slotNdx			= 0;					\
		while (slotNdx < slotTableSize)				\
		{											\
			if (hash->slotTable[slotNdx])			\
				break;								\
			slotNdx++;								\
		}											\
		DE_ASSERT(slotNdx < slotTableSize);			\
		iter->curSlotIndex = slotNdx;				\
		iter->curSlot = hash->slotTable[slotNdx];	\
		DE_ASSERT(iter->curSlot);					\
	}	\
}	\
\
DE_INLINE deBool TYPENAME##Iter_hasItem	(const TYPENAME##Iter* iter)    \
{	\
	return (iter->curSlot != DE_NULL); \
}	\
\
DE_INLINE void TYPENAME##Iter_next (TYPENAME##Iter* iter)    \
{	\
	DE_ASSERT(TYPENAME##Iter_hasItem(iter));	\
	if (++iter->curElemIndex == iter->curSlot->numUsed)	\
	{													\
		iter->curElemIndex = 0;							\
		if (iter->curSlot->nextSlot)					\
		{												\
			iter->curSlot = iter->curSlot->nextSlot;	\
		}												\
		else											\
		{												\
			const TYPENAME*	hash			= iter->hash;			\
			int				curSlotIndex	= iter->curSlotIndex;	\
			int				slotTableSize	= hash->slotTableSize;	\
			while (++curSlotIndex < slotTableSize)		\
			{											\
				if (hash->slotTable[curSlotIndex])		\
					break;								\
			}											\
			iter->curSlotIndex = curSlotIndex;			\
			if (curSlotIndex < slotTableSize)					\
				iter->curSlot = hash->slotTable[curSlotIndex];	\
			else												\
				iter->curSlot = DE_NULL;						\
		}	\
	}	\
}	\
\
DE_INLINE KEYTYPE TYPENAME##Iter_getKey	(const TYPENAME##Iter* iter)    \
{	\
	DE_ASSERT(TYPENAME##Iter_hasItem(iter));	\
	return iter->curSlot->keys[iter->curElemIndex];	\
}	\
\
DE_INLINE deBool TYPENAME##_safeInsert (TYPENAME* set, KEYTYPE key)	\
{																	\
	DE_ASSERT(set);													\
	if (TYPENAME##_exists(set, key))								\
		return DE_TRUE;												\
	return TYPENAME##_insert(set, key);								\
}																	\
\
DE_INLINE void TYPENAME##_safeDelete (TYPENAME* set, KEYTYPE key)	\
{																	\
	DE_ASSERT(set);													\
	if (TYPENAME##_exists(set, key))								\
		TYPENAME##_delete(set, key);								\
}																	\
\
struct TYPENAME##Dummy_s { int dummy; }

/*--------------------------------------------------------------------*//*!
 * \brief Implement a template pool set class.
 * \param TYPENAME	Type name of the declared set.
 * \param KEYTYPE	Type of the key.
 * \param HASHFUNC	Function used for hashing the key.
 * \param CMPFUNC	Function used for exact matching of the keys.
 *
 * This macro has implements the set declared with DE_DECLARE_POOL_SET.
 * Usually this macro should be used from a .c file, since the macro expands
 * into multiple functions. The TYPENAME and KEYTYPE parameters
 * must match those of the declare macro.
*//*--------------------------------------------------------------------*/
#define DE_IMPLEMENT_POOL_SET(TYPENAME, KEYTYPE, HASHFUNC, CMPFUNC)		\
\
TYPENAME* TYPENAME##_create (deMemPool* pool)    \
{   \
	/* Alloc struct. */ \
	TYPENAME* set = DE_POOL_NEW(pool, TYPENAME); \
	if (!set) \
		return DE_NULL; \
\
	/* Init array. */ \
	memset(set, 0, sizeof(TYPENAME)); \
	set->pool = pool; \
\
	return set; \
} \
\
void TYPENAME##_reset (TYPENAME* set)    \
{   \
	int slotNdx; \
	for (slotNdx = 0; slotNdx < set->slotTableSize; slotNdx++)	\
	{	\
		TYPENAME##Slot* slot = set->slotTable[slotNdx]; \
		while (slot) \
		{ \
			TYPENAME##Slot*	nextSlot = slot->nextSlot;	\
			slot->nextSlot = set->slotFreeList;		\
			set->slotFreeList = slot;	\
			slot->numUsed = 0;			\
			slot = nextSlot;			\
		}	\
		set->slotTable[slotNdx] = DE_NULL; \
	}	\
	set->numElements = 0; \
}	\
\
TYPENAME##Slot* TYPENAME##_allocSlot (TYPENAME* set)    \
{   \
	TYPENAME##Slot* slot; \
	if (set->slotFreeList) \
	{ \
		slot = set->slotFreeList; \
		set->slotFreeList = set->slotFreeList->nextSlot; \
	} \
	else \
		slot = (TYPENAME##Slot*)deMemPool_alloc(set->pool, sizeof(TYPENAME##Slot) * DE_SET_ELEMENTS_PER_SLOT); \
\
	if (slot) \
	{ \
		slot->nextSlot = DE_NULL; \
		slot->numUsed = 0; \
	} \
\
	return slot; \
} \
\
deBool TYPENAME##_rehash (TYPENAME* set, int newSlotTableSize)    \
{    \
	DE_ASSERT(deIsPowerOfTwo32(newSlotTableSize) && newSlotTableSize > 0); \
	if (newSlotTableSize > set->slotTableSize)    \
	{ \
		TYPENAME##Slot**	oldSlotTable = set->slotTable; \
		TYPENAME##Slot**	newSlotTable = (TYPENAME##Slot**)deMemPool_alloc(set->pool, sizeof(TYPENAME##Slot*) * newSlotTableSize); \
		int					oldSlotTableSize = set->slotTableSize; \
		int					slotNdx; \
\
		if (!newSlotTable) \
			return DE_FALSE; \
\
		for (slotNdx = 0; slotNdx < oldSlotTableSize; slotNdx++) \
			newSlotTable[slotNdx] = oldSlotTable[slotNdx]; \
\
		for (slotNdx = oldSlotTableSize; slotNdx < newSlotTableSize; slotNdx++) \
			newSlotTable[slotNdx] = DE_NULL; \
\
		set->slotTableSize		= newSlotTableSize; \
		set->slotTable			= newSlotTable; \
\
		for (slotNdx = 0; slotNdx < oldSlotTableSize; slotNdx++) \
		{ \
			TYPENAME##Slot* slot = oldSlotTable[slotNdx]; \
			newSlotTable[slotNdx] = DE_NULL; \
			while (slot) \
			{ \
				int elemNdx; \
				for (elemNdx = 0; elemNdx < slot->numUsed; elemNdx++) \
				{ \
					set->numElements--; \
					if (!TYPENAME##_insert(set, slot->keys[elemNdx])) \
						return DE_FALSE; \
				} \
				slot = slot->nextSlot; \
			} \
		} \
	} \
\
	return DE_TRUE;    \
}    \
\
deBool TYPENAME##_exists (const TYPENAME* set, KEYTYPE key)    \
{    \
	if (set->numElements > 0) \
	{	\
		int				slotNdx	= HASHFUNC(key) & (set->slotTableSize - 1); \
		TYPENAME##Slot*	slot	= set->slotTable[slotNdx]; \
		DE_ASSERT(deInBounds32(slotNdx, 0, set->slotTableSize)); \
	\
		while (slot) \
		{ \
			int elemNdx; \
			for (elemNdx = 0; elemNdx < slot->numUsed; elemNdx++) \
			{ \
				if (CMPFUNC(slot->keys[elemNdx], key)) \
					return DE_TRUE; \
			} \
			slot = slot->nextSlot; \
		} \
	} \
\
	return DE_FALSE; \
}    \
\
deBool TYPENAME##_insert (TYPENAME* set, KEYTYPE key)    \
{    \
	int				slotNdx; \
	TYPENAME##Slot*	slot; \
\
	DE_ASSERT(set); \
	DE_ASSERT(!TYPENAME##_exists(set, key)); \
\
	if ((set->numElements + 1) >= set->slotTableSize * DE_SET_ELEMENTS_PER_SLOT) \
		if (!TYPENAME##_rehash(set, deMax32(4, 2*set->slotTableSize))) \
			return DE_FALSE; \
\
	slotNdx	= HASHFUNC(key) & (set->slotTableSize - 1); \
	DE_ASSERT(slotNdx >= 0 && slotNdx < set->slotTableSize); \
	slot	= set->slotTable[slotNdx]; \
\
	if (!slot) \
	{ \
		slot = TYPENAME##_allocSlot(set); \
		if (!slot) return DE_FALSE; \
		set->slotTable[slotNdx] = slot; \
	} \
\
	for (;;) \
	{ \
		if (slot->numUsed == DE_SET_ELEMENTS_PER_SLOT) \
		{ \
			if (slot->nextSlot) \
				slot = slot->nextSlot; \
			else \
			{ \
				TYPENAME##Slot* nextSlot = TYPENAME##_allocSlot(set); \
				if (!nextSlot) return DE_FALSE; \
				slot->nextSlot = nextSlot; \
				slot = nextSlot; \
			} \
		} \
		else \
		{ \
			slot->keys[slot->numUsed]	= key; \
			slot->numUsed++; \
			set->numElements++; \
			return DE_TRUE; \
		} \
	} \
} \
\
void TYPENAME##_delete (TYPENAME* set, KEYTYPE key)    \
{    \
	int				slotNdx; \
	TYPENAME##Slot*	slot; \
	TYPENAME##Slot*	prevSlot = DE_NULL; \
\
	DE_ASSERT(set->numElements > 0); \
	slotNdx	= HASHFUNC(key) & (set->slotTableSize - 1); \
	DE_ASSERT(slotNdx >= 0 && slotNdx < set->slotTableSize); \
	slot	= set->slotTable[slotNdx]; \
	DE_ASSERT(slot); \
\
	for (;;) \
	{ \
		int elemNdx; \
		DE_ASSERT(slot->numUsed > 0); \
		for (elemNdx = 0; elemNdx < slot->numUsed; elemNdx++) \
		{ \
			if (CMPFUNC(key, slot->keys[elemNdx])) \
			{ \
				TYPENAME##Slot*	lastSlot = slot; \
				while (lastSlot->nextSlot) \
				{ \
					prevSlot = lastSlot; \
					lastSlot = lastSlot->nextSlot; \
				} \
\
				slot->keys[elemNdx] = lastSlot->keys[lastSlot->numUsed-1]; \
				lastSlot->numUsed--; \
\
				if (lastSlot->numUsed == 0) \
				{ \
					if (prevSlot) \
						prevSlot->nextSlot = DE_NULL; \
					else \
						set->slotTable[slotNdx] = DE_NULL; \
\
					lastSlot->nextSlot = set->slotFreeList; \
					set->slotFreeList = lastSlot; \
				} \
\
				set->numElements--; \
				return; \
			} \
		} \
\
		prevSlot = slot; \
		slot = slot->nextSlot; \
		DE_ASSERT(slot); \
	} \
}    \
\
struct TYPENAME##Dummy2_s { int dummy; }

/* Copy-to-array templates. */

#define DE_DECLARE_POOL_SET_TO_ARRAY(SETTYPENAME, ARRAYTYPENAME)		\
	deBool SETTYPENAME##_copyToArray(const SETTYPENAME* set, ARRAYTYPENAME* array);	\
	struct SETTYPENAME##_##ARRAYTYPENAME##_declare_dummy { int dummy; }

#define DE_IMPLEMENT_POOL_SET_TO_ARRAY(SETTYPENAME, ARRAYTYPENAME)		\
	deBool SETTYPENAME##_copyToArray(const SETTYPENAME* set, ARRAYTYPENAME* array)	\
	{	\
		int numElements	= set->numElements;	\
		int arrayNdx	= 0;	\
		int slotNdx;	\
\
		if (!ARRAYTYPENAME##_setSize(array, numElements))	\
			return DE_FALSE;	\
\
		for (slotNdx = 0; slotNdx < set->slotTableSize; slotNdx++) \
		{ \
			const SETTYPENAME##Slot* slot = set->slotTable[slotNdx]; \
			while (slot) \
			{ \
				int elemNdx; \
				for (elemNdx = 0; elemNdx < slot->numUsed; elemNdx++) \
					ARRAYTYPENAME##_set(array, arrayNdx++, slot->keys[elemNdx]); \
				slot = slot->nextSlot; \
			} \
		}	\
		DE_ASSERT(arrayNdx == numElements);	\
		return DE_TRUE;	\
	}	\
	struct SETTYPENAME##_##ARRAYTYPENAME##_implement_dummy { int dummy; }

/*--------------------------------------------------------------------*//*!
 * \brief Declare set-wise operations for a set template.
 * \param TYPENAME	Type name of the declared set.
 * \param KEYTYPE	Type of the key.
 *
 * This macro declares union and intersection operations for a set.
 * For implementation see DE_IMPLEMENT_POOL_SET_UNION_INTERSECT.
 *
 * \todo [petri] Detailed description.
 *
 * The functions for operating the set are:
 * \todo [petri] Figure out how to comment these in Doxygen-style.
 *
 * \code
 * deBool	Set_union				(Set* to, const Set* a, const Set* b);
 * deBool	Set_unionInplace		(Set* a, const Set* b);
 * deBool	Set_intersect			(Set* to, const Set* a, const Set* b);
 * void		Set_intersectInplace	(Set* a, const Set* b);
 * deBool   Set_difference			(Set* to, const Set* a, const Set* b);
 * void     Set_differenceInplace	(Set* a, const Set* b);
 * \endcode
*//*--------------------------------------------------------------------*/
#define DE_DECLARE_POOL_SET_SETWISE_OPERATIONS(TYPENAME)	\
	deBool TYPENAME##_union (TYPENAME* to, const TYPENAME* a, const TYPENAME* b);	\
	deBool TYPENAME##_unionInplace (TYPENAME* a, const TYPENAME* b);	\
	deBool TYPENAME##_intersect (TYPENAME* to, const TYPENAME* a, const TYPENAME* b);	\
	void TYPENAME##_intersectInplace (TYPENAME* a, const TYPENAME* b);	\
	deBool TYPENAME##_difference (TYPENAME* to, const TYPENAME* a, const TYPENAME* b);	\
	void TYPENAME##_differenceInplace (TYPENAME* a, const TYPENAME* b);	\
	struct TYPENAME##SetwiseDeclareDummy_s { int dummy; }

#define DE_IMPLEMENT_POOL_SET_SETWISE_OPERATIONS(TYPENAME, KEYTYPE)	\
deBool TYPENAME##_union (TYPENAME* to, const TYPENAME* a, const TYPENAME* b)	\
{	\
	TYPENAME##_reset(to);	\
	if (!TYPENAME##_unionInplace(to, a))	\
		return DE_FALSE;	\
	if (!TYPENAME##_unionInplace(to, b))	\
		return DE_FALSE;	\
	return DE_TRUE;	\
}	\
\
deBool TYPENAME##_unionInplace (TYPENAME* a, const TYPENAME* b)	\
{	\
	TYPENAME##Iter iter;	\
	for (TYPENAME##Iter_init(b, &iter);	\
		 TYPENAME##Iter_hasItem(&iter);	\
		 TYPENAME##Iter_next(&iter))	\
	{	\
		KEYTYPE key = TYPENAME##Iter_getKey(&iter);	\
		if (!TYPENAME##_exists(a, key))	\
		{	\
			if (!TYPENAME##_insert(a, key))	\
				return DE_FALSE;	\
		}	\
	}	\
	return DE_TRUE;	\
}	\
\
deBool TYPENAME##_intersect (TYPENAME* to, const TYPENAME* a, const TYPENAME* b)	\
{	\
	TYPENAME##Iter iter;	\
	TYPENAME##_reset(to);	\
	for (TYPENAME##Iter_init(a, &iter);	\
		 TYPENAME##Iter_hasItem(&iter);	\
		 TYPENAME##Iter_next(&iter))	\
	{	\
		KEYTYPE key = TYPENAME##Iter_getKey(&iter);	\
		if (TYPENAME##_exists(b, key))	\
		{	\
			if (!TYPENAME##_insert(to, key))	\
				return DE_FALSE;	\
		}	\
	}	\
	return DE_TRUE;	\
}	\
\
void TYPENAME##_intersectInplace (TYPENAME* a, const TYPENAME* b)	\
{	\
	DE_UNREF(a && b);	\
	DE_ASSERT(!"Not implemented.");	\
}	\
\
deBool TYPENAME##_difference (TYPENAME* to, const TYPENAME* a, const TYPENAME* b)	\
{	\
	TYPENAME##Iter iter;	\
	TYPENAME##_reset(to);	\
	for (TYPENAME##Iter_init(a, &iter);	\
		 TYPENAME##Iter_hasItem(&iter);	\
		 TYPENAME##Iter_next(&iter))	\
	{	\
		KEYTYPE key = TYPENAME##Iter_getKey(&iter);	\
		if (!TYPENAME##_exists(b, key))	\
		{	\
			if (!TYPENAME##_insert(to, key))	\
				return DE_FALSE;	\
		}	\
	}	\
	return DE_TRUE;	\
}	\
\
void TYPENAME##_differenceInplace (TYPENAME* a, const TYPENAME* b)	\
{	\
	TYPENAME##Iter iter;	\
	for (TYPENAME##Iter_init(b, &iter);	\
		 TYPENAME##Iter_hasItem(&iter);	\
		 TYPENAME##Iter_next(&iter))	\
	{	\
		KEYTYPE key = TYPENAME##Iter_getKey(&iter);	\
		if (TYPENAME##_exists(a, key))	\
			TYPENAME##_delete(a, key);	\
	}	\
}	\
\
struct TYPENAME##UnionIntersectImplementDummy_s { int dummy; }

#endif /* _DEPOOLSET_H */
