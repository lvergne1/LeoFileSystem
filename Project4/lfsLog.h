//
//  lfsLog.h
//  
//
//  Created by Leo Vergnetti on 12/4/18.
//

#ifndef lfsLog_h
#define lfsLog_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DRIVESIZE 3906 //BLOCKS
#define BLOCKSIZE 512   //BYTES
#define ROOTDIRECTORY 1 //INUMBER
#define INODESIZE 128   //BYTES
#define NUMDIRECTPOINTERS 20
#define INODESPERBLOCK 4
#define DATABITMAP 2    //BLOCKNUMBER
#define INODEBITMAP 1   //BLOCKNUMBER
#define STARTOFINODETABLE 3 //BLOCKNUMBER
#define STARTOFDATAREGION 488 //BLOCKNUMBER
#define MAXNUMOPENFILES 500 //
#define DIRENTSPERBLOCK 32 

//TODO struct for indirect pointer
//TODO struct for double indirect pointer
/*
 *LAYOUT OF DISK
 *TOTAL SECTORS 3906
 *NUMBER OF INODES 1940
 *TOTAL DATA BLOCKS 3418
 *SUPERBLOCK = BLOCK 0
 *INODE BITMAP = BLOCK 1
 *DATA REGION BITMAP = BLOCK 2
 *INODE REGION = BLOCK 3 - 487
 *DATA REGION = BLOCK 488 - 3906
 
 * MAX SIZE W DIRECT POINTERS : 16*512 = 8,192 bytes
 * MAX SIZE WITH INDIRECT POINTER : 512 bps * 128 = 65,536 + 8,192 = 73,728 bytes
 * MAX SIZE WITH DOUBLE INDIRECT : 8,388,608 bytes (larger than 2MB disk)
 */

typedef struct inode{
    unsigned int inumber;
    int fileType; // -1 for free ;0 for directory ;1 for file
    int length;  //bytes for file, entries for directory
    int numberOfBlocks;
    time_t created; 
    time_t accessed; //long DOUBLE (use with tm struct)
    time_t modified; //long DOUBLE (use with tm struct)
    unsigned int directPointer[NUMDIRECTPOINTERS];
    unsigned int indirectPointer;
    unsigned int doubleIndirectPointer;
}inode;

typedef struct superBlock{
    unsigned int size;
    unsigned int startOfSuperBlock;
    unsigned int startOfInodeBitmap; //block
    unsigned int startOfDataBitmap;//block
    unsigned int startOfInodeTable; //block
    unsigned int startOfDataRegion;//block
    unsigned int sizeOfInode;//bytes
    unsigned int rootInumber;//
    unsigned int availableDataBlocks;
}superBlock;

// DIRECTORY ENTRY = 16 bytes, we can store 32/block
typedef struct lDirectoryEntry{
    unsigned char namelen;
    char fileName[11];
    unsigned int inumber;
}lDirectoryEntry;

typedef struct openFileTableEntry{
    char occupied;
    unsigned int offset;
    inode * Inode;
}openFileTableEntry;

typedef struct openFileTable{
    openFileTableEntry openFiles[MAXNUMOPENFILES];
    int numberOfEntries;
}openFileTable;

//METHODS
inode * newEmptyInode(int i);
void printSuperBlock();
inode * getInodeFromNumber(int inodeNumber);
void printInode(inode in);
int markBitmapAllocated(int bitNumber, int bitmap);
int findFreeInodeFromBitmap();
int findFreeDataBlockFromBitmap();
int writeInodeToDrive(inode * InodeToBeWritten);
void makeDrive(char* driveName);
int mountLFS(char* driveName);
int unmountLFS();
int addFileToTable(inode * inodeOfFile, unsigned int offset);
int lCreateFile(char* fileName);
int writeDirectoryEntryToDrive(lDirectoryEntry entryToBeWritten, unsigned int inumberOfDirectory);
void printDirectoryContents(int inumber, int parentInumber, int level);
int lOpenFile(char* fileName, char* flag);
void printOpenFileTable();
int lCloseFile(int fd);
unsigned int makeEmptyDirectoryBlock();
int lmkDir(char* dirName);
int searchDirByFileName(char* fileName);
int changeCurrentWorkingDirectory(char* dirName);
//int lWriteFile(int fd, char* string);
int lWriteFile(int fd, char c);
char lReadFile(int fd);
int formatDrive(char * fileName);
void printAvailableDataBlocks();
void ls();
void lDeleteFile(char* fileToBeDeleted);
int deallocateBitmap(int bitNumber, int bitmap);
void printCurrentWorkingDirectory();
char* getDirNameByInumber(int inumber);
#endif /* lfsLog_h */
