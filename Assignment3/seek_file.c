#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	FILE *fp = fopen(argv[1], "rb+");	    // b => Binary File, + => Read/Update
	int pos;
	unsigned char buf[320];

	pos = 512 * 19;
	fseek(fp, pos, SEEK_SET);		           // SEEK_SET is the beginning of the file
	fread(buf, sizeof(unsigned char), 320, fp);

	for(uint64_t i = 0; i < 320; i++)
   	{
 		printf("%02X ", (unsigned int)(buf[i]& 0xFF));
     		if(i%8 == 7)
			printf("   ");
     		if(i%16 == 15)
			printf("\n");
		if(i%32 == 31)
			printf("\n");

   	}

	fclose(fp);
	return 0;
}
