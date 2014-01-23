#include <fstream>
#include <sys/stat.h>
#include <sys/time.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#define FLAGS_blocksize 512
#define fileSize 2048000000
#define COUNT 100
int main(){
  struct timeval start, end;
  long mtime, seconds, useconds;
  
  char buf[FLAGS_blocksize];
  srand(unsigned(time(0)));
  std::ifstream in("/dev/xvda2",std::ifstream::binary);
  if(!in){
	printf("open error.\n");
  }
  int count = fileSize/FLAGS_blocksize;
  for(int i=0;i<10;i++)
  {
  gettimeofday(&start,NULL);

////////////////////
    for(int j=0;j<500;j++){
    int offset = (rand()%(count-1))*512;
    //printf("offset = %d\n",offset);
    if(!in.seekg(offset)){
       printf("seek error\n");
       return 0;
    }
    in.read(buf,512);
    }
//    if(write(fd,buf,FLAGS_blocksize)==-1){
//       printf("write error\n");
//    }
  
  /////////////////////////////////////
  gettimeofday(&end,NULL);
  seconds = end.tv_sec-start.tv_sec;
  useconds = end.tv_usec-start.tv_usec;
  mtime = ((seconds)*1000.0+useconds/1000.0)+0.5;
  printf("%ld\n",mtime);
  //if(remove("./test.svg")==-1)
  //    return -1;
  }
  in.close();
  return 0;
}
