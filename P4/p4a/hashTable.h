#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_
#include <stdint.h>

typedef struct list {
    char *str;
    struct list *next;
} list_t;

typedef struct hashTable {
    int size;
    list_t **table;
} hashTable_t;

hashTable_t *HT_initialization(int);
unsigned int hashIndex(hashTable_t*, char *);
list_t *lookup(hashTable_t *, char *);
int add(hashTable_t *hashtable, char *str);
void free_HT(hashTable_t *);
uint16_t fletcher16( uint8_t const *data, size_t bytes );

#endif
