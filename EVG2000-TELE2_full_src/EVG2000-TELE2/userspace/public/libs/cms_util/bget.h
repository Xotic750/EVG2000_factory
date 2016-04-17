/*

    Interface definitions for bget.c, the memory management package.

*/

#define  _(x)  x		      /* If compiler knows prototypes */


typedef long bufsize;
#if defined(__cplusplus)
extern "C" {
#endif
void  bcm_cleanup_bpool    _((void));
void  bcm_secondary_bpool  _((void *buffer));
void	bpool	    _((void *buffer, bufsize len));
void   *bget	    _((bufsize size));
void   *bgetz	    _((bufsize size));
void   *bgetr	    _((void *buffer, bufsize newsize));
void	brel	    _((void *buf));
void	bectl	    _((int (*compact)(bufsize sizereq, int sequence),
		       void *(*acquire)(bufsize size),
		       void (*release)(void *buf), bufsize pool_incr));
void	bstats	    _((bufsize *curalloc, bufsize *totfree, bufsize *maxfree,
		       long *nget, long *nrel));
void	bstatse     _((bufsize *pool_incr, long *npool, long *npget,
		       long *nprel, long *ndget, long *ndrel));
void	bufdump     _((void *buf));
void	bpoold	    _((void *pool, int dumpalloc, int dumpfree));
int	bpoolv	    _((void *pool));
#if defined(__cplusplus)
}
#endif
