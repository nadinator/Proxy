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
 * webcache.h
 * CODE DESCRIPTION
 *
 * Header for webcache.c
 */


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Line structure */
typedef struct Line {
	size_t size;	// Size of the content (web_obj)
	int count; 		// Usage count, for LRU 
	char *key;      // Client request, used for identification
	char *web_obj;  // Contents of the web object
	struct Line *next;
} line_t;

/* Web Cache structure */
typedef struct Cache {
	size_t size; 	  // Overall size of the cache
	struct Line *hd;  // A pointer to the header line in the cache
} cache_t;

/* Cache functions */
cache_t* cache_init();
int cache_full(cache_t *cache);
int cache_empty(cache_t *cache);
void decr_counts(cache_t *cache);
size_t cache_size(cache_t *cache);
line_t* in_cache(cache_t *cache, char *key);
void add_object(cache_t *cache, char *key, char *web_obj, size_t s);
/* Line functions */
size_t line_size(line_t *line);
void insert_line(cache_t *cache, line_t *line);
void remove_line(cache_t *cache, line_t *line);
line_t* create_line(cache_t *cache, char *key, char *web_obj, size_t s);
/* Eviction functions */
void evict(cache_t *cache);
line_t* lru_line(cache_t *cache);
/* Clean-up functions */
void free_cache(cache_t *cache);
void free_line(cache_t *cache, line_t *line);