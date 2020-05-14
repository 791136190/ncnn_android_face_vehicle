/***********************************************************************
* 说明: 用于提供统一内存管理
		第一版本:实现非行线程安全的内存池操作
		第二版本:加上多线程和多堆管理
*
* 作者:研发部->算法组
************************************************************************/

#ifndef HU_ALG_MEM_SYS_H_
#define HU_ALG_MEM_SYS_H_

#ifdef __cplusplus  
extern "C" {
#endif 

//#define FASE_MEM (1)

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>

#define ALG_MEE_ALIGN (16)

#define MAX_MEM_SIZE (2 * GB)

#ifndef CHUNKHEADER
#define CHUNKHEADER sizeof(struct _chunk)
#endif

#ifndef CHUNKEND
#define CHUNKEND sizeof(struct _chunk *)
#endif


#define init_Memory(mm) do { \
	mm->alloc_mem = 0; \
	mm->alloc_prog_mem = 0; \
	mm->free_list = (Chunk *)mm->start; \
	mm->free_list->is_free = 1; \
	mm->free_list->alloc_mem = mp->mem_pool_size; \
	mm->free_list->prev = NULL; \
	mm->free_list->next = NULL; \
	mm->alloc_list = NULL; \
} while (0)

#define dlinklist_insert_front(head,x) do { \
	x->prev = NULL; \
	x->next = head; \
	if (head) \
		head->prev = x; \
	head = x; \
} while(0)

#define dlinklist_delete(head,x) do { \
	if (!x->prev) { \
		head = x->next; \
		if (x->next) x->next->prev = NULL; \
	} else { \
		x->prev->next = x->next; \
		if (x->next) x->next->prev = x->prev; \
	} \
} while(0)

#define KB (size_t)(1 << 10)
#define MB (size_t)(1 << 20)
#define GB (size_t)(1 << 30)

typedef struct _chunk
{
	size_t alloc_mem;
	struct _chunk *prev;
	struct _chunk *next;
	bool is_free;
}Chunk;

typedef struct _mem_pool_list
{
	int id;
	char *start;
	char *end;
	size_t alloc_mem;
	size_t alloc_prog_mem;
	Chunk *free_list, *alloc_list;
	struct _mem_pool_list *next;
}Memory;

typedef struct _mem_pool
{
	int last_id;
	size_t mem_pool_size, total_mem_pool_size;
	struct _mem_pool_list *mlist;
	bool auto_extend;
}MemoryPool;

void get_memory_list_count(MemoryPool *mp, size_t *mlist_len);

int alg_mem_init(size_t nSize, unsigned int bAutoExtend);

void* alg_malloc_mem(size_t nSize, const char* func, const int line, unsigned int bClean);
int alg_free_mem(void* pAddr, const char* func, const int line);

int MemoryPool_Clear();

int MemoryPool_Destroy();

double get_mempool_usage();

double get_mempool_prog_usage();

#define alg_malloc(_size) alg_malloc_mem(_size, __FUNCTION__, __LINE__, 0)
#define alg_calloc(_size) alg_malloc_mem(_size, __FUNCTION__, __LINE__, 1)
#define alg_free(_pAddr) alg_free_mem(_pAddr, __FUNCTION__, __LINE__)

#ifdef __cplusplus
};
#endif
#endif