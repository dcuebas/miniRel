/*to use NULL*/
#include <stddef.h> 
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "bfinternal.h"
#include "../h/bf.h"
#include "../h/minirel.h"

int BFerrno = 1;

BFpage *LRUhead = NULL;
BFhash_entry *hashT[BF_HASH_TBL_SIZE] = {NULL};
BFpage BFentry[BF_MAX_BUFS];
BFhash_entry *freeHashHead = NULL;
BFhash_entry freeHash[BF_MAX_BUFS];
BFpage *FreeListHead = NULL;
int numLRU = 0;

void BF_Init(void){
    /*create buffer entries and add them to the free list
    initialize hash table*/

    int i;
    for (i=0; i<BF_MAX_BUFS-1; i++)
        BFentry[i].nextpage=&BFentry[i+1];
    BFentry[BF_MAX_BUFS-1].nextpage = NULL;

    FreeListHead = &BFentry[0]; /*add all the entries to free list*/

    /*freeHashList init*/
    for (i=0; i<BF_MAX_BUFS-1; i++)
        freeHash[i].nextentry = &freeHash[i+1];
    freeHash[BF_MAX_BUFS-1].nextentry = NULL;
    freeHashHead = &freeHash[0];
}

int BF_AllocBuf(BFreq bq, PFpage **fpage){
    BFpage *BFptr = NULL;
    int msg;
    if(!isPagePresent(bq, &BFptr)){
        /*there is no page*/
        
        if((msg=freePage()) != BFE_OK)
            return msg;

        /*there is at least one free buffer*/
        BFptr = FreeListHead;
        FreeListHead = FreeListHead->nextpage;
        copyBuff(bq,BFptr);
        numLRU++;

        /*add to hash table*/
        addToHash(BFptr);

        *fpage = &(BFptr->fpage);
        return BFE_OK;
    } else {
        /*there is page: error*/
        BFerrno = BFE_PAGEINBUF;
        return BFE_PAGEINBUF;
    }
}
int BF_GetBuf(BFreq bq, PFpage **fpage){
    BFpage *BFptr = NULL;
    if(!isPagePresent(bq, &BFptr)){
        /*there is no page*/
        int msg;
        if((msg =freePage()) != BFE_OK)
            return msg;
        
        /*there is at least one free buffer*/
        BFptr = FreeListHead;
        FreeListHead = FreeListHead->nextpage;

        copyBuff(bq, BFptr);

        numLRU++;
        readPage(BFptr);

        addToHash(BFptr);

        *fpage = &(BFptr->fpage);

        return BFE_OK;
    } else {
        /*found page*/
        BFptr->count++;
        *fpage = &(BFptr->fpage);

        updateLRU(BFptr);
        return BFE_OK;
    }    
}
int BF_UnpinBuf(BFreq bq){
    BFpage *BFptr = NULL;
    if(!isPagePresent(bq, &BFptr)){
        /*did not find page*/
        BFerrno = BFE_PAGENOTINBUF;
        return BFE_PAGENOTINBUF;
    } else {
        /*found page*/
        if(BFptr->count<=0) {/* already unpinned: error*/
            BFerrno = BFE_PAGEUNPINNED;
            return BFE_PAGEUNPINNED;
        } else{
            BFptr->count--;
            return BFE_OK;
        }
    }
}
int BF_TouchBuf(BFreq bq){
    BFpage *BFptr = NULL;
    if (!isPagePresent(bq, &BFptr)){
        /*did not find page*/
        BFerrno = BFE_PAGENOTINBUF;
        return BFE_PAGENOTINBUF;
    }
    else {
        /*found page*/
        if (BFptr->count <= 0) {/* already unpinned: error*/
            BFerrno = BFE_PAGEUNPINNED;
            return BFE_PAGEUNPINNED;
        } else {
            BFptr->dirty = TRUE;
            updateLRU(BFptr);
            return BFE_OK;
        }
    }
}
int BF_FlushBuf(int fd){
    BFhash_entry *hashPtr = NULL;
    int i;
    BFhash_entry *tmpPtr = NULL;

    for(i=0; i<BF_HASH_TBL_SIZE;i++){
        hashPtr = hashT[i];

        while(hashPtr!=NULL){
            if(hashPtr->fd == fd){
                if(hashPtr->bpage->count > 0) {/* unpinned: err*/
                    BFerrno = BFE_PAGEPINNED;
                    return BFE_PAGEPINNED;
                }
                tmpPtr = hashPtr->nextentry;
                freeBufferEntryFromHash(hashPtr, i);
                hashPtr = tmpPtr;
            } else
                hashPtr = hashPtr->nextentry;
        }
    }

    /*
    for(i=0; i<BF_HASH_TBL_SIZE; i++){
        assert(hashT[i]==NULL);
    }
    assert(LRUhead==NULL);
    */

    return BFE_OK;
}
void BF_ShowBuf(void){
    BFpage *BFptr;
    int i;
    printf("number of pages in LRU: %d\n", numLRU);
    BFptr = LRUhead;
    for(i=0; i<numLRU; i++){
        printf("%3dth", i);
        printBuff(BFptr);
        BFptr = BFptr->nextpage;
    }
}
void BF_PrintError(const char *s){
    fputs(s, stderr);
    switch(BFerrno){
        case 1:
            /*not initialized*/
            break;
        case 0:
            /*BFE_OK*/
            break;
        case -1:
            fputs("BFE_NOMEM", stderr);
            break;
        case -2:
            fputs("BFE_NOBUF", stderr);
            break;
        case -3:
            fputs("BFE_PAGEPINNED", stderr);
            break;
        case -4:
            fputs("BFE_PAGEUNPINNED", stderr);
            break;
        case -5:
            fputs("BFE_PAGEINBUF", stderr);
            break;
        case -6:
            fputs("BFE_PAGENOTINBUF", stderr);
            break;
        case -7:
            fputs("BFE_INCOMPLETEWRITE", stderr);
            break;
        case -8:
            fputs("BFE_INCOMPLETEREAD", stderr);
            break;
        case -9:
            fputs("BFE_MISSDIRTY", stderr);
            break;
        case -10:
            fputs("BFE_INVALIDTID", stderr);
            break;
        case -11:
            fputs("BFE_MSGERR", stderr);
            break;
        case -12:
            fputs("BFE_HASHNOTFOUND", stderr);
            break;
        case -13:
            fputs("BFE_HASHPAGEEXIST", stderr);
            break;
        default:
            break;
    }
}
