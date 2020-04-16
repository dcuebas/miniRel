#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "bfinternal.h"

int hashF(int fd, int pageNum)
{
    return (fd + pageNum) % BF_HASH_TBL_SIZE;
}
bool_t isPagePresent(BFreq bq, BFpage **BFptr)
{
    /*checks of page is present in the buffer pool*/
    int fd = bq.fd;
    int pageNum = bq.pagenum;
    BFhash_entry *hashPtr = hashT[hashF(fd, pageNum)];
    while (hashPtr != NULL){
        if (hashPtr->fd != fd || hashPtr->pageNum != pageNum)
            hashPtr = hashPtr->nextentry;
        else
        { /*there is one*/
            *BFptr = hashPtr->bpage;
            return TRUE;
        }
    }
    return FALSE;
}
void writePage(BFpage *BFptr){
    lseek(BFptr->unixfd, (BFptr->pageNum + 1)* PAGE_SIZE,SEEK_SET);
    write(BFptr->unixfd, BFptr->fpage.pagebuf, PAGE_SIZE);
}
void readPage(BFpage *BFptr){
    lseek(BFptr->unixfd, (BFptr->pageNum+1)*PAGE_SIZE, SEEK_SET);
    read(BFptr->unixfd, BFptr->fpage.pagebuf, PAGE_SIZE);
}
void freeHashPtr(BFhash_entry *hashPtr, int hashInd){
    if (hashPtr == hashT[hashInd]){
        hashT[hashInd] = hashPtr->nextentry;
        if(hashT[hashInd] != NULL)
            hashT[hashInd]->preventry = NULL;
    }
    if(hashPtr->preventry != NULL)
        hashPtr->preventry->nextentry = hashPtr->nextentry;
    if(hashPtr->nextentry != NULL)
        hashPtr->nextentry->preventry = hashPtr->preventry;
    
    /*freeing hash entry*/
    hashPtr->nextentry = freeHashHead;
    hashPtr->preventry = NULL;
    freeHashHead = hashPtr;
}
void freeBFPtr(BFpage *BFptr){
    /*free from LRU list*/
    if(BFptr == LRUhead)
        LRUhead = BFptr->nextpage;
    BFptr->prevpage->nextpage = BFptr->nextpage;
    BFptr->nextpage->prevpage = BFptr->prevpage;

    numLRU--;
    if(numLRU == 0)
        LRUhead = NULL;

    /*into free list*/
    BFptr->nextpage = FreeListHead;
    FreeListHead = BFptr;
}
int freeBufferEntry(BFpage *BFptr){
    int fd, pageNum, hashInd;
    BFhash_entry *hashPtr;

    /*if dirty*/
    if(BFptr->dirty)
        writePage(BFptr);
    
    /*buffer is assumed to be unpinned*/
    fd = BFptr->fd;
    pageNum = BFptr->pageNum;

    /*free from hash table*/
    hashInd = hashF(BFptr->fd, BFptr->pageNum);
    hashPtr = hashT[hashInd];
    while (hashPtr != NULL) {
        if (hashPtr->fd != fd || hashPtr->pageNum != pageNum)
            hashPtr = hashPtr->nextentry;
        else { /*there is one*/
            freeHashPtr(hashPtr, hashInd);
            break;
        }
    }
    if (hashPtr == NULL) { /*not in hash*/
        BFerrno = BFE_HASHNOTFOUND;
        return BFE_HASHNOTFOUND;
    }
    freeBFPtr(BFptr);

    return BFE_OK;
}
int freeBufferEntryFromHash(BFhash_entry *hashPtr, int currIndex){
    BFpage *BFptr;

    freeHashPtr(hashPtr, currIndex);

    BFptr = hashPtr->bpage;
    /*if dirty*/
    if(BFptr->dirty)
        writePage(BFptr);
    freeBFPtr(BFptr);

    return BFE_OK;
}

int freePage() {
    if (FreeListHead != NULL) /* there is already free entry*/
        return BFE_OK;
    else {   
        /*free list is empty*/
        BFpage *temp = LRUhead;
        int numRead = 0;
        while (numRead < BF_MAX_BUFS)
        {
            if (temp->count <= 0)
                return freeBufferEntry(temp);
            numRead++;
        }
        /*all the pages are pinned*/
        BFerrno = BFE_NOBUF;
        return BFE_NOBUF;
    }
}
void updateLRU(BFpage *BFptr)
{
    if (BFptr == LRUhead->prevpage) /* already head*/
        return;

    /*bring the page to the front of LRU*/
    BFptr->prevpage->nextpage = BFptr->nextpage;
    BFptr->nextpage->prevpage = BFptr->prevpage;
    BFptr->prevpage = LRUhead->prevpage;
    BFptr->nextpage = LRUhead;
    LRUhead->prevpage->nextpage = BFptr;
    LRUhead->prevpage = BFptr;
}
void copyBuff(BFreq bq, BFpage *BFptr){
    /*copy bq to BFptr and brint the front of LRU list*/
    /*BFptr->fpage = **fpage;*/
    BFptr->dirty = FALSE;
    BFptr->count = 1;
    BFptr->pageNum = bq.pagenum;
    BFptr->fd = bq.fd;
    BFptr->unixfd = bq.unixfd;
    BFptr->prevpage = BFptr;
    BFptr->nextpage = BFptr;

    /* bring to the front*/
    if(LRUhead==NULL)
        LRUhead = BFptr;

    BFptr->prevpage = LRUhead->prevpage;
    BFptr->nextpage = LRUhead;
    LRUhead->prevpage->nextpage = BFptr;
    LRUhead->prevpage = BFptr;
}
void addToHash(BFpage *BFptr){
    /*assumes there is no BFptr*/
    int hashInd = hashF(BFptr->fd, BFptr->pageNum);
    BFhash_entry *hashPtr = hashT[hashInd];
    BFhash_entry *newEntry;
    
    newEntry = freeHashHead;
    freeHashHead = freeHashHead->nextentry;
    newEntry->bpage = BFptr;
    newEntry->fd = BFptr->fd;
    newEntry->pageNum = BFptr->pageNum;
    newEntry->preventry = NULL;
    newEntry->nextentry = NULL;

    if(hashPtr!=NULL){
        newEntry->nextentry = hashPtr;
        hashPtr->preventry = newEntry;
    }
    hashT[hashInd] = newEntry;
}

void printBuff(BFpage *BFptr){
    printf("----------------------\n");
    printf("dirty: %d\n", BFptr->dirty);
    printf("cout: %d\n", BFptr->count);
    printf("pagenum: %d\n", BFptr->pageNum);
    printf("fd: %d\n", BFptr->fd);
    printf("unixfd: %d\n", BFptr->unixfd);
    printf("---------------------------\n");
}
