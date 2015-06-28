/******************************************************************************
 * list.h
 * Descripton:
 *      This is a simple unbounded array implementation, called a LIST
 *
 * Author: Hasan Al-Jawaheri <hbj@qatar.cmu.edu>
 ****************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

typedef struct LIST_tag {
    void** list;
    int num_elems, size;
} *LIST;

LIST list_create(void);
void list_free(LIST l);
void list_add(LIST l, void* data);
void list_remove(LIST l, int index);
void list_modify(LIST l, int index, void* value);
int list_find(LIST l, void* value, int (*comp)(void*, void*));
void* list_get(LIST l, int index);
int list_size(LIST l);
void list_dump(LIST l);
void list_dump_int(LIST l);

int generic_finder(void* e1, void* e2);
int string_finder(void* e1, void* e2);

#endif //__LIST_H__