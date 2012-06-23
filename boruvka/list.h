/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */


#ifndef __BOR_LIST_H__
#define __BOR_LIST_H__

#include <string.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * List
 * =====
 */
struct _bor_list_t {
    struct _bor_list_t *next, *prev;
};
typedef struct _bor_list_t bor_list_t;

/**
 * Simliar struct as bor_list_t but with extra member "mark" which can be
 * used for marking a particular struct.
 * In your code, you can put this struct into your own struct then use
 * borListMAsList() for iterating over list and finaly borListMFromList()
 * for backcast to this struct. (In other words retyping from bor_list_m_t
 * to bor_list_t is safe!).
 */
struct _bor_list_m_t {
    struct _bor_list_m_t *next, *prev;
    int mark;
};
typedef struct _bor_list_m_t bor_list_m_t;

/**
 * Example
 * --------
 *
 * First define struct that will hold head of list and struct that will be
 * member of a list.
 *
 * ~~~~~~
 * struct head_t {
 *     int a, b, c;
 *     ...
 *
 *     bor_list_t head;
 * };
 *
 * struct member_t {
 *     int d, e, f;
 *     ...
 *
 *     bor_list_t list;
 * };
 * ~~~~~~~
 *
 * Then initialize a list and new memeber can be added.
 * ~~~~~~
 * struct head_t head;
 * struct member_t m1, m2, m3;
 *
 * // init head of list
 * borListInit(&head.head);
 *
 * // append members to list
 * borListAppend(&head.head, &m1.list);
 * borListAppend(&head.head, &m2.list);
 * borListAppend(&head.head, &m3.list);
 * ~~~~~~
 *
 * Now you can iterate over list or do anything else.
 * ~~~~~
 * bor_list_t *item;
 * struct member_t *m;
 *
 * BOR_LIST_FOR_EACH(&head.head, item){
 *     m = BOR_LIST_ENTRY(item, struct member_t, list);
 *     ....
 * }
 * ~~~~~
 */

/**
 * Functions and Macros
 * ---------------------
 */


/**
 * Static declaration of a list with initialization.
 * 
 * Example:
 * ~~~~~
 * static BOR_LIST(name);
 * void main()
 * {
 *     bor_list_t *item;
 *
 *     BOR_LIST_FOR_EACH(&name, item){
 *      ...
 *     }
 * }
 */
#define BOR_LIST(name) \
    bor_list_t name = { &name, &name }

/**
 * Get the struct for this entry.
 * {ptr}: the &bor_list_t pointer.
 * {type}: the type of the struct this is embedded in.
 * {member}: the name of the list_struct within the struct.
 */
#define BOR_LIST_ENTRY(ptr, type, member) \
    bor_container_of(ptr, type, member)

#define BOR_LIST_M_ENTRY(ptr, type, member, offset) \
    (type *)((char *)bor_container_of(ptr, type, member) - (sizeof(bor_list_m_t) * offset))

/**
 * Iterates over list.
 */
#define BOR_LIST_FOR_EACH(list, item) \
        for (item = (list)->next; \
             _bor_prefetch((item)->next), item != (list); \
             item = (item)->next)

/**
 * Iterates over list safe against remove of list entry
 */
#define BOR_LIST_FOR_EACH_SAFE(list, item, tmp) \
	    for (item = (list)->next, tmp = (item)->next; \
             item != (list); \
		     item = tmp, tmp = (item)->next)

/**
 * Iterates over list of given type.
 * {pos}:    the type * to use as a loop cursor.
 * {head}:   the head for your list.
 * {member}: the name of the list_struct within the struct.
 */
#define BOR_LIST_FOR_EACH_ENTRY(head, postype, pos, member) \
	for (pos = BOR_LIST_ENTRY((head)->next, postype, member);	\
	     _bor_prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = BOR_LIST_ENTRY(pos->member.next, postype, member))

/**
 * Iterates over list of given type safe against removal of list entry
 * {pos}:	the type * to use as a loop cursor.
 * {n}:		another type * to use as temporary storage
 * {head}:	the head for your list.
 * {member}:the name of the list_struct within the struct.
 */
#define BOR_LIST_FOR_EACH_ENTRY_SAFE(head, postype, pos, ntype, n, member)         \
    for (pos = BOR_LIST_ENTRY((head)->next, postype, member),             \
		 n = BOR_LIST_ENTRY(pos->member.next, postype, member);	\
	     &pos->member != (head); 					\
	     pos = n, n = BOR_LIST_ENTRY(n->member.next, ntype, member))


/**
 * Initialize list.
 */
_bor_inline void borListInit(bor_list_t *l);

/**
 * Returns next element in list. If called on head first element is
 * returned.
 */
_bor_inline bor_list_t *borListNext(bor_list_t *l);

/**
 * Returns previous element in list. If called on head last element is
 * returned.
 */
_bor_inline bor_list_t *borListPrev(bor_list_t *l);

/**
 * Returns true if list is empty.
 */
_bor_inline int borListEmpty(const bor_list_t *head);

/**
 * Appends item to end of the list l.
 */
_bor_inline void borListAppend(bor_list_t *l, bor_list_t *item);

/**
 * Prepends item before first item in list.
 */
_bor_inline void borListPrepend(bor_list_t *l, bor_list_t *item);

/**
 * Removes item from list.
 */
_bor_inline void borListDel(bor_list_t *item);


/**
 * Returns number of items in list - this takes O(n).
 */
_bor_inline size_t borListSize(const bor_list_t *head);


/**
 * Move all items from {src} to {dst}. Items will be appended to dst.
 */
_bor_inline void borListMove(bor_list_t *src, bor_list_t *dst);


/**
 * Retypes given "M" list struct to regular list struct.
 */
_bor_inline bor_list_t *borListMAsList(bor_list_m_t *l);

/**
 * Opposite to borListMAsList().
 */
_bor_inline bor_list_m_t *borListMFromList(bor_list_t *l);




/**** INLINES ****/
_bor_inline void borListInit(bor_list_t *l)
{
    l->next = l;
    l->prev = l;
}

_bor_inline bor_list_t *borListNext(bor_list_t *l)
{
    return l->next;
}

_bor_inline bor_list_t *borListPrev(bor_list_t *l)
{
    return l->prev;
}

_bor_inline int borListEmpty(const bor_list_t *head)
{
    return head->next == head;
}

_bor_inline void borListAppend(bor_list_t *l, bor_list_t *n)
{
    n->prev = l->prev;
    n->next = l;
    l->prev->next = n;
    l->prev = n;
}

_bor_inline void borListPrepend(bor_list_t *l, bor_list_t *n)
{
    n->next = l->next;
    n->prev = l;
    l->next->prev = n;
    l->next = n;
}

_bor_inline void borListDel(bor_list_t *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->next = item;
    item->prev = item;
}

_bor_inline size_t borListSize(const bor_list_t *head)
{
    bor_list_t *item;
    size_t size = 0;

    BOR_LIST_FOR_EACH(head, item){
        size++;
    }

    return size;
}

_bor_inline void borListMove(bor_list_t *src, bor_list_t *dst)
{
    bor_list_t *item;

    while (!borListEmpty(src)){
        item = borListNext(src);
        borListDel(item);
        borListAppend(dst, item);
    }
}

_bor_inline bor_list_t *borListMAsList(bor_list_m_t *l)
{
    return (bor_list_t *)l;
}

_bor_inline bor_list_m_t *borListMFromList(bor_list_t *l)
{
    return (bor_list_m_t *)l;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_LIST_H__ */
