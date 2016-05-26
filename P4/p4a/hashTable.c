#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
//#include <pthread.h>
//#include <stdbool.h>

#include "hashTable.h"

hashTable_t *HT_initialization(int size) {
  hashTable_t *HT;
  if (size < 1)
      return NULL;
  if ((HT = malloc(sizeof(hashTable_t))) == NULL)
      return NULL;
  if ((HT->table = malloc(sizeof(list_t*)*size))==NULL)
      return NULL;
  int i;
  for (i = 0; i < size; i++){
      HT->table[i] = NULL;
  }
  HT->size = size;
  return HT;
}

unsigned int hashIndex(hashTable_t *hashtable, char *str) {
  unsigned int index = 0;
  for (;*str!='\0';str++)
    index = *str + (index << 5) - index;
  return index % hashtable->size;
}

list_t *lookup(hashTable_t *hashtable, char *str) {
  list_t *list;
  char* str2 = strdup(str);
  int length = strlen(str);
  //unsigned int index = hashIndex(hashtable, str);
  int index = (fletcher16(str2, length)%10);
  for (list = hashtable->table[index]; list!=NULL;list=list->next) {
    if (strcmp(str, list->str)==0)
      return list;
  }
  return NULL;
}

int add(hashTable_t *hashtable, char *str) {
  list_t *new_list;
  list_t *curr_list;
  char* str2 = strdup(str);
  int length = strlen(str);
  //unsigned int index = hashIndex(hashtable, str);
  int index = (fletcher16(str2, length)%10);
  if ((new_list = malloc(sizeof(list_t))) == NULL){return 1;}
  curr_list = lookup(hashtable, str);
  if (curr_list != NULL) {return 2;}
  else{
    new_list->str = strdup(str);
    new_list->next = hashtable->table[index];
    hashtable->table[index] = new_list;
    return 0;
  }
}

void free_HT(hashTable_t *hashtable) {
  list_t *list, *tmp;
  int i;
  if(hashtable==NULL) {return;}
  for (i = 0; i<hashtable->size; i++) {
    list = hashtable->table[i];
    while(list!=NULL) {
      tmp = list;
      list = list->next;
      free(tmp->str);
      free(tmp);
    }
  }
  free(hashtable->table);
  free(hashtable);
}

uint16_t fletcher16( uint8_t const *data, size_t bytes )
{
        uint16_t sum1 = 0xff, sum2 = 0xff;
        size_t tlen;

        while (bytes) {
                tlen = bytes >= 20 ? 20 : bytes;
                bytes -= tlen;
                do {
                        sum2 += sum1 += *data++;
                } while (--tlen);
                sum1 = (sum1 & 0xff) + (sum1 >> 8);
                sum2 = (sum2 & 0xff) + (sum2 >> 8);
        }
        /* Second reduction step to reduce sums to 8 bits */
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
        return sum2 << 8 | sum1;
}

