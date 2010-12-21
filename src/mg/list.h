/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */


#ifndef __MG_LIST_H__
#define __MG_LIST_H__

#include <string.h>
#include <mg/core.h>

struct _mg_list_t {
    struct _mg_list_t *next, *prev;
};
typedef struct _mg_list_t mg_list_t;



/**
 * Get the struct for this entry.
 * @ptr:	the &mg_list_t pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define mgListEntry(ptr, type, member) \
    mg_container_of(ptr, type, member)

/**
 * Iterates over list.
 */
#define mgListForEach(list, item) \
        for (item = (list)->next; \
             _mg_prefetch((item)->next), item != (list); \
             item = (item)->next)

/**
 * Iterates over list safe against remove of list entry
 */
#define mgListForEachSafe(list, item, tmp) \
	    for (item = (list)->next, tmp = (item)->next; \
             item != (list); \
		     item = tmp, tmp = (item)->next)

/**
 * Iterates over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define mgListForEachEntry(head, postype, pos, member) \
	for (pos = mgListEntry((head)->next, postype, member);	\
	     _mg_prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = mgListEntry(pos->member.next, postype, member))

/**
 * Iterates over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define mgListForEachEntrySafe(head, postype, pos, n, ntype, member)         \
    for (pos = mgListEntry((head)->next, postype, member),             \
		 n = mgListEntry(pos->member.next, postype, member);	\
	     &pos->member != (head); 					\
	     pos = n, n = mgListEntry(n->member.next, ntype, member))


/**
 * Initialize list.
 */
_mg_inline void mgListInit(mg_list_t *l);

_mg_inline mg_list_t *mgListNext(mg_list_t *l);
_mg_inline mg_list_t *mgListPrev(mg_list_t *l);

/**
 * Returns true if list is empty.
 */
_mg_inline int mgListEmpty(const mg_list_t *head);

/**
 * Appends item to end of the list l.
 */
_mg_inline void mgListAppend(mg_list_t *l, mg_list_t *item);

/**
 * Removes item from list.
 */
_mg_inline void mgListDel(mg_list_t *item);


/**
 * Returns number of items in list - this takes O(n).
 */
_mg_inline size_t mgListSize(const mg_list_t *head);


///
/// INLINES:
///

_mg_inline void mgListInit(mg_list_t *l)
{
    l->next = l;
    l->prev = l;
}

_mg_inline mg_list_t *mgListNext(mg_list_t *l)
{
    return l->next;
}

_mg_inline mg_list_t *mgListPrev(mg_list_t *l)
{
    return l->prev;
}

_mg_inline int mgListEmpty(const mg_list_t *head)
{
    return head->next == head;
}

_mg_inline void mgListAppend(mg_list_t *l, mg_list_t *new)
{
    new->prev = l->prev;
    new->next = l;
    l->prev->next = new;
    l->prev = new;
}

_mg_inline void mgListDel(mg_list_t *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->next = item;
    item->prev = item;
}

_mg_inline size_t mgListSize(const mg_list_t *head)
{
    mg_list_t *item;
    size_t size = 0;

    mgListForEach(head, item){
        size++;
    }

    return size;
}
#endif /* __MG_LIST_H__ */
