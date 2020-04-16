#ifndef __BF_H__
#define __BF_H__

/****************************************************************************
 * bf.h: external interface definition for the BF layer
 ****************************************************************************/

/*
 * size of buffer pool
 */
#define BF_MAX_BUFS     40

/*
 * size of BF hash table
 */
#define BF_HASH_TBL_SIZE 20

/*
 * prototypes for BF-layer functions
 */
void BF_Init(void);
int BF_AllocBuf(BFreq bq, PFpage **fpage);
int BF_GetBuf(BFreq bq, PFpage **fpage);
int BF_UnpinBuf(BFreq bq);
int BF_TouchBuf(BFreq bq);
int BF_FlushBuf(int fd);
void BF_ShowBuf(void);
void BF_PrintError(const char *s);

/******************************************************************************/
/*      BF Layer - Error codes definition                                     */
/******************************************************************************/
#define BF_NERRORS              14      /* number of error codes used */

#define BFE_OK                  0
#define BFE_NOMEM               (-1)
#define BFE_NOBUF               (-2)
#define BFE_PAGEPINNED          (-3)
#define BFE_PAGEUNPINNED        (-4)
#define BFE_PAGEINBUF           (-5)
#define BFE_PAGENOTINBUF        (-6)
#define BFE_INCOMPLETEWRITE     (-7)
#define BFE_INCOMPLETEREAD      (-8)
#define BFE_MISSDIRTY           (-9)
#define BFE_INVALIDTID          (-10)
#define BFE_MSGERR              (-11)
#define BFE_HASHNOTFOUND        (-12)
#define BFE_HASHPAGEEXIST       (-13)

/*
 * error in UNIX system call or library routine
 */
#define BFE_UNIX		(-100)

/*
 * most recent BF error code
 */
extern int BFerrno;

#endif

