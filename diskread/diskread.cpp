#include <fstream>
#include <cerrno>
#include <stdexcept>
#include <cstring>
#include <iostream>
#define DISKNAME "/dev/xvda2"
using namespace std;
int main(int argc, char* argv){
  char diskName[]=DISKNAME;
  ifstream disk(diskName,ios_base::binary);
  if(!disk)
     cout<<"open failed\n";
  return 0;
}
