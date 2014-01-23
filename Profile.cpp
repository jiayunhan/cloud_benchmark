#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <resolv.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
//Constant for diskRand
#define DR_BLOCKSIZE 512
#define DR_RANGE 1024000000
#define DR_COUNT 10

//Constant for llc_probe
#define ONE_MB_BYTES (1024*1024UL)
#define ONE_KB_BYTES 1024UL
#define WORD_SIZE 64
#define PROBE_COUNT 100
//Constant for Net
#define NET_IP "141.212.108.231"
#define NET_FILENAME "/file/lfile.0"
#define NET_Port 80
#define NET_BUF_SIZE 4096

typedef struct node{
    struct node* next;
    char pad[WORD_SIZE-sizeof(struct node*)];
}node;


//Variable for time
struct timeval start,end;

void init(){
    srand(unsigned(time(0)));

}
int cache_size_kb(void){
    char line[512],buffer[32];
    char* column;
    FILE *cpuinfo;
    if(!(cpuinfo = fopen("/proc/cpuinfo","r"))){
    	perror("/proc/cpuinfo: fopen");
    	return -1;
    }
    while(fgets(line, sizeof(line), cpuinfo)){
    	if(strstr(line, "cache size")){
    	    column = strstr(line, ":");
    	    strncpy(buffer,++column, sizeof(buffer));
    	    fclose(cpuinfo);
    	    return (int)strtol(buffer,NULL,10);
    	    }
    }
    fclose(cpuinfo);
    return -1;
}
int is_power_of_2(int num){
    return ((num>0)&&((num & (num-1))==0));
}
inline void probe(node *head){
    node *p;
    for(p = head;p;p=p->next);
}
node* init_buffer(node** buff, unsigned cache_size_kb){
    unsigned long cache_size = 0;
    unsigned aligned_size_kb = 0;
    node tmp, *head;
    int i,j;
    int stride_nodes, array_nodes, core = -1;
    cache_size = cache_size_kb * ONE_KB_BYTES;
    aligned_size_kb = 1;
    while(aligned_size_kb < cache_size_kb){
    	aligned_size_kb = aligned_size_kb <<1;
    }
    posix_memalign((void**)buff, aligned_size_kb * ONE_KB_BYTES, cache_size);
    struct node *array = (struct node*) *buff;
    array_nodes = cache_size / sizeof(struct node);
    stride_nodes = WORD_SIZE / sizeof(struct node);
    for (i=0; i<array_nodes; i += stride_nodes)
    	array[i].next = array + i;
    array[0].next = 0;
    //Permute using Sattolo's algoritn for generating a random cyclic permutation:
    for( i = array_nodes / stride_nodes -1 ;i>0 ; --i){
    	int ii,jj;
    	j = random() % i;
    	ii = i * stride_nodes;
    	jj = j * stride_nodes;
    	tmp = array[ii];
    	array[ii] = array[jj];
    	array[jj] = tmp;
    }
    //Resulting linked list:
    head = array;
    return head;
}
long mtime(struct timeval start, struct timeval end){
    long seconds = end.tv_sec - start.tv_sec;
    long useconds = end.tv_usec-start.tv_usec;
    return  ((seconds)*1000.0 + useconds/1000.0)+0.5;
    }
long cacheProbe(unsigned cache_size_kb, int count){
    node* head =(node*) malloc(sizeof(node));
    long *results = new long[count];
    head->next = NULL;
    init_buffer(&head,cache_size_kb);
    node* tmp = head;
    for(int i=0; i<count;i++){
    	gettimeofday(&start,NULL);
    	for(int j=0;j<PROBE_COUNT;j++){
    	    probe(head);
    	    head = tmp;
	}
	gettimeofday(&end,NULL);
	results[i] = mtime(start,end);
    }
    long sum = 0;
    for(int i=0;i<count;i++){
    	sum+=results[i];
    }
    return sum/count;
}
long memProbe(unsigned mem_size_kb,int count){
    node* head =(node*) malloc(sizeof(node));
    long* results = new long[count];
    head->next = NULL;
    init_buffer(&head,mem_size_kb);
    node* tmp = head;
    for(int i=0;i<count;i++){
    	gettimeofday(&start,NULL);
    	for(int j=0;j<1;j++){
    	    probe(head);
    	    head = tmp;
	}
	gettimeofday(&end,NULL);
	results[i] = mtime(start,end);
//	printf("mem time = %ld\n",results[i]);
    }
    long sum = 0;
    for(int i=0;i<count;i++){
    	sum += results[i];
    }
    return sum/count;
}
long diskRand(char* diskname,int count){
    char buf[DR_BLOCKSIZE];
    long * results = new long [count];
    std::ifstream in(diskname,std::ifstream::binary);
    if(!in){
    	printf("open error.\n");
    }
    int cnt = DR_RANGE/DR_BLOCKSIZE;
    for(int i=0;i<count;i++){
    	gettimeofday(&start,NULL);
    	for(int j=0;j<100;j++){
    	    int offset = (rand()%(cnt-1))*DR_BLOCKSIZE;
    	    if(!in.seekg(offset)){
    	    	printf("seek error.\n");
    	    	return 0;
	    }
	    in.read(buf,DR_BLOCKSIZE);
	}
	gettimeofday(&end,NULL);
	results[i] = mtime(start,end);
    }
    long sum = 0;
    for(int i=0;i<count;i++){
    	sum += results[i];
    }
    return sum/count;
}
long Net(char* filename,int count){
    int sockfd, bytes_read;
    struct sockaddr_in dest;
    char buffer[NET_BUF_SIZE];
    long* results = new long[count];
    for(int i=0;i<count;i++){
    gettimeofday(&start,NULL);
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
    	printf("socket error\n");
    	return -1;
    }
    bzero(&dest,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(80);
    dest.sin_addr.s_addr = inet_addr(NET_IP);
    if(connect(sockfd,(struct sockaddr*)&dest, sizeof(dest))!=0){
    	printf("connect error\n");
    	return -3;
    }
    sprintf(buffer,"GET %s HTTP/1.0\n\n",filename);
    send(sockfd,buffer,strlen(buffer),0);
    do{
    	bzero(buffer,sizeof(buffer));
    	bytes_read = recv(sockfd,buffer,sizeof(buffer),0);
    	//if(bytes_read>0)
    	//    printf("%s\n",buffer);
    }
    while(bytes_read>0);
    gettimeofday(&end,NULL);
    results[i] = mtime(start,end);
    close(sockfd);
    }
    int sum = 0;
    for(int i=0;i<count;i++){
    	sum+=results[i];
    }
    return sum/count;
}
int inferBottleneck(){
    long dr = diskRand("dev/xvda2",10);
    long cp = cacheProbe(9*1024,10);
    long mp = memProbe(400*1024,10);
    long net = Net(NET_FILENAME,1);
    printf("diskRand = %ld\n cacheProbe = %ld\n memProbe = %ld\n Net = %ld\n",dr,cp,mp,net);
    return 0;
}
int main(){
    printf("cache size = %d",cache_size_kb());
    //init();
    //long dr =  diskRand("/dev/xvda2",10);
    //long mp = memProbe(400*1024,10);
    //long net = Net(NET_FILENAME,1);
    //printf("avg = %ld\n",net);
}



