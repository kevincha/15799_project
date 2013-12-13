/**
 * @brief Provides another memory allocation interface so that one can aggregate
 * objects within a single page to exploit row buffer locality in DRAM.
 **/
#include "memory_aggregator.h"
#include <stdio.h>
#include <errno.h>
#include <math.h>

// --- Global vars ---
page_state *page_pool = NULL;
int free_page_count = 0;
int page_pool_size = 20;

/**
 * @brief Initiate a few physical pages to begin with
 **/
void init_page_pool()
{
    int i = 0;
    for (; i < page_pool_size; i++)
    {
        int ret = alloc_page();
        if (ret == -1)
            exit(-1);
        // Keep trying until we find one
        else if (ret == -2)
            i--;
    }
}

/**
 * @brief Set the size of the initial page pool
 **/
void set_page_pool_size(int num_requests)
{
    page_pool_size = ceil((double)num_requests*(double)(24)/(double)PAGE_SIZE); 
    printf ("PAGE POOL SIZE - %d\n", page_pool_size);
}

/**
 * @brief Allocate an entry in a physical page
 * We start using a page sequentially starting from the head of the page pool.
 * Once a page if fully allocated it is removed from the pool.
 *
 * *** IMPORTANT ***
 * We are assuming pages will not be reclaimed as opposed to malloc. Meaning that once
 * a key value pair is, it's never deleted.
 *
 * *** CAVEAT ***
 * Due to page removal, specifying the same index does not gurantee that all those entries are
 * allocated in the same page.
 *
 * @param size size of the entry
 * @param page_i index of a page that one wants to use. -1 uses a sequential order starting from the lowest index
 **/

void *alloc_entry_random(size_t size)
{
    int page_i;
    page_i = rand()%page_pool_size;
//    page_i = 0;
    debug ("PAGE POOL SIZE - %d\n", page_pool_size);
//    page_i = 1;
    debug("Try to allocate a new entry in page %d.", page_i);
    if (page_i >= free_page_count)
    {
        debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
        exit(-1);
    }

    // Select a page based on the index
    page_state *page_sel = page_pool;
    int i = 0;
    if (page_i >= 0)
    {
        while (i != page_i && page_sel->next != NULL)
        {
            page_sel = page_sel->next;
            i++;
        }

        if (page_sel == NULL)
        {
            debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
            exit(-1);
        }
    }

    // Check the current page
    void *p_page = page_sel->p_page;
    int offset = page_sel->current_offset;
    // Run out of pages -- allocate a new page and insert in the head of the page pool (linked list)
    if ((offset + size) > PAGE_SIZE)
    {
//        page_i = rand()%page_pool_size;
//        printf ("PAGE POOL SIZE - %d\n", page_pool_size);
//        debug("Try to allocate a new entry in page %d.", page_i);
//        if (page_i >= free_page_count)
//        {
//            debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
//            exit(-1);
//        }
//    
//        // Select a page based on the index
//        page_state *page_sel = page_pool;
//        int i = 0;
//        if (page_i >= 0)
//        {
//            while (i != page_i && page_sel->next != NULL)
//            {
//                page_sel = page_sel->next;
//                i++;
//            }
//    
//            if (page_sel == NULL)
//            {
//                debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
//                exit(-1);
//            }
//        }
//        p_page = page_sel->p_page;
//        offset = page_sel->current_offset;

          
        debug("Insufficient page size!");
        // Remove the page and use page 0 instead
        remove_page(page_sel);
        // Allocate
        alloc_page();
        page_sel = page_pool;
        offset = page_sel->current_offset;
    }

    page_sel->current_offset = offset + size;
    debug("Allocated a page in %d with offset %d!", page_i, offset);
    return (void *)(((char *)p_page) + offset);
}


void *alloc_entry(size_t size, int page_i)
{
    debug("Try to allocate a new entry in page %d.", page_i);
    if (page_i >= free_page_count)
    {
        debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
        exit(-1);
    }

    // Select a page based on the index
    page_state *page_sel = page_pool;
    int i = 0;
    if (page_i >= 0)
    {
        while (i != page_i && page_sel->next != NULL)
        {
            page_sel = page_sel->next;
            i++;
        }

        if (page_sel == NULL)
        {
            debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
            exit(-1);
        }
    }

    // Check the current page
    void *p_page = page_sel->p_page;
    int offset = page_sel->current_offset;
    // Run out of pages -- allocate a new page and insert in the head of the page pool (linked list)
    if ((offset + size) > PAGE_SIZE)
    {
        debug("Insufficient page size!");
        // Remove the page and use page 0 instead
        remove_page(page_sel);
        // Allocate
        alloc_page();
        page_sel = page_pool;
        offset = page_sel->current_offset;
    }

    page_sel->current_offset = offset + size;
    debug("Allocated a page in %d with offset %d!", page_i, offset);
    return (void *)(((char *)p_page) + offset);
}


void *alloc_entry_skip(size_t size, int page_i)
{
    int local_offset = 0; 
    debug("Try to allocate a new entry in page %d.", page_i);
    if (page_i >= free_page_count)
    {
        debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
        exit(-1);
    }

    // Select a page based on the index
    page_state *page_sel = page_pool;
    int i = 0;
    if (page_i >= 0)
    {
        while (i != page_i && page_sel->next != NULL)
        {
            page_sel = page_sel->next;
            i++;
        }

        if (page_sel == NULL)
        {
            debug("Out-of-bound page index %d with only %d pages", page_i, free_page_count);
            exit(-1);
        }
    }

    // Check the current page
    void *p_page = page_sel->p_page;
    int offset = page_sel->current_offset;
    // Run out of pages -- allocate a new page and insert in the head of the page pool (linked list)
    if ((offset + size) > PAGE_SIZE)
    {
        debug("Insufficient page size!");
        // Remove the page and use page 0 instead
        remove_page(page_sel);
        // Allocate
        alloc_page();
        page_sel = page_pool;
        offset = page_sel->current_offset;
    }

    page_sel->current_offset = offset + size + local_offset;
    debug("Allocated a page in %d with offset %d!", page_i, offset);
    return (void *)(((char *)p_page) + offset);
}

void remove_page(page_state *p_page)
{
    // This is the head
    if (p_page->prev == NULL)
        page_pool = p_page->next;
    else
        (p_page->prev)->next = p_page->next;
    free(p_page);
}

/**
 * @brief Create a new 4K memory region that aligns with a physical page
 *
 * @return 0 on success
 **/
int alloc_page()
{
    debug("Allocate a new page!");
    void *p_page = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (p_page == (void *)-1)
    {
        printf("ERROR: Failed to allocate a new page.\n");
        perror("");
        return -1;
    }
    if (((long unsigned int)p_page & (PAGE_SIZE - 1)) != 0)
    {
        printf("ERROR: Not PAGE_SIZE (4KB) aligned.\n");
        munmap(p_page, PAGE_SIZE);
        return -2;
    }

    debug("Page address = %p", p_page);

    // Insert at head
    page_state *new_page = (page_state *)malloc(sizeof(page_state));
    new_page->p_page = p_page;
    new_page->current_offset = 0;
    new_page->next = page_pool;
    if (page_pool != NULL)
        page_pool->prev = new_page;
    new_page->prev = page_pool;
    page_pool = new_page;
    free_page_count++;

    return 0;
}

