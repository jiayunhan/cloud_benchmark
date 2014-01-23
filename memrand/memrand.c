#include "memrand.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"
#include "ctime"
int main()
{
  struct timeval start, end;
  long mtime, seconds, useconds;
  //for(int i=0;i<10;i++){
  gettimeofday(&start,NULL);
  node* head = malloc(sizeof(node));
  head->next = NULL;
  init_buffer(&head,1024*400);
  node * tmp = head;
  for(int i=0;i<10;i++){

  probe(head);
  head = tmp;
  }
  gettimeofday(&end,NULL);
  seconds = end.tv_sec-start.tv_sec;
  useconds = end.tv_usec-start.tv_usec;
  mtime = ((seconds)*1000.0+useconds/1000.0)+0.5;
  printf("%ld\n",mtime);
//}
 // printf("node =%d",head);
  return 0;
}
