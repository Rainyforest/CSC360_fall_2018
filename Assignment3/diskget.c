#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#define BYTES_PER_SECTOR 512
#define BITS_PER_BYTE 8
#define FILE_NAME_LEN 8
#define FILE_EXTENSION_LEN 3
void getString(int offset,int length,char*result,char* memblock){
  int i;
  for(i = 0;i < length;i++){
      if(memblock[offset + i]==' ')break;
      result[i] = memblock[offset + i];
  }
}
int getFirstLogicalCluster(int offset,char*file_to_copy,char* memblock){
  char* file_name =  malloc(sizeof(char));
  getString(offset,8,file_name,memblock);
  return memblock[offset + 26] + (memblock[offset + 27] << 8);
}
/*
  For a directory entry, if the field of “First Logical Cluster” is 0 or 1, then this directory
  entry should be skipped and not listed.
  (1) the value of it’s atrribute field is 0x0F or
  (2) the 4-th bit of it’s atrribute field is 1 or
  (3) the Volume Label bit of its attribute field is set as 1 or
  (4) the directory_entry is free (see documentation for details)
*/
void rootTraverseFile(char* memblock){
  int offset = BYTES_PER_SECTOR * 19;
  while (memblock[0 + offset] != 0x00) {
    if( memblock[0 + offset] == 0xE5
        || (memblock[11 + offset] & 0x08)
        || memblock[11 + offset] == 0x0F      /* long file name ignore */
        || memblock[26 + offset]  < 0x02){    /* first logical cluster is 0 or 1 */
      offset += 32;
      continue;
    }
    offset += 32;
  }
}
void traverseFile(int offset,char* memblock){
  
}
void copyFromDisk(char* file_name,char* memblock){
  if(1/* find file */){
    /* copy */
  }else{
    printf("File not found.");
    exit(EXIT_FAILURE);
  }
}
int main(int argc, char * argv[]){
  /* check arguments validity */
  if(argc != 3){
    printf("Wrong number of arguments.\nUsage: ./diskinfo <disk Image file> <file to search>\n");
    exit(1);
  }
  /* create a pointer and a state buffer and map the disk */
  char *memblock;
  struct stat buf;
  int fd = open(argv[1], O_RDONLY);
  fstat(fd, &buf);
  memblock = mmap(NULL, buf.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (memblock == MAP_FAILED){
    perror("Error, cannot mmap. Exit."); exit(EXIT_FAILURE);
  }
  char file_name[FILE_NAME_LEN + FILE_EXTENSION_LEN + 1];
  strncpy(file_name,argv[2],FILE_NAME_LEN + FILE_EXTENSION_LEN + 1);
  copyFromDisk(file_name,memblock);
  return 0;
}
