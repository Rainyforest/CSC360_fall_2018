/* ---------- Constants and global variables ---------- */

#define TRUE 1
#define FALSE 0
#define SECTOR_SIZE 512

/* ---------- Helper functions ---------- */

int getFatEntry(int n, char* p);
int getTotalDiskSize(char* p);
int getFreeDiskSize(int diskSize, char* p);
int getFileSize(char* fileName, char* p);
int getFirstLogicalSector(char* fileName, char* p);
int diskContainsFile(char* fileName, char* p);
