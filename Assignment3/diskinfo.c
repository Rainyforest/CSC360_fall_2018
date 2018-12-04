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

/*
    From boot sector get info of OSname.
*/
void getOsName(char* osname, char* memblock) {
	int i;
  int pos = 3;
	for (i = 0; i < 8; i++) {
		osname[i] = memblock[i+pos];
	}
}

/*
    From boot sector get the volume label.
*/
void getLabel(char* label, char* memblock) {
  int i;
  int offset = 43;
  	for (i = 0; i < 8; i++) {
  		label[i] = memblock[i+offset];
  	}
  	if (label[0] == ' ') {
  		offset = BYTES_PER_SECTOR * 19;    //jump to root directory
  		while (memblock [offset] != 0x00) {
  			if (memblock [offset+11] == 0x08) {
  				for (i = 0; i < 8; i++) {
  					label[i] = memblock [offset+i];
  				}
  			}
  			offset += 32;
  		}
  	}
}

/*
    From boot sector get info to calculate the total size.
*/
int getTotalSize(char* memblock){
  /* Get bytes per sector, should be equal to 512 as defined*/
	int bytes_per_sector = (memblock[12] << BITS_PER_BYTE) + memblock[11];
  /* Get sector numbers in total */
	int sector_num = (memblock[20] << BITS_PER_BYTE) + memblock[19];
  /*Total size of the disk = total sector count * bytes per sector*/
  return sector_num * bytes_per_sector;
}

/*
    From fat entry table get fat entry, each entry 12 bits (1.5 bytes).
    Need to mention that bytes are arranged in little endian while bits are arranged in large endian.
*/
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
int getFreeSize(char* memblock){
  int free_sector_num = 0;
  /*the entry should traverse from 2 to 2879, 2880 - 2(reserved) = 2878 entries in total */
  int i;
  for (i = 2; i < (getTotalSize(memblock) / BYTES_PER_SECTOR); i++) {
    if (getFATEntry(i, memblock) == 0x00) {
      free_sector_num++;
    }
  }
  return BYTES_PER_SECTOR * free_sector_num;
}

/* 
    Get sectors per FAT, should equal to 9. 
*/
int getSectorsPerFat(char* memblock){return (memblock[23] << BITS_PER_BYTE) + memblock[22];}

/* 
    Get number of FAT copies, should equal to 2.
*/
int getNumberOfFatCopies(char* memblock){return memblock[16];}



/* 
    Traverse the root directory and count the file numbers.
*/
int getFileNum(char * memblock){
  int offset = BYTES_PER_SECTOR * 19;   //skip to root directory
  int file_num = 0;
  /* first byte of directory entry file name = 0x00 means the rest of directory is free  */
  while (memblock[0+offset] != 0x00) {
    /* get the attributes */
    if ((memblock[11+offset] & 0x10) == 0 && (memblock[11+offset] & 0x08) == 0 && (memblock[11+offset] & 0x20) == 0) {
      file_num++;
    }
    offset += 32;
  }
  return file_num;
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

  char* osname =  malloc(sizeof(unsigned char));
  char* label =  malloc(sizeof(unsigned char));

  /* string processed by the functions are returned in pointers*/
  getOsName(osname,memblock);
  getLabel(label,memblock);

  /* print info */
  printf("OS Name: %s\n",osname);
  printf("Label of the disk: %s\n", label);
  printf("Total size of the disk: %d\n",getTotalSize(memblock));
  printf("Free size of the disk: %d\n",getFreeSize(memblock));
  printf("==============\nThe number of files in the disk (including all files in the root directory and files in all subdirectories)\n%d\n==============\n",getFileNum(memblock));
  printf("Number of FAT copies: %d\n",getNumberOfFatCopies(memblock));
  printf("Sectors per FAT: %d\n",getSectorsPerFat(memblock));
	return 0;
}
