#include "list.h"

LIST list_create(void) {
    LIST l = (LIST)malloc(sizeof(LIST));
    l->size = 10;
    l->list = (void**)malloc(sizeof(void*)*l->size);
    l->num_elems = 0;
    return l;
}
void list_free(LIST l) {
    free(l->list);
    l->num_elems = 0;
    free(l);
}
void list_add(LIST l, void* data) {
    if (l->num_elems == l->size) {
        l->size *= 2;
        l->list = (void**)realloc(l->list, sizeof(void*)*l->size);
    }
    l->list[l->num_elems] = data;
    l->num_elems++;
}
void list_remove(LIST l, int index) {
    if (index >= l->num_elems || index < 0)
        return;
    for (int i = index; i < l->num_elems; i++) {
        if (i == l->num_elems-1)
            l->list[i] = NULL;
        else
            l->list[i] = l->list[i+1];
    }
    l->num_elems--;
}
void list_modify(LIST l, int index, void* value) {
    if (index >= l->num_elems || index < 0)
        return;
    l->list[index] =  value;
}
int list_find(LIST l, void* value, int (*comp)(void*, void*)) {
    for (int i = 0; i < l->num_elems; i++)
        if (comp(value, l->list[i]))
            return i;
    return -1;
}
void* list_get(LIST l, int index) {
    if (index >= l->num_elems || index < 0)
        return NULL;
    return l->list[index];
}
int list_size(LIST l) {
    return l->num_elems;
}

// only if list is storing strings
#include <stdio.h>
#include <string.h>
void list_dump(LIST l) {
#ifdef DEBUG
    for (int i = 0; i < l->num_elems; i++)
        printf("| %s ", (char *)list_get(l, i));
    printf("|\n");
#endif
}

void list_dump_int(LIST l) {
#ifdef DEBUG
    for (int i = 0; i < l->num_elems; i++)
        printf("| %i ", (int)list_get(l, i));
    printf("|\n");
#endif
}


int string_finder(void* e1, void* e2) {
    return !strcmp(e1, e2);
}
int generic_finder(void* e1, void* e2) {
    return e1 == e2;
}


