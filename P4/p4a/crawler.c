#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include "crawler.h"

//1.queue(fixed size) of links (parsers to downloaders)
//2.queue(unbounded size) of pages (downloaders to parsers)  
//producer&consumer problem
//3.hash set //don't visit same page twice

//Queue of links:
//parser waits when queue is full (need cond variable)
//downloader waits when queue is empty (need another cond variable)
//need 1 mutex, 2 cond variables

//Queue of pages: (unbounded)
//parser waits when queue is empty (need cond variable)
//downloader waits when never (don't need another cond variable)
//need 1 mutex, 1 con variable

//Hash Set:
//1 mutex, fletcher32 

#define HASH_TABLE_SIZE 10
#define LINK_SIZE 10000

typedef struct page{
	char* from;
	char* content;
} page_t;

//CV, locks
pthread_mutex_t link_queue_lock;
pthread_mutex_t page_queue_lock;
pthread_mutex_t num_work_lock;
pthread_mutex_t ht_lock;
pthread_cond_t link_queue_fill;
pthread_cond_t link_queue_empty;
pthread_cond_t page_queue_fill;
pthread_cond_t num_work_zero;

int num_work = 0; //number of active work
int link_queue_size; //max size of link queue
char** link_queue; //link queue
page_t** page_queue; //page queue
hashTable_t *link_ht; //hash table

int link_queue_top = -1;//test empty of link queue
int page_queue_top = -1;//test empty of page queue
int page_queue_size = 10;
char *(*fetch_fn)(char *url);
void (*edge_fn)(char *from, char *to);

void *consumer(void *ptr) {
  while (1) {
    pthread_mutex_lock(&link_queue_lock);
    while (link_queue_top < 0){
      pthread_cond_wait(&link_queue_fill, &link_queue_lock);
    }
    char* link = link_queue[link_queue_top];
    link_queue_top--;
    pthread_cond_signal(&link_queue_empty);
    pthread_mutex_unlock(&link_queue_lock);
    page_t* page = malloc(sizeof(page_t));
    page->from = strdup(link);
    char* page_content = fetch_fn(link);
    page->content= strdup(page_content);
    pthread_mutex_lock(&page_queue_lock);
    page_queue_top++;
    if (page_queue_top + 1 > page_queue_size) {
      page_queue_size += 10;
      page_queue = (page_t**)realloc(page_queue, page_queue_size);
    }
    page_queue[page_queue_top] = page;
    pthread_cond_signal(&page_queue_fill);
    pthread_mutex_unlock(&page_queue_lock);
  }
}

void *producer(void *ptr) {
  while (1) {
    //wait while empty
    pthread_mutex_lock(&page_queue_lock);
    while(page_queue_top < 0) {
      pthread_cond_wait(&page_queue_fill, &page_queue_lock);
    }
    page_t* page = page_queue[page_queue_top];
    page_queue_top--;
    pthread_mutex_unlock(&page_queue_lock);
    //parseing
    char* str = strdup(page->content);
    char* saveptr;
    char* token = strtok_r(str, " \n", &saveptr);
    char needle[6] = "link:";
    char link[LINK_SIZE];
    while(token != NULL) {
      char* tmp = strstr(token, needle);
      if (tmp == token){
        strcpy(link, tmp+5);
        edge_fn(page->from, link);
        int processed = 1;
        //check link in hash set
        pthread_mutex_lock(&ht_lock);
        if (!lookup(link_ht, link)){
          add(link_ht, link);
          processed = 0;
        }
        pthread_mutex_unlock(&ht_lock);
        if (!processed){
          pthread_mutex_lock(&link_queue_lock);
          while(link_queue_top + 1 >= link_queue_size){
            pthread_cond_wait(&link_queue_empty, &link_queue_lock);
          }
          link_queue_top ++;
          link_queue[link_queue_top] = strdup(link);
          pthread_cond_signal(&link_queue_fill);
          pthread_mutex_lock(&num_work_lock);
          num_work ++;
          pthread_mutex_unlock(&num_work_lock);
          pthread_mutex_unlock(&link_queue_lock);
        }
      }       
      token = strtok_r(NULL, " \n", &saveptr);
    }
    free(page->content);
    free(page);
    free(str);
    // after parsing one page, decrease the number of works
    pthread_mutex_lock(&num_work_lock);
    num_work--;
    if (num_work == 0){
      pthread_cond_signal(&num_work_zero);
    }
    pthread_mutex_unlock(&num_work_lock);
  }
}


int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*_fetch_fn)(char *url),
	  void (*_edge_fn)(char *from, char *to)) {
    
  link_ht = HT_initialization(HASH_TABLE_SIZE);
  add(link_ht, start_url);
  link_queue_size = queue_size;
  fetch_fn = _fetch_fn;
  edge_fn = _edge_fn;
  link_queue = (char**)malloc(queue_size*sizeof(char*));
  link_queue_top++;
  link_queue[link_queue_top] = start_url;
  num_work++;
  page_queue = (page_t**)malloc(page_queue_size);
  //create two thread pools  
  pthread_t parsers[parse_workers], downloaders[download_workers];
  int i;
  int *ptr;
  //create threads
  for (i = 0; i < parse_workers; i++) {
    ptr = (int *)malloc(sizeof(int));
    *ptr = i;
    pthread_create(&parsers[i], NULL, producer, NULL);
  }
  for (i = 0; i < download_workers; i++) {
    ptr = (int *)malloc(sizeof(int));
    *ptr = i;
    pthread_create(&downloaders[i], NULL, consumer, NULL);
  }
  //wait  
  pthread_mutex_lock(&num_work_lock);
  while(num_work > 0){
    pthread_cond_wait(&num_work_zero, &num_work_lock);
  }
  pthread_mutex_unlock(&num_work_lock);
    
  //free(link_queue);
  free_HT(link_ht);
  //cancel threads
  int m;
  for(m = 0; m < download_workers; m++){	
    pthread_cancel(downloaders[m]);
  }
  int n;
  for(n = 0; n < parse_workers; n++){
    pthread_cancel(parsers[n]);
  }
  return 0;
}
