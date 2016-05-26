#ifndef __CRAWLER_H
#define __CRAWLER_H
#include "hashTable.h"

int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*fetch_fn)(char *url),
	  void (*edge_fn)(char *from, char *to));
void *consumer(void *ptr);
void *producer(void *ptr);

#endif
