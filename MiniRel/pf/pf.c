#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "../h/pf.h"
#include "../h/bf.h"
#include "../h/minirel.h"

#include "pfinternal.h"
#include "bfinternal.h"

#define FILE_CREATE_MASK (S_IRUSR|S_IWUSR|S_IRGRP)

int PFerrno = 1;

PFftab_ele *freePFHead;
int numPFftab_ele=0;

char header[PAGE_SIZE];
PFhdr_str hdrStr;

void PF_Init(void){
    PFftab_ele *PFftab;
    PFftab = (PFftab_ele*)malloc(sizeof(PFftab_ele) * PF_FTAB_SIZE);

    //make all free and invalid
    int i;
    for(i=0; i<PF_FTAB_SIZE-1; i++){
        PFftab[i].next_ele = &PFftab[i+1];
        PFftab[i].valid = FALSE;
    }
    PFftab[PF_FTAB_SIZE-1].next_ele  = NULL;
    PFftab[PF_FTAB_SIZE-1].valid = FALSE;

    freePFHead = &PFftab[0];
}
int PF_CreateFile(const char *filename){
    int fd, unixfd;
    if ((unixfd = open(filename, O_RDWR|O_CREAT|O_EXCL, FILE_CREATE_MASK))<0){
        /*file open fail*/
        return PFE_UNIX;
    }

    memset(header, 0x00, PAGE_SIZE);
    hdrStr.numpages = 0;
    memcpy(&header, &hdrStr, sizeof(hdrStr));

    if(write(unixfd, header, PAGE_SIZE) != PAGE_SIZE){
        return PFE_UNIX;
    }

    if((close(unixfd))<0)
        return PFE_UNIX;

    return PFE_OK;
}
int PF_DestroyFile(const char *filename){
    char command[128];

    if(remove(filename)<0)
        return PFE_UNIX;
    
    return PFE_OK;
}
int PF_OpenFile(const char *filename){
    int fd, unixfd;
    PFftab_ele *newEle;
    struct stat file_stat;
    int error;
    int nbytes;
    PFhdr_str newHdr;

    if((unixfd = open(filename, O_RDWR))<0){
        /*file open fail*/
        return PFE_UNIX;
    }
    newEle = freePFHead;
    freePFHead = freePFHead->next_ele;

    if((error = fstat(unixfd, &file_stat))<0){
        //error
        return PFE_UNIX;
    }
    newEle->valid = TRUE;
    newEle->inode = file_stat.st_ino;
    strcpy(newEle->fname, filename);
    newEle->unixfd = unixfd;
    newEle->hdrchanged = FALSE;
    newEle->next_ele = NULL;

    nbytes=read(unixfd, &newHdr, sizeof(PFhdr_str));
    if(nbytes != sizeof(PFhdr_str))
        return PFE_UNIX;

    newEle->hdr = newHdr;

    numPFftab_ele++;
    
    return newEle - PFftab;
}
int PF_CloseFile(int fd){
    int err;
    if((err=BF_FlushBuf(fd)<0)){
        BF_PrintError("Cannot Flush");
        PFerrno = PFE_PAGEFREE;
        return PFerrno;
    }
    
    if(PFftab[fd].hdrchanged){
        if(write(PFftab[fd].unixfd, &(PFftab[fd].hdr), sizeof(PFhdr_str))!=sizeof(PFhdr_str))
            return PFE_UNIX;
    }

    PFftab[fd].valid = FALSE;
    PFftab[fd].next_ele = freePFHead;
    freePFHead = &(PFftab[fd]);

    return PFE_OK;
}
int PF_AllocPage(int fd, int *pagenum, char **pagebuf){
    int msg;

    //Update hdr in PF file table
    PFftab[fd].hdr.numpages = PFftab[fd].hdr.numpages + 1;
    //Update file header
    pagenum = &PFftab[fd].hdr.numpages;
    PFftab[fd].hdrchanged = TRUE;
    //Append to the end of the file 
    if(msg = write(PFftab[fd].unixfd, &(PFftab[fd].hdr), (*pagenum)*sizeof(PAGE_SIZE)) == -1){
        return msg;
    }
    //Create a BFreq
    BFreq new_page_req;
    new_page_req.fd = fd;
    new_page_req.unixfd = PFftab[fd].unixfd;
    new_page_req.pagenum = pagenum;
    new_page_req.dirty = TRUE;
    PFpage new_page_data;
    new_page_data.pagebuf[PAGE_SIZE] = &pagebuf;    //What index? PAGE_SIZE? 0?
    //Allocates buffer entry
     if (msg = BF_AllocBuf(new_page_req, &new_page_data ) == BFE_OK){
         return BFE_OK;
     }
     else{
         return msg;
     }
    
}
int PF_GetFirstPage(int fd, int *pagenum, char **pagebuf){
    int msg;
    if(msg = PF_GetNextPage(fd, -1, *pagebuf) == PFE_OK){
        return PFE_OK;

    }else if(PF_GetNextPage(fd, -1, *pagebuf) == PFE_EOF){
        return PFE_EOF;
    }
    else{
        return msg;
    }
}
int PF_GetNextPage(int fd, int *pagenum, char **pagebuf){
    int msg;
    BFreq req;
    PFpage req_fpage;
    
    //Check to see if the next page exists
    if(PFftab[fd].hdr.numpages <= (*pagenum + 1)){
        return PFE_EOF;
    }
    else if((*pagenum == -1)){
        //Return the first page
        *pagebuf = PFftab[fd].hdr.first_page;
    }
    else{

        //Buffer Request
        req.fd = fd;
        req.pagenum = *pagenum + 1;
        req.unixfd = PFftab[fd].unixfd;
        req.dirty = FALSE;

        //Set pointer *pagebuf to point to start of page data
        if(msg = BF_GetBuf(req, &req_fpage)== BFE_OK){
            *pagebuf = &req_fpage;
            *pagenum = *pagenum +1;
            return PFE_OK;
        }
        else{
            return msg;
        }
    }

}
int PF_GetThisPage(int fd, int pagenum, char **pagebuf){
    int msg;
    BFreq req;
    PFpage req_fpage;

    //Check to see if page exists
    if(PFftab[fd].hdr.numpages >= pagenum){
        //Buffer Request
        req.fd = fd;
        req.pagenum = pagenum;
        req.unixfd = PFftab[fd].unixfd;
        req.dirty = FALSE;
        
        if(BF_GetBuf(req, &req_fpage) == BFE_OK){
            *pagebuf = &req_fpage;
            return PFE_OK;
        }
        else{
            return msg;
        }
    }
    else{
        return PFE_INVALIDPAGE;
    }

}
int PF_DirtyPage(int fd, int pagenum){
    int msg;
    BFreq req;
    //Check page exists and is valid
    if((PFftab[fd].hdr.numpages >= pagenum) && (PFftab[fd].valid == TRUE)){
        req.fd = fd;
        req.pagenum = pagenum;
        req.unixfd = PFftab[fd].unixfd;
        req.dirty = TRUE;
        
        //Not sure how to access the BFPage and change the dirty flag to TRUE
        // BFpage *dirty_page = hashT[hashF(fd,pagenum)].bpage;
        // *dirty_page.dirty == TRUE;

        return PFE_INVALIDPAGE;
    }
int PF_UnpinPage(int fd, int pagenum, int dirty){
    BFreq req;
    
    //Check page exists and is valid
    if((PFftab[fd].hdr.numpages >= pagenum) && (PFftab[fd].valid == TRUE)){
        req.fd = fd;
        req.pagenum = pagenum;
        req.unixfd = PFftab[fd].unixfd;
        if(dirty == TRUE){
            req.dirty = TRUE;
        }
        else{
            req.dirty = FALSE;
        }

        //Unpins
        if(msg = BF_UnpinBuf(req) == BFE_OK){
            return PFE_OK;
        }
        else{
            return msg;
        }
    }
    
    //I dont know what error to return
    //return error;
}
void PF_PrintError(const char *s);