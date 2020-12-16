/*
 * 					  CMUQ
 * 			     15-213, Fall '20
 * 				    Proxy Lab
 *				
 *			Written by Nadim Bou Alwan
 * 			   Andrew ID: nboualwa 
 *
 *
 *
 * webcache.c
 * CODE DESCRIPTION
 *
 * A fully-associative web cache used by the proxy. Caches client requests
 * and returns them if requested again, instead of having to go through
 * the client's requested server. Uses LRU eviction policy.
 * 
 * Maximum cache size: 1 MiB
 * Maximum cache object size: 100 KiB 
 *
 *
 * Possible bugs:
 * - Line "count" variable is subject to over/underflow.
 * - Race conditions in reading/writing of cache
 */

#include "csapp.h"
#include "webcache.h"


/***********************/
/*** CACHE FUNCTIONS ***/
/***********************/

/*
 * cache_init - initialize the web cache
 */
cache_t *cache_init() 
{
	// Need to do some locking/unlocking magic here
	
	cache_t *cache = (cache_t *)Malloc(sizeof(cache_t));
	cache->size = 0;
	cache->hd = NULL;

	return cache;
}

/*
 * add_object - inserts a web object into the cache
 */
void add_object(cache_t *cache, char *key, char *web_obj, size_t s)
{
	/* Add the object to the cache if its size is <=MAX_OBJECT_SIZE */
	if (s <= MAX_OBJECT_SIZE)
	{	
		line_t *line = create_line(cache, key, web_obj, s);
		insert_line(cache, line);
	}
}

/*
 * in_cache - given a request (key), determine whether its respective 
 *		web content is cached.
 *      Returns the object if found, otherwise returns NULL
 */
line_t* in_cache(cache_t *cache, char *key)
{
	line_t *ptr;

	/* Accessing cache, so decrement all counts */
	decr_counts(cache);

	for (ptr = cache->hd; ptr != NULL; ptr = ptr->next)
	{
		if (!(strcmp(ptr->key, key)))
		{	
			/* Increment the usage count of the line and return it */
			ptr->count++;
			return ptr;
		}
	}

	return NULL;
}

/*
 * cache_size - return the web cache's current size
 */
size_t cache_size(cache_t *cache) 
{
	return cache->size;
}

/*
 * cache_full - Returns whether a cache can fit another web object
 * 			Returns 1 if true, 0 otherwise
 */
int cache_full(cache_t *cache)
{
	size_t space_left = MAX_CACHE_SIZE - cache_size(cache);

	return (space_left <= MAX_OBJECT_SIZE);
}

/*
 * cache_empty - Returns whether a cache is empty
 * 			Returns 1 if true, 0 otherwise
 */
int cache_empty(cache_t *cache)
{	
	/* Cache is empty if its linked list is empty */
	return !(!(cache->hd)); // OR !(cache->size)
}

/*
 * decr_counts - Decrements the usage count of each line.
 * 			Following the LRU policy, this is called every time
 *			the cache is accessed to be able to determine the LRU line.
 *			The line with the lowest usage count gets evicted.
 */
void decr_counts(cache_t *cache)
{
	line_t *line = cache->hd;

	while (line)
	{
		line->count--;
		line = line->next;
	}
}

/***************************/
/*** END CACHE FUNCTIONS ***/
/***************************/


/**********************/
/*** LINE FUNCTIONS ***/
/**********************/

/*
 * line_size - return the size of the *Web Content* held in the line
 */
size_t line_size(line_t *line) 
{
	return line->size;
}

/*
 * line_count - return the usage count of the line. Used for LRU purposes
 */
int line_count(line_t *line)
{
	return line->count;
}

/*
 * create_line - create a line to be inserted into the cache
 */
line_t* create_line(cache_t *cache, char *key, char *web_obj, size_t s)
{
	line_t *new_line = (line_t *)(Malloc(sizeof(line_t)));

	/* Initialize line values */
	new_line->size = s;
	new_line->next = NULL;
	new_line->count = 1; // Considering current use
	new_line->key = (char *)(Malloc(strlen(key)+1));
	new_line->web_obj = (char *)(Malloc(s));

	/* Save line values */
	strcpy(new_line->key, key);
	memcpy(new_line->web_obj, web_obj, s);

	return new_line;
}

/*
 * insert_line - insert a just-created line into the cache
 */
void insert_line(cache_t *cache, line_t *line)
{	
	/* Check if the cache is full; evict if necessary */
	if(cache_full(cache))
		evict(cache);

	/* Point line's next to the current head of the list if it exists */
	if (cache->hd)
		line->next = cache->hd;
	/* Add line at the head of the list */
	cache->hd = line;
	/* Increase the cache size */
	cache->size += line_size(line);
}

/*
 * remove_line - remove a given line from the linked list
 */
void remove_line(cache_t *cache, line_t *line)
{
	line_t *ptr = cache->hd;

	/* Update the linked list to make the pointers reflect loss of line */
	while (ptr)
	{
		/* Line found: free it and make current next point to line next */
		if (ptr->next == line)
		{
			/* Update linked list */
			ptr->next = line->next;
			/* Decrement the cache size and free the line */
			cache->size -= line_size(line);
			free_line(cache, line);
			return;
		}

		ptr = ptr->next;
	}
}

/**************************/
/*** END LINE FUNCTIONS ***/
/**************************/


/**************************/
/*** EVICTION FUNCTIONS ***/
/**************************/

/*
 * evict - Evict a line from the cache based on LRU policy
 */
void evict(cache_t *cache)
{
	line_t *line = lru_line(cache);
	remove_line(cache, line);
}

/*
 * lru_line - Returns the least recently used line in the cache.
 * 		If more than one line is least used, returns the first such line.
 */
line_t* lru_line(cache_t *cache)
{
	int count_next, lru_count;
	line_t *lru, *ptr;

	/* A current pointer for comparison */
	ptr = cache->hd; 
	/* In the beginning, LRU line is assumed to be the first line */
	lru = ptr;
	/* lru_line only called when cache is full, so no need for NULL check */
	lru_count = line_count(lru);

	while (ptr)
	{
		/* Get the usage count of the next line in the list */
		count_next = line_count(ptr);
		/* If the next line has been used less recently, make it the new lru */
		if (count_next < lru_count)
		{
			lru_count = count_next;
			lru = ptr;
		}

		ptr = ptr->next;
	}	

	return lru;
}

/******************************/
/*** END EVICTION FUNCTIONS ***/
/******************************/


/**************************/
/*** CLEAN-UP FUNCTIONS ***/
/**************************/ 

/*
 * free_line - Free the given line of its contents
 */
void free_line(cache_t *cache, line_t *line)
{		
	/* Only pointers can have freedom */
	Free(line->web_obj);
	Free(line->key);
	Free(line);
}

/*
 * free_cache - Free the given cache of its contents
 */
void free_cache(cache_t *cache)
{
	line_t *line = cache->hd;

	/* Free the line & cache and return if the cache has only one line */
	if (!(line->next))
	{
		free_line(cache, line);
		Free(cache);
		return;
	}

	/* A next line for iteration */
	line_t *line_next = line->next;

	while (line)
	{
		line_next = line->next;
		free_line(cache, line);
		line = line_next;
	}

	Free(cache);
}

/******************************/
/*** END CLEAN-UP FUNCTIONS ***/
/******************************/
















