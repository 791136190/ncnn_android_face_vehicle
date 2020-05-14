
#include "huAlgMemSys.h"

#ifdef __cplusplus  
extern "C" {
#endif 

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void get_memory_list_count(MemoryPool *mp, size_t *mlist_len)
{
#ifndef FASE_MEM
	return;
#else
	size_t mlist_l = 0;
	Memory *mm = mp->mlist;
	while (mm)
	{
		mlist_l++;
		mm = mm->next;
	}

	*mlist_len = mlist_l;
#endif
}

void get_every_memory_info(Memory *mm, size_t *free_list_len, size_t *alloc_list_len)
{
#ifndef FASE_MEM
	return;
#else
	size_t free_l = 0, alloc_l = 0;
	Chunk *p = mm->free_list;
	while (p)
	{
		free_l++;
		p = p->next;
	}

	p = mm->alloc_list;
	while (p)
	{
		alloc_l++;
		p = p->next;
	}

	*free_list_len = free_l;
	*alloc_list_len = alloc_l;
#endif
}

#ifdef FASE_MEM
static Memory *extend_memory_list(MemoryPool *mp)
{
	fprintf(stderr, "into extend mem\n");
	Memory *mm = (Memory *)malloc(sizeof(Memory));
	if (NULL == mm)
	{
		fprintf(stderr, "extend mem malloc Memory err\n");
		return NULL;
	}
	memset(mm, 0, sizeof(Memory));

	mm->start = (char *)malloc(mp->mem_pool_size * sizeof(char));
	if (NULL == mm->start)
	{
		fprintf(stderr, "extend mem malloc start err\n");
		return NULL;
	}
	memset(mm->start, 0, sizeof(char) * mp->mem_pool_size);
	mm->end = mm->start + mp->mem_pool_size * sizeof(char);

	init_Memory(mm);
	mm->id = mp->last_id++;
	mm->next = mp->mlist;
	mp->mlist = mm;
	return mm;
}
static Memory *find_memory_list(MemoryPool *mp, void *p)
{
	Memory *tmp = mp->mlist;
	while (tmp)
	{
		if (tmp->start <= (char *)p && tmp->end > (char *)p)
			break;
		tmp = tmp->next;
	}

	return tmp;
}

static bool merge_free_chunk(MemoryPool *mp, Memory *mm, Chunk *c)
{
	if (!mp || !mm || !c)
		return 0;

	Chunk *p0 = *(Chunk **)((char *)c - CHUNKEND), *p1 = c;
	while ((char *)p0 > mm->start && (char *)p0 < mm->end && p0->is_free)
	{
		p1 = p0;
		p0 = *(Chunk **)((char *)p0 - CHUNKEND);
	}

	p0 = (Chunk *)((char *)p1 + p1->alloc_mem);
	while ((char *)p0 < mm->end && p0->is_free)
	{
		dlinklist_delete(mm->free_list, p0);

		p1->alloc_mem += p0->alloc_mem;
		p0 = (Chunk *)((char *)p0 + p0->alloc_mem);
	}

	*(Chunk **)((char *)p1 + p1->alloc_mem - CHUNKEND) = p1;

	return 1;
}

static MemoryPool* g_mem_pool = NULL;
#endif

int alg_mem_init(size_t nSize, unsigned int bAutoExtend)
{
#ifndef FASE_MEM
	return 0;
#else
	if (NULL != g_mem_pool)
	{
		//fprintf(stderr, "no need malloc init mem pool\n");
		return 0;
	}
	if (nSize > MAX_MEM_SIZE)
	{
		fprintf(stderr, "[MemoryPool_Init] MemPool Init ERROR! Mempoolsize is too big! \n");
		return -1;
	}

	MemoryPool *mp = (MemoryPool *)malloc(sizeof(MemoryPool));
	if (NULL == mp)
	{
		fprintf(stderr, "[MemoryPool_Init] alloc memorypool struct error!\n");
		return -2;
	}
	memset(mp, 0, sizeof(MemoryPool));

	mp->total_mem_pool_size = mp->mem_pool_size = nSize;
	mp->auto_extend = bAutoExtend > 0 ? true: false;
	mp->last_id = 0;

	mp->mlist = (Memory*)malloc(sizeof(Memory));
	if (NULL == mp->mlist)
	{
		fprintf(stderr, "[MemoryPool_Init] No memory! \n");
		return -3;
	}

	char *s = (char *)malloc(sizeof(char) * mp->mem_pool_size); // memory pool
	if (NULL == s)
	{
		fprintf(stderr, "[MemoryPool_Init] No memory! \n");
		return -4;
	}
	memset(s, 0, sizeof(char) * mp->mem_pool_size);

	mp->mlist->start = s;
	mp->mlist->end = s + mp->mem_pool_size * sizeof(char);

	init_Memory(mp->mlist);
	mp->mlist->next = NULL;
	mp->mlist->id = mp->last_id++;
	
	g_mem_pool = mp;
	MemoryPool_Clear();

	return 0;
#endif
}

void* alg_malloc_mem(size_t nSize, const char* func, const int line, unsigned int bClean)
{
#ifndef FASE_MEM
	//offset地址偏移量(byte)
	//alignd_byte对齐字节的预分配空间
	//sizeof(void *)保存真实指针的预分配空间
	const size_t offset = sizeof(void *) + ALG_MEE_ALIGN - 1;

	//预分配更大的内存块
	//q指向这块内存的首地址
	void* q = malloc(nSize + offset);
	if (!q)
	{
		return NULL;
	}
	//printf("q = 0x%p\n", q);

	//对齐后的内存块
	//不管怎样q指针都向后偏移，再& ~(alignd_byte - 1)地址对齐
	//如果是任意字节对齐的话这个偏移计算方法要换
	void* p = (void *)(((size_t)(q)+offset) & ~(ALG_MEE_ALIGN - 1));
	//为了配合free函数，保存q指针到p-1的位置
	//p[-1] = q;直接这样做不行void *大小未知无法寻址
	*(((void **)p) - 1) = q;

	if (bClean && NULL != p)
	{
		memset(p, 0, nSize);
	}
	//返回对齐后的指针
	return p;
#else
	if (NULL == g_mem_pool)
	{
		alg_mem_init(8*1024*1024, 1);
		if (NULL == g_mem_pool)
		{
			fprintf(stderr, "sys malloc mem err\n");
			return NULL;
		}
	}

	size_t total_size = nSize + CHUNKHEADER + CHUNKEND + ALG_MEE_ALIGN;//自带对齐
	Memory *mm = NULL, *mm1 = NULL;
	Chunk *_free = NULL, *_not_free = NULL;
	MemoryPool *mp = g_mem_pool;
	
FIND_FREE_CHUNK:
	mm = mp->mlist;

	while (mm)
	{
		if (mp->mem_pool_size < mm->alloc_mem + total_size)
		{
			if (0 == mm->alloc_mem && mp->mem_pool_size < 100 * MB)
			{
				fprintf(stderr, "your mem pool size is too small[%zd VS %zd]\n", mp->mem_pool_size, total_size);
				mp->mem_pool_size = total_size * 2;
			}
			mm = mm->next;
			continue;
		}

		_free = mm->free_list;
		_not_free = NULL;

		while (_free)
		{
			if (_free->alloc_mem >= total_size)
			{
				// 如果free块分割后剩余内存还可以做下一次分配, 则进行分割
				if (_free->alloc_mem  > total_size + CHUNKHEADER + CHUNKEND)
				{
					// 从free块头开始分割出alloc块
					// nf指向分割后的free块
					_not_free = _free;
					_free = (Chunk *)((char *)_not_free + total_size);
					*_free = *_not_free;
					_free->alloc_mem -= total_size;
					*(Chunk **)((char *)_free + _free->alloc_mem - CHUNKEND) = _free;
					
					// update free_list
					if (!_free->prev)
					{
						mm->free_list = _free;
						if (_free->next) _free->next->prev = _free;
					}
					else
					{
						_free->prev->next = _free;
						if (_free->next) _free->next->prev = _free;
					}

					_not_free->is_free = 0;
					_not_free->alloc_mem = total_size;

					*(Chunk **)((char *)_not_free + total_size - CHUNKEND) = _not_free;
				}
				// 不够 则整块分配为alloc
				else
				{
					_not_free = _free;
					dlinklist_delete(mm->free_list, _not_free);
					_not_free->is_free = 0;
				}

				dlinklist_insert_front(mm->alloc_list, _not_free);

				mm->alloc_mem += _not_free->alloc_mem;
				mm->alloc_prog_mem += (_not_free->alloc_mem - CHUNKHEADER - CHUNKEND);
				//return (void *)((unsigned char *)_not_free + CHUNKHEADER);

				unsigned char* base_ptr = (unsigned char *)_not_free + CHUNKHEADER;//模块内部释放地址
				unsigned char* mem_ptr = (unsigned char *)(((size_t)base_ptr + ALG_MEE_ALIGN - 1) & ~(ALG_MEE_ALIGN - 1));//对齐
				if (mem_ptr == base_ptr)
				{
					mem_ptr = (unsigned char*)((size_t)base_ptr + ALG_MEE_ALIGN);
				}
				//printf("get size=%u,al=%u mem_ptr:%p,base_ptr:%p\n", size, align_bytes, mem_ptr, base_ptr);
				*((size_t*)mem_ptr - 1) = (size_t)base_ptr;
				if (bClean)
				{
					memset(mem_ptr, 0x0, nSize);
				}
				return mem_ptr;
			}
			_free = _free->next;
		}

		mm = mm->next;
	}

	if (mp->auto_extend)
	{
		mm1 = extend_memory_list(mp);
		if (!mm1)
			return NULL;
		mp->total_mem_pool_size += mp->mem_pool_size;
		if (mp->total_mem_pool_size > mp->mem_pool_size * 15)
		{
			return NULL;
		}
		goto FIND_FREE_CHUNK;
	}
	fprintf(stderr, "Not enough memory, your mem used:%f\n", get_mempool_prog_usage());
	fprintf(stderr, "need:%zu, func:%s, line:%d\n", nSize, func, line);
	return NULL;
#endif
}

int alg_free_mem(void* pAddr, const char* func, const int line)
{
#ifndef FASE_MEM
	void* base_addr = ((void **)pAddr)[-1];
	free(base_addr);
	return 0;
#else
	if (NULL == g_mem_pool || NULL == pAddr)
	{
		fprintf(stderr, "man loop free mem, get null ptr g_mem_pool:%p, pAddr:%p\n", g_mem_pool, pAddr);
		fprintf(stderr, "func:%s, line:%d\n", func, line);
		return -1;
	}
	MemoryPool *mp = g_mem_pool;

	unsigned char *base_addr = (unsigned char *)(*((size_t *)pAddr - 1));//从对齐位找到真实值
	//void *base_addr = pAddr;//从对齐位找到真实值
	Memory *mm = mp->mlist;
	if (mp->auto_extend)
		mm = find_memory_list(mp, base_addr);

	Chunk *ck = (Chunk *)((unsigned char *)base_addr - CHUNKHEADER);
	dlinklist_delete(mm->alloc_list, ck);
	dlinklist_insert_front(mm->free_list, ck);
	ck->is_free = 1;

	mm->alloc_mem -= ck->alloc_mem;
	mm->alloc_prog_mem -= (ck->alloc_mem - CHUNKHEADER - CHUNKEND);

	merge_free_chunk(mp, mm, ck);
	pAddr = NULL;

	return 0;
#endif
}

int MemoryPool_Clear()
{
#ifndef FASE_MEM
	return 0;
#else
	if (NULL == g_mem_pool)
	{
		return -1;
	}

	MemoryPool *mp = g_mem_pool;

	Memory *mm = mp->mlist;
	while (mm)
	{
		init_Memory(mm);
		mm = mm->next;
	}

	return 0;
#endif
}

int MemoryPool_Destroy()
{
#ifndef FASE_MEM
	return 0;
#else
	if (NULL == g_mem_pool)
	{
		fprintf(stderr, "no need del mem pool");
		return -1;
	}
	MemoryPool *mp = g_mem_pool;

	Memory *mm = mp->mlist, *mm1 = NULL;
	while (mm)
	{
		free(mm->start);
		mm1 = mm;
		mm = mm->next;
		free(mm1);
	}
	free(mp);

    g_mem_pool = NULL;

	return 0;
#endif
}

double get_mempool_usage()
{
#ifndef FASE_MEM
	return 0.0;
#else
	if (NULL == g_mem_pool)
	{
		return -1;
	}
	MemoryPool *mp = g_mem_pool;

	size_t total_alloc = 0;
	Memory *mm = mp->mlist;
	while (mm)
	{
		total_alloc += mm->alloc_mem;
		mm = mm->next;
	}
	return (double)total_alloc / mp->total_mem_pool_size;
#endif
}

double get_mempool_prog_usage()
{
#ifndef FASE_MEM
	return 0.0;
#else
	if (NULL == g_mem_pool)
	{
		return -1;
	}
	MemoryPool *mp = g_mem_pool;

	size_t total_alloc_prog = 0;
	Memory *mm = mp->mlist;
	while (mm)
	{
		total_alloc_prog += mm->alloc_prog_mem;
		mm = mm->next;
	}
	return (double)total_alloc_prog / mp->total_mem_pool_size;
#endif
}

#ifdef __cplusplus
};
#endif