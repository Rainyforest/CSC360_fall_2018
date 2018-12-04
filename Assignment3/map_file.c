#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
   const char *memblock;
   int fd;
   struct stat sb;

   fd = open(argv[1], O_RDONLY);
   fstat(fd, &sb);
   printf("Size: %lu\n\n", (uint64_t)sb.st_size);

   memblock = mmap(NULL, sb.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
   if (memblock == MAP_FAILED) handle_error("mmap");

   int pos = 512*19;

   for(uint64_t i = 96; i < 512; i++)
   {
     printf("%02X ", (unsigned int)(memblock[i+pos]& 0xFF));
     if(i%8 == 7)
	    printf("\n");
     if(i%16 == 15)
	    printf("\n");

   }
   printf("\n");
   return 0;
}
