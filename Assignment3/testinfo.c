/*
	University of Victoria
	Fall 2016
	CSC 360 Assignment 3 Part 1
	Michael Reiter
	V00831568
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "a3helpers.h"

/* ---------- Helper functions ---------- */

/*
	osname: a char array to read the os name into
	p: a pointer to the mapped memory
	Reads the OS name read from a disk image
*/
void getOsName(char* osname, char* p) {
	int i;
	for (i = 0; i < 8; i++) {
		osname[i] = p[i+3];
	}
}

/*
	p: a pointer to the mapped memory
	Returns the number of sectors per FAT read from p
*/
int getSectorsPerFat(char* p) {
	return p[22] + (p[23] << 8);
}

/*
	diskLabel: a character pointer to store the volume label
	p: a pointer to the mapped memory
	Reads the volume label into diskLabel
*/
void getDiskLabel(char* diskLabel, char* p) {
	int i;
	for (i = 0; i < 8; i++) {
		diskLabel[i] = p[i+43];
	}

	if (diskLabel[0] == ' ') {
		p += SECTOR_SIZE * 19;
		while (p[0] != 0x00) {
			if (p[11] == 8) {
				for (i = 0; i < 8; i++) {
					diskLabel[i] = p[i];
				}
			}
			p += 32;
		}
	}
}

/*
	p: a pointer to the mapped memory
	Returns the number of files in the root directory
*/
int getNumberOfRootFiles(char* p) {
	p += SECTOR_SIZE * 19;
	int count = 0;

	while (p[0] != 0x00) {
		if ((p[11] & 0b00000010) == 0 && (p[11] & 0b00001000) == 0 && (p[11] & 0b00010000) == 0) {
			count++;
		}
		p += 32;
	}

	return count;
}

/*
	p: a pointer to the mapped memory
	Returns the number of copies of the FAT
*/
int getNumberOfFatCopies(char* p) {
	return p[16];
}

/*
	osName: name of operating system
	diskLabel: label of volume
	diskSize: total disk size in bytes
	freeSize: total free disk size in bytes
	numberOfRootFiles: number of files in the disk's root directory excluding subdirectories
	numberOfFatCopies: number of copies of the disk's FAT
	sectorsPerFat: number of sectors in a FAT
	Prints file system image information
*/
void printInfo(char* osName, char* diskLabel, int diskSize, int freeSize, int numberOfRootFiles, int numberOfFatCopies, int sectorsPerFat) {
	printf("OS Name: %s\n", osName);
	printf("Label of the disk: %s\n", diskLabel);
	printf("Total size of the disk: %d bytes\n", diskSize);
	printf("Free size of the disk: %d bytes\n\n", freeSize);
	printf("==============\n");
	printf("The number of files in the root directory (not including subdirectories): %d\n\n", numberOfRootFiles);
	printf("==============\n");
	printf("Number of FAT copies: %d\n", numberOfFatCopies);
	printf("Sectors per FAT: %d\n", sectorsPerFat);
}

/* ---------- Main ---------- */

/*
	Displays information about an input file system
*/
int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Error: use as follows diskinfo <file system image>\n");
		exit(1);
	}

	// Open disk image and map memory
	int fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Error: failed to read disk image\n");
		exit(1);
	}
	struct stat buf;
	fstat(fd, &buf);
	char* p = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	// Read information from disk image then print it
	char* osName = malloc(sizeof(char));
	getOsName(osName, p);
	char* diskLabel = malloc(sizeof(char));
	getDiskLabel(diskLabel, p);
	int diskSize = 0;
	int freeSize = 1;
	int numberOfRootFiles = getNumberOfRootFiles(p);
	int numberOfFatCopies = getNumberOfFatCopies(p);
	int sectorsPerFat = getSectorsPerFat(p);
	printInfo(osName, diskLabel, diskSize, freeSize, numberOfRootFiles, numberOfFatCopies, sectorsPerFat);

	// Clean up
	munmap(p, buf.st_size);
	close(fd);
	free(osName);
	free(diskLabel);

	return 0;
}
