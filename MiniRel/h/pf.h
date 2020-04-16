#ifndef __PF_H__
#define __PF_H__

/****************************************************************************
 * pf.h: external interface definition for the PF layer
 ****************************************************************************/

 /*
 * size of open file table
 */
#define PF_FTAB_SIZE	MAXOPENFILES

#ifdef PF_FREEPAGES_MAINTAINED
/*
 * PF page size
 */
#define PF_PAGE_SIZE	(PAGE_SIZE-sizeof(int))
#endif


/*
 * prototypes for PF-layer functions
 */
void PF_Init		(void);
int  PF_CreateFile	(const char *filename);
int  PF_DestroyFile	(const char *filename);
int  PF_OpenFile	(const char *filename);
int  PF_CloseFile	(int fd);
int  PF_AllocPage	(int fd, int *pagenum, char **pagebuf);
int  PF_GetFirstPage	(int fd, int *pagenum, char **pagebuf);
int  PF_GetNextPage	(int fd, int *pagenum, char **pagebuf);
int  PF_GetThisPage	(int fd, int pagenum, char **pagebuf);
int  PF_DirtyPage	(int fd, int pagenum);
int  PF_UnpinPage	(int fd, int pagenum, int dirty);
void PF_PrintError	(const char *s);

/******************************************************************************/
/*      PF Layer - Error codes definition                                     */
/******************************************************************************/
#define PF_NERRORS              12      /* number of error codes used */

#define PFE_OK			0
#define PFE_INVALIDPAGE		(-1)
#define	PFE_FTABFULL		(-2)
#define PFE_FD			(-3)
#define PFE_EOF			(-4)
#define PFE_FILEOPEN		(-5)
#define PFE_FILENOTOPEN		(-6)
#define PFE_HDRREAD		(-7)
#define PFE_HDRWRITE		(-8)
#define PFE_PAGEFREE		(-9)
#define PFE_NOUSERS		(-10)
#define PFE_MSGERR              (-11)

/*
 * error in UNIX system call or library routine
 */
#define PFE_UNIX		(-100)

/*
 * most recent PF error code
 */
extern int PFerrno;

#endif
