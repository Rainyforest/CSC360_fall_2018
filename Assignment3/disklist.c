#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#define BYTES_PER_SECTOR 512
#define BITS_PER_BYTE 8
#define DATE_OFFSET 16
#define TIME_OFFSET 14


void getString(int offset,int length,char*result,char* memblock){
  int i;
  for(i = 0;i < length;i++){
      if(memblock[offset + i]==' ')break;
      result[i] = memblock[offset + i];
  }
}
int getFATEntry(int n, char* memblock){
  int offset = BYTES_PER_SECTOR * 1; // skip the boot sector
  int first_byte;
  int second_byte;
  double entry_pos = (n+1)*1.5;
  if(entry_pos == floor(entry_pos)){
    first_byte = memblock[offset + (int)floor(entry_pos) - 1] & 0xFF;
    second_byte = memblock[offset + (int)floor(entry_pos) - 2] & 0xF0;
    return (first_byte << BITS_PER_BYTE/2) + (second_byte >> BITS_PER_BYTE/2);
  }else{
    first_byte = memblock[offset + (int)floor(entry_pos) ] & 0x0F;
    second_byte = memblock[offset + (int)floor(entry_pos) - 1] & 0xFF;
    return (first_byte << BITS_PER_BYTE) + second_byte;
  }
}
int getNextAddress(int offset,char* memblock){
    int first_logical_cluster = (memblock[26]) + (memblock[27] << 8);
  	return first_logical_cluster + 31;
}
int getNextDataOffset(int address){
  return (BYTES_PER_SECTOR + address)*31;
}
/*
    Input the start pointer offset of a file entry and return file size.
*/
int getFileSize(int offset, char* memblock){
  return  (memblock[offset + 31] & 0xFF << 24)
          + (memblock[offset + 30] & 0xFF << 16)
          + (memblock[offset + 29] & 0xFF << 8)
          + (memblock[offset + 28] & 0xFF);
}

void printFileInfo(int offset, char* memblock){
  int file_size;
  char* file_name =  malloc(sizeof(char));
  char* file_extension =  malloc(sizeof(char));
  int hours, minutes, day, month, year;


    /* clear the file info buffer */
    int i;
    for(i = 0;i < 8;i++){
      file_name[i] = '\0';
      if(i<3)file_extension[i] = '\0';
    }
    file_size = getFileSize(offset, memblock);
    getString(offset,8,file_name,memblock);
    getString(offset + 8, 3, file_extension, memblock);
    //the year is stored in the high seven bits
    year = ((memblock[offset + DATE_OFFSET + 1] & 0xFE) >> 1) + 1980;
    //the month is stored in the middle four bits
    month = ((memblock[offset + DATE_OFFSET + 1] & 0x01) << 3) + ((memblock[offset + 16] & 0xE0) >> 5);
    //the day is stored in the low five bits
    day = (memblock[offset + DATE_OFFSET] & 0x1F);


    //the hours are stored in the high five bits
    hours = (memblock[offset + TIME_OFFSET + 1] & 0xF8) >> 3;
    //the minutes are stored in the middle 6 bits
    minutes = ((memblock[offset + TIME_OFFSET + 1] & 0x07) << 3) + ((memblock[offset + TIME_OFFSET] & 0xE0) >> 5);
    printf("∗ %s %10d %20s.%3s %d-%02d-%02d %02d:%02d\n",((memblock[11+offset] & 0x10)==0)?"F":"D",file_size,file_name,file_extension,year, month, day,hours,minutes);

}
void traverseFile(char* memblock,void (*f)(int,char*),int offset){
  while (memblock[0 + offset] != 0x00) {
    if( memblock[0 + offset] == 0xE5
        || (memblock[11 + offset] & 0x08)
        || memblock[11 + offset] == 0x0F      /* long file name ignore */
        || memblock[26 + offset]  < 0x02){    /* first logical cluster is 0 or 1 */
      offset += 32;
      continue;
    }
    (*f)(offset,memblock);
    if(memblock[11+offset] & 0x10){
      printf("==================\n");
      int new_offset = getNextDataOffset(getNextAddress(offset,memblock));
      traverseFile(memblock,*f,new_offset);
    }
    
    offset += 32;
  }
}

void rootTraverseFile(char* memblock,void (*f)(int,char*)){
  int offset = BYTES_PER_SECTOR * 19;
  while (memblock[0 + offset] != 0x00) {
    if( memblock[0 + offset] == 0xE5
        || (memblock[11 + offset] & 0x08)
        || memblock[11 + offset] == 0x0F      /* long file name ignore */
        || memblock[26 + offset]  < 0x02){    /* first logical cluster is 0 or 1 */
      offset += 32;
      continue;
    }
    (*f)(offset,memblock);
    if(memblock[11+offset] & 0x10){
      printf("==================\n");
      int new_offset = getNextDataOffset(getNextAddress(offset,memblock));
      traverseFile(memblock,*f,new_offset);
    }
    offset += 32;
  }
}

int main(int argc, char *argv[])
{
  /* check arguments validity */
  if(argc!=2){
    printf("Wrong number of arguments.\nUsage: ./diskinfo <disk file name>\n");
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
  printf("Directory Name\n");
  printf("==================\n");
  rootTraverseFile(memblock,printFileInfo);

	return 0;
}
