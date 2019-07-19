/* 
    File: my_allocator.c

    Author: Khanh Nguyen
            Department of Computer Science
            Texas A&M University
    Date  : 2/4/2018

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "my_allocator.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/
Header** fl_array;
unsigned int bbs;
unsigned int total;
void* head_ptr; // point to first header of free list

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/


void add_block(Header* node){
    //insert node into the beginning of the tier with size B
    int i=log2(total)-log2(node->size);
    node->next=fl_array[i];
    //fl_array[B] points to the head of that tier
    fl_array[i]=node; 
}

void delete_block(Header* node){
    int i=log2(total)-log2(node->size);
    Header* head=fl_array[i];
    if (head==node){
        fl_array[i]=node->next;
    }
    else{
        while(head->next!=node){
            head=head->next;
        }
        head->next=node->next;
    }
}

Header* combine_blocks(Header *h1, Header *h2){
    //delete 2 small blocks from free list
    delete_block(h1);
    delete_block(h2);
    //merge two blocks into a large block
    Header *h;
    if(h1<h2) h=h1;
    else h=h2;
    h->size*=2;
    //add large block back to free list
    add_block(h);
    return h;
}

/* Don't forget to implement "init_allocator" and "release_allocator"! */
extern unsigned int init_allocator( unsigned int _basic_block_size, unsigned int _length)
{      
    //Validate input  
    if ((_length==0)||(_length<=_basic_block_size))
        return 0;
    else if (_length>_basic_block_size){
        total= pow(2,ceil (log(_length)/log(2)));
        bbs= pow(2,ceil (log(_basic_block_size)/log(2)));// round the bbs to power of 2
        int list_size= (log2(total)-log2(bbs))+1;
        printf("list size: %d \n", list_size);
        //Allocate memory
        head_ptr=(char*)malloc(total);//malloc the chunk memory to first index
        fl_array=(Header**)malloc(list_size*sizeof(Header*));// create free list array
        //Header* head =(Header*)head_ptr;
        fl_array[0]= (Header*)head_ptr;// first index of fl_array is the mem chunk
        fl_array[0]->is_free=true;
        fl_array[0]->size=total;
        fl_array[0]->next=NULL;

    for (int i = 1; i< list_size; i++){
       // printf("i= %d \n",i);
        fl_array[i]=NULL;}
}
}
void split_block(Header* h){
    delete_block(h);
    //split into two blocks??????
    //setup two headers
    Header*h2=(Header*)((char*)h+h->size/2);
    h->size/=2;
    h2->size=h->size;
    h->is_free=h2->is_free=true;
    //add two new blocks to free list
    add_block(h);
    add_block(h2);
}

extern Addr my_malloc(unsigned int _length) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
    printf("allocating %d\n",_length);
    //compute the size needed
    int B=pow(2,ceil (log(_length+sizeof(Header))/log(2)));
    if (B<bbs) B=bbs;
    if (B>total) {
        printf("Not enough memory!\n");
        exit(0);}
    // find a block that satisfies B
    Header* h=NULL;
    int S=B;
    while (h==NULL&&S<=total){
        int index=log2(total)-log2(S);
        printf("index is %d\n",index);
        h=fl_array[index];
        S*=2;
    }
    if (h==NULL){
        printf("Block not found!");
        return 0;
    }
    while (h->size>B){
        printf("spliting\n");
        split_block(h);
    }
    //delete block from free list give block to user
    printf("Deleting\n");
    delete_block(h);
    h->is_free=false;
    return h+1;
}

Addr get_buddy(Addr begin, int size) {
    uint64_t foundBuddy =  ((((uint64_t)begin - (uint64_t)head_ptr) ^ size) + (uint64_t)head_ptr);
    return (Addr) foundBuddy;
}

extern int my_free(Addr _a) {
  /* Same here! */
    //add block back to free list
    Header* h=(Header*)((char*)_a-sizeof(Header));
    h->is_free=true;
    add_block(h);
    //merge if possible
    while (h->size<total){
        Header*buddy=(Header*)get_buddy(h,h->size);
        if (buddy->is_free&& buddy->size==h->size){
            h=combine_blocks(h,buddy);
        }
        else
            break;
    }
  //free(_a);
  return 0;
}


int release_allocator()
{
    /* This function returns any allocated memory to the operating system.
       After this function is called, any allocation fails.
    */
    free((void*)head_ptr);
    free((void*)fl_array);
    printf("deallocated memory\n");
    return 0;
}