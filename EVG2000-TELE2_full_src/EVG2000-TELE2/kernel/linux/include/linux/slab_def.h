#ifndef _LINUX_SLAB_DEF_H
#define	_LINUX_SLAB_DEF_H

/*
 * Definitions unique to the original Linux SLAB allocator.
 *
 * What we provide here is a way to optimize the frequent kmalloc
 * calls in the kernel by selecting the appropriate general cache
 * if kmalloc was called with a size that can be established at
 * compile time.
 */

#include <linux/init.h>
#include <asm/page.h>		/* kmalloc_sizes.h needs PAGE_SIZE */
#include <asm/cache.h>		/* kmalloc_sizes.h needs L1_CACHE_BYTES */
#include <linux/compiler.h>

/* Size description struct for general caches. */
struct cache_sizes {
	size_t		 	cs_size;
	struct kmem_cache	*cs_cachep;
#ifdef CONFIG_ZONE_DMA
	struct kmem_cache	*cs_dmacachep;
#endif
};
extern struct cache_sizes malloc_sizes[];

#ifdef CONFIG_KMALLOC_ACCOUNTING
void __kmalloc_account(const void *, const void *, int, int);
static void inline kmalloc_account(const void * addr, int size, int req)
{
        __kmalloc_account(__builtin_return_address(0), addr, size, req);
}
static void inline kfree_account(const void * addr, int size)
{
        __kmalloc_account(__builtin_return_address(0), addr, size, -1);
}
#else
#define kmalloc_account(a,b,c)
#define kfree_account(a,b)
#endif

#ifdef CONFIG_KLOB
extern void klob_init(void);
extern void *kmalloc(size_t size, int flags);
extern void *kzalloc(size_t size, int flags);
#else

static inline void *kmalloc(size_t size, gfp_t flags)
{
#ifndef CONFIG_KMALLOC_ACCOUNTING
	if (__builtin_constant_p(size)) {
		int i = 0;
#define CACHE(x) \
		if (size <= x) \
			goto found; \
		else \
			i++;
#include "kmalloc_sizes.h"
#undef CACHE
		{
			extern void __you_cannot_kmalloc_that_much(void);
			__you_cannot_kmalloc_that_much();
		}
found:
#ifdef CONFIG_ZONE_DMA
		if (flags & GFP_DMA) {
			return kmem_cache_alloc(malloc_sizes[i].cs_dmacachep,
						flags);
		}
#endif
		return kmem_cache_alloc(malloc_sizes[i].cs_cachep, flags);
	}
#endif
	return __kmalloc(size, flags);
}

static inline void *kzalloc(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size)) {
		int i = 0;
#define CACHE(x) \
		if (size <= x) \
			goto found; \
		else \
			i++;
#include "kmalloc_sizes.h"
#undef CACHE
		{
			extern void __you_cannot_kzalloc_that_much(void);
			__you_cannot_kzalloc_that_much();
		}
found:
#ifdef CONFIG_ZONE_DMA
		if (flags & GFP_DMA)
			return kmem_cache_zalloc(malloc_sizes[i].cs_dmacachep,
						flags);
#endif
		return kmem_cache_zalloc(malloc_sizes[i].cs_cachep, flags);
	}
	return __kzalloc(size, flags);
}
#endif //end of KLOB

#ifdef CONFIG_NUMA
extern void *__kmalloc_node(size_t size, gfp_t flags, int node);

static inline void *kmalloc_node(size_t size, gfp_t flags, int node)
{
	if (__builtin_constant_p(size)) {
		int i = 0;
#define CACHE(x) \
		if (size <= x) \
			goto found; \
		else \
			i++;
#include "kmalloc_sizes.h"
#undef CACHE
		{
			extern void __you_cannot_kmalloc_that_much(void);
			__you_cannot_kmalloc_that_much();
		}
found:
#ifdef CONFIG_ZONE_DMA
		if (flags & GFP_DMA)
			return kmem_cache_alloc_node(malloc_sizes[i].cs_dmacachep,
						flags, node);
#endif
		return kmem_cache_alloc_node(malloc_sizes[i].cs_cachep,
						flags, node);
	}
	return __kmalloc_node(size, flags, node);
}

#endif	/* CONFIG_NUMA */

#endif	/* _LINUX_SLAB_DEF_H */
