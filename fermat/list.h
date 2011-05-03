/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */


#ifndef __FER_LIST_H__
#define __FER_LIST_H__

#include <string.h>
#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * List
 * =====
 */
struct _fer_list_t {
    struct _fer_list_t *next, *prev;
};
typedef struct _fer_list_t fer_list_t;

/**
 * Simliar struct as fer_list_t but with extra member "mark" which can be
 * used for marking a particular struct.
 * In your code, you can put this struct into your own struct then use
 * ferListMAsList() for iterating over list and finaly ferListMFromList()
 * for backcast to this struct. (In other words retyping from fer_list_m_t
 * to fer_list_t is safe!).
 */
struct _fer_list_m_t {
    struct _fer_list_m_t *next, *prev;
    int mark;
};
typedef struct _fer_list_m_t fer_list_m_t;

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
 *     fer_list_t head;
 * };
 *
 * struct member_t {
 *     int d, e, f;
 *     ...
 *
 *     fer_list_t list;
 * };
 * ~~~~~~~
 *
 * Then initialize a list and new memeber can be added.
 * ~~~~~~
 * struct head_t head;
 * struct member_t m1, m2, m3;
 *
 * // init head of list
 * ferListInit(&head.head);
 *
 * // append members to list
 * ferListAppend(&head.head, &m1.list);
 * ferListAppend(&head.head, &m2.list);
 * ferListAppend(&head.head, &m3.list);
 * ~~~~~~
 *
 * Now you can iterate over list or do anything else.
 * ~~~~~
 * fer_list_t *item;
 * struct member_t *m;
 *
 * FER_LIST_FOR_EACH(&head.head, item){
 *     m = FER_LIST_ENTRY(item, struct member_t, list);
 *     ....
 * }
 * ~~~~~
 */

/**
 * Functions and Macros
 * ---------------------
 */


/**
 * Get the struct for this entry.
 * {ptr}: the &fer_list_t pointer.
 * {type}: the type of the struct this is embedded in.
 * {member}: the name of the list_struct within the struct.
 */
#define FER_LIST_ENTRY(ptr, type, member) \
    fer_container_of(ptr, type, member)

#define FER_LIST_M_ENTRY(ptr, type, member, offset) \
    (type *)((char *)fer_container_of(ptr, type, member) - (sizeof(fer_list_m_t) * offset))

/**
 * Iterates over list.
 */
#define FER_LIST_FOR_EACH(list, item) \
        for (item = (list)->next; \
             _fer_prefetch((item)->next), item != (list); \
             item = (item)->next)

/**
 * Iterates over list safe against remove of list entry
 */
#define FER_LIST_FOR_EACH_SAFE(list, item, tmp) \
	    for (item = (list)->next, tmp = (item)->next; \
             item != (list); \
		     item = tmp, tmp = (item)->next)

/**
 * Iterates over list of given type.
 * {pos}:    the type * to use as a loop cursor.
 * {head}:   the head for your list.
 * {member}: the name of the list_struct within the struct.
 */
#define FER_LIST_FOR_EACH_ENTRY(head, postype, pos, member) \
	for (pos = FER_LIST_ENTRY((head)->next, postype, member);	\
	     _fer_prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = FER_LIST_ENTRY(pos->member.next, postype, member))

/**
 * Iterates over list of given type safe against removal of list entry
 * {pos}:	the type * to use as a loop cursor.
 * {n}:		another type * to use as temporary storage
 * {head}:	the head for your list.
 * {member}:the name of the list_struct within the struct.
 */
#define FER_LIST_FOR_EACH_ENTRY_SAFE(head, postype, pos, n, ntype, member)         \
    for (pos = FER_LIST_ENTRY((head)->next, postype, member),             \
		 n = FER_LIST_ENTRY(pos->member.next, postype, member);	\
	     &pos->member != (head); 					\
	     pos = n, n = FER_LIST_ENTRY(n->member.next, ntype, member))


/**
 * Initialize list.
 */
_fer_inline void ferListInit(fer_list_t *l);

/**
 * Returns next element in list. If called on head first element is
 * returned.
 */
_fer_inline fer_list_t *ferListNext(fer_list_t *l);

/**
 * Returns previous element in list. If called on head last element is
 * returned.
 */
_fer_inline fer_list_t *ferListPrev(fer_list_t *l);

/**
 * Returns true if list is empty.
 */
_fer_inline int ferListEmpty(const fer_list_t *head);

/**
 * Appends item to end of the list l.
 */
_fer_inline void ferListAppend(fer_list_t *l, fer_list_t *item);

/**
 * Prepends item before first item in list.
 */
_fer_inline void ferListPrepend(fer_list_t *l, fer_list_t *item);

/**
 * Removes item from list.
 */
_fer_inline void ferListDel(fer_list_t *item);


/**
 * Returns number of items in list - this takes O(n).
 */
_fer_inline size_t ferListSize(const fer_list_t *head);



/**
 * Retypes given "M" list struct to regular list struct.
 */
_fer_inline fer_list_t *ferListMAsList(fer_list_m_t *l);

/**
 * Opposite to ferListMAsList().
 */
_fer_inline fer_list_m_t *ferListMFromList(fer_list_t *l);




/**** INLINES ****/
_fer_inline void ferListInit(fer_list_t *l)
{
    l->next = l;
    l->prev = l;
}

_fer_inline fer_list_t *ferListNext(fer_list_t *l)
{
    return l->next;
}

_fer_inline fer_list_t *ferListPrev(fer_list_t *l)
{
    return l->prev;
}

_fer_inline int ferListEmpty(const fer_list_t *head)
{
    return head->next == head;
}

_fer_inline void ferListAppend(fer_list_t *l, fer_list_t *n)
{
    n->prev = l->prev;
    n->next = l;
    l->prev->next = n;
    l->prev = n;
}

_fer_inline void ferListPrepend(fer_list_t *l, fer_list_t *n)
{
    n->next = l->next;
    n->prev = l;
    l->next->prev = n;
    l->next = n;
}

_fer_inline void ferListDel(fer_list_t *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->next = item;
    item->prev = item;
}

_fer_inline size_t ferListSize(const fer_list_t *head)
{
    fer_list_t *item;
    size_t size = 0;

    FER_LIST_FOR_EACH(head, item){
        size++;
    }

    return size;
}

_fer_inline fer_list_t *ferListMAsList(fer_list_m_t *l)
{
    return (fer_list_t *)l;
}

_fer_inline fer_list_m_t *ferListMFromList(fer_list_t *l)
{
    return (fer_list_m_t *)l;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_LIST_H__ */
