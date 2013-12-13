#include <sys/mman.h>
#include <stdlib.h>

#ifndef __MEMORY_AGGREGATOR_H_
#define __MEMORY_AGGREGATOR_H_

#define PAGE_SIZE (1 << 12)
//#define INIT_POOL_SIZE 10 // # of pages
#define INIT_POOL_SIZE 512 // # of pages
#define ENTRY_SIZE 24 //# size of a single entry
#define NDEBUG

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

// Booking keeping for each allocated page
typedef struct page_state {
  void *p_page;
  int current_offset;
  struct page_state *prev;
  struct page_state *next;
} page_state;

// Function Prototypes
void init_page_pool();
void set_page_pool_size(int num_requests);
void *alloc_entry_random(size_t size);
void *alloc_entry(size_t size, int page_i);
void *alloc_entry_skip(size_t size, int page_i);
void remove_page(page_state *p_page);
int alloc_page();

#endif // __MEMORY_AGGREGATOR_H_
