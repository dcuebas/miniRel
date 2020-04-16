#ifndef PFINTERNAL_H
#define PFINTERNAL_H

#include <unistd.h>
#include <sys/types.h> /*ino_t*/

#include "../h/minirel.h"
#include "../h/pf.h"

typedef struct PFhdr_str {
    int    numpages;      /* number of pages in the file */
    PFpage *first_page;
    PFpage *free_page;
} PFhdr_str;

typedef struct PFftab_ele {
    bool_t    valid;       /* set to TRUE when a file is open. */
    ino_t     inode;       /* inode number of the file         */
    char      *fname;      /* file name                        */
    int       unixfd;      /* Unix file descriptor             */
    PFhdr_str hdr;         /* file header                      */
    short     hdrchanged;  /* TRUE if file header has changed  */

    PFftab_ele *next_ele;
} PFftab_ele;


extern PFftab_ele *PFftab;
extern PFftab_ele *freePFHead;
extern int numPFftab_ele;

#endif