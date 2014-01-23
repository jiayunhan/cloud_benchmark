/****************************************************************
 * llc_probe.h -- cache probe helper
 * Author: Venkat
 * Adopted from Toto and Thawan's CacheBench
 * Department of Computer Sciences, UW-Madison
 ****************************************************************/

#ifndef __LLC_PROBE_H__
#define __LLC_PROBE_H__

#include <stdlib.h>
#include <stdio.h>
#define ONE_MB_BYTES (1024*1024UL)
#define ONE_KB_BYTES 1024UL
#define WORD_SIZE 64

typedef struct node {
  struct node* next;
  char pad[WORD_SIZE - sizeof(struct node*)];
} node;

int is_power_of_2(int num)
{
  return ( (num > 0) && ((num & (num - 1)) == 0) );
}


inline void bounded_probe(node *head, unsigned count) {
  node *p;
  int i = count;
  for(p = head; p && (i--); p = p->next);
}

inline void probe(node *head) {
  node *p; 
  
  /*
  int i = 0;
  int stride_nodes = WORD_SIZE / sizeof(struct node);

  printf("\n PROBE -- START\n"); 

  for(p = head; p; p = p->next)
    printf("%d ", (p->next - head)/stride_nodes);

  printf("\n PROBE -- END\n"); 
  */

  for(p = head; p; p = p->next);

}


node* init_buffer(node** buff, unsigned cache_size_kb) {

  unsigned long cache_size = 0;
  unsigned aligned_size_kb = 0;
  node tmp, *head;
  int i, j;
  int stride_nodes, array_nodes, core = -1;

  cache_size = cache_size_kb * ONE_KB_BYTES;

  aligned_size_kb = 1;
  while( aligned_size_kb < cache_size_kb ) {
    aligned_size_kb = aligned_size_kb << 1;
  }

  //printf("Buffer size: %u KB, Aligned at: %u KB\n", cache_size_kb, aligned_size_kb);
  posix_memalign((void**) buff, aligned_size_kb * ONE_KB_BYTES, cache_size);

  /*
  if(! is_power_of_2(cache_size)) {
    unsigned long temp = cache_size;    
  }
  */

  struct node *array = (struct node*) *buff;
  array_nodes =  cache_size / sizeof(struct node);
  stride_nodes = WORD_SIZE / sizeof(struct node);
//jack: stride_nodes may be the step length.

  for (i = 0; i < array_nodes; i += stride_nodes)
    array[i].next = array + i;
  array[0].next = 0;

  // Permute using Sattolo's algorithm for generating a random cyclic permutation:
  //
  for (i = array_nodes / stride_nodes - 1; i > 0; --i) {
    int ii, jj;
    j = random() % i;
    ii = i * stride_nodes;
    jj = j * stride_nodes;
    tmp = array[ii];
    array[ii] = array[jj];
    array[jj] = tmp;
  }
  /* Resulting linked list: */
  head = array;
  //length = array_nodes / stride_nodes;

  /*
test:
  for (i=0; i<array_nodes; i+=stride_nodes) {
    int n = array[i].next?(array[i].next-array):0; assert(n%stride_nodes==0); 
    printf("%d ", n/stride_nodes); 
  } 
  printf(" -- OK\n"); 
  */

  //printf("%s", "Done randomizing buffer list\n");
  return head;
}


#endif  //__LLC_PROBE_H__
