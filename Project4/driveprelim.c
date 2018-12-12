#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BLOCKSIZE 512
#define ROOTDIRECTORY 1
#define INODESIZE 128
#define NUMDIRECTPOINTERS 16
#define INODESPERBLOCK 4
#define DATABITMAP 2
#define INODEBITMAP 1



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
    long double accessed; //long DOUBLE (use with tm struct)
    long double modified; //long DOUBLE (use with tm struct)
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
    unsigned int rootInumber;
}superBlock;

// DIRECTORY ENTRY = 16 bytes, we can store 32/block
typedef struct lDirectoryEntry{
    unsigned char namelen;
    char dirName[11];
    unsigned int inumber;
    
}lDirectoryEntry;


void printSuperBlock();
void getInodeFromNumber(int inodeNumber);
void printInode(inode in);
int markBitmapAllocated(int bitNumber, int bitmap);
int findFreeInodeFromBitmap();
void findFreeDataBlockFromBitmap();
int writeInodeToDrive(inode * InodeToBeWritten);
int makeLFS(FILE *drive);
//PART OF MAKE LFS

 inode * newEmptyInode(int i){
    inode * emptyInode = malloc(sizeof(inode));
    emptyInode->fileType = -1;
    emptyInode->inumber = i;
    emptyInode->accessed = 0;
    emptyInode->modified = 0;
    int j;
    for(j = 0; j <  NUMDIRECTPOINTERS; j++){
        emptyInode->directPointer[j] = 0;
    }
    emptyInode->indirectPointer = 0;
    emptyInode->doubleIndirectPointer = 0;
    return emptyInode;
}


superBlock sb;
FILE * drive; //MUST BE PRESENT IN FILESYSTEM! drive will point to drive FILE when mounting.

//PART OF LFS AND MAKELFS

/***************************************************
 METHOD: makeLFS
 INPUT: file : FILE*
 OUTPUT: 0 on success, -1 else
 DESCRIPTION: makeLFS creates an LFS on the specified
 file pointer.  MUST BE 2MB.
***************************************************/
int makeLFS(FILE *file){
    
}

/***************************************************
 METHOD: markBitmapAllocated
 INPUT: bitNumber: int, bitmap: int
 OUTPUT: 0 on success, -1 on error
 
 DESCRIPTION: markBitmapAllocated takes a bitnumber
 (corresponding to an inode or a data block, depending
 on which instance is desired) and changes its bit
 to 1 in the bitmap
 ***************************************************/
int markBitmapAllocated(int bitNumber, int bitmap){
    
    int byteNumber = bitNumber/8;
    char bytes[BLOCKSIZE];
    //GO TO CORRECT BITMAP
    if (fseek(drive, bitmap * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    //LOAD BITMAP TO BYTES ARRAY
    fread(&bytes, sizeof(char), BLOCKSIZE, drive);
    //READ bytes[byteNumber]

    int localBitNumber = bitNumber % 8;
    int mask = 1 << localBitNumber;
    bytes[byteNumber] = bytes[byteNumber] | mask;
    //write bytes back to sector;
    if (fseek(drive, bitmap * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fwrite(&bytes, BLOCKSIZE *sizeof(char), 1, drive);
    return 0;
}

int findFreeInodeFromBitmap(){
    
  //  int byteNumber = bitNumber/8;
    unsigned char bytes[BLOCKSIZE];
    //GO TO INODE BITMAP
    if (fseek(drive, INODEBITMAP * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        exit(-1);
    }
    //LOAD BITMAP TO BYTES ARRAY
    fread(&bytes, sizeof(char), BLOCKSIZE, drive);
    int i;
    for(i = 0; i < BLOCKSIZE; i++){
        if(bytes[i]!= 0xFF){
            int j;
            for(j = 0; j < 8; j++){
                int mask = 1;
                if(((mask << j) & bytes[i]) == 0){
                    printf("inode number %d is FREE\n", (i*8)+j);
                   // printf("bit %d of byte %d is FREE\n", j, i);
                    return(j);
                }
            }
        }
    }
    return -1;
}
void findFreeDataBlockFromBitmap(){
}

/*************************************
 METHOD: mklDir() makes a directory
 *************************************/



int writeInodeToDrive(inode * InodeToBeWritten){
    //FIND BLOCKNUMBER
    int inodeNumber = InodeToBeWritten->inumber;
    int blockNumber = (inodeNumber/INODESPERBLOCK) + sb.startOfInodeTable;
    int offset = inodeNumber % INODESPERBLOCK;
    //GO TO LOCATION
    if (fseek(drive, blockNumber * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    //ALLOCATE AN ARRAY OF INODES SINCE WE MUST READ THE WHOLE BLOCK
    inode inodeBlock[INODESPERBLOCK];
    //READ BLOCK CONTAINING INODE
    fread(&inodeBlock, sizeof(inode), INODESPERBLOCK, drive);
    inodeBlock[offset] = *InodeToBeWritten;
    if (fseek(drive, blockNumber * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fwrite(&inodeBlock, sizeof(inode), INODESPERBLOCK, drive);
    return(0);
    
}
void getInodeFromNumber(int inodeNumber){
    //(inodeNumber * BLOCKSIZE)//
    //FIND BLOCKNUMBER
    int blockNumber = (inodeNumber/INODESPERBLOCK) + sb.startOfInodeTable;
    int offset = inodeNumber % INODESPERBLOCK;
    //GO TO LOCATION
    if (fseek(drive, blockNumber * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        exit(-1);
    }
    //ALLOCATE AN ARRAY OF INODES SINCE WE MUST READ THE WHOLE BLOCK
    inode inodeBlock[INODESPERBLOCK];
    //READ BLOCK CONTAINING INODE
    fread(&inodeBlock, sizeof(inode), INODESPERBLOCK, drive);
    
    printInode(inodeBlock[offset]);
}


void printSuperBlock(){
    printf("Drive Size: %u\n", sb.size);
    printf("Superblock number: %u\n", sb.startOfSuperBlock);
    printf("Inode Bitmap Block: %u\n", sb.startOfInodeBitmap);
    printf("Data Bitmap Block: %u\n", sb.startOfDataBitmap);
    printf("Start of Inode Table: %u\n", sb.startOfInodeTable);
    printf("Start of Data Region: %u\n", sb.startOfDataRegion);
    printf("Size of Inode: %u\n", sb.sizeOfInode);
    printf("Root Inode Inumber: %u\n", sb.rootInumber);
}

void printInode(inode in){
    printf("inumber: %u\n", in.inumber);
    printf("inode type: %d\n", in.fileType);
    printf("accessed: %Lf\n", in.accessed);
    printf("modified: %Lf\n", in.modified);
    int i;
    for(i = 0; i < NUMDIRECTPOINTERS; i++){
        printf("Direct pointer[%d]: %u\n", i, in.directPointer[i]);
    }
    printf("Indirect pointer: %u\n", in.indirectPointer);
    printf("Double indirect pointer: %u\n", in.doubleIndirectPointer);
}

int main(int argc, char** argv){
    //INIT SUPERBLOCK STRUCTURE
   // superBlock sb;
    sb.size = 2000000;
    sb.startOfSuperBlock = 0; //block
    sb.startOfInodeBitmap = 1; //block
    sb.startOfDataBitmap = 2; //block
    sb.startOfInodeTable = 3; //block
    sb.startOfDataRegion = 488; //block
    sb.sizeOfInode = 128; //bytes
    sb.rootInumber = ROOTDIRECTORY;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    printf("Size of ldirectory Entry: %lu\n", sizeof(lDirectoryEntry));
    printf("Size of char : %lu\n", sizeof(char));
    printf("Size of super block: %lu\n", sizeof(superBlock));
    printf("Size of inode: %lu\n", sizeof(inode));
    printf("Size of unsigned int: %lu\n", sizeof(unsigned int));
    printf("Value of time_t : %ld", t);
    printf("Size of time_t : %lu\n", sizeof(&t));
    printf("Size of tm : %lu\n", sizeof(tm));
    printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    
    
    // CREATE FILESYSTEM
    
    //WRITE SUPERBLOCK
    
    drive = fopen("Drive2MB", "r+b");
    if (drive == NULL){
        printf("Error loading file, exiting\n");
        return(-1);
    }
    FILE *textFile = fopen("textFile.txt", "r+b");
    if (drive == NULL){
        printf("Error loading file, exiting\n");
        return(-1);
    }
    
    
    fwrite(&sb, sizeof(superBlock), 1, drive);
    
    
    
    //WRITE EMPTY INODE TABLE:
   
    if (fseek(drive, sb.startOfInodeTable * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    
    int i;
    for(i = 0; i < INODESPERBLOCK  * (sb.startOfDataRegion - sb.startOfInodeTable); i++){
        inode * emptyInode = newEmptyInode(i);
        fwrite(emptyInode, sizeof(inode), 1, drive);
        free(emptyInode);
    }
    
    inode * rootInode = newEmptyInode(1);
    rootInode->fileType = 0;
    rootInode->length = 1;
    rootInode->directPointer[0] = sb.startOfDataRegion;
    rootInode->accessed = time(NULL);
    rootInode->modified = time(NULL);
    writeInodeToDrive(rootInode);
    markBitmapAllocated(1, INODEBITMAP);
    markBitmapAllocated(0, DATABITMAP);
    lDirectoryEntry ldirent[32];
    ldirent[0].dirName[0] = '.';
    ldirent[0].inumber = 1;
    ldirent[0].namelen = strlen(ldirent[0].dirName);
    printf("result of strlen on dirName: %d\n", (int)ldirent[0].namelen);
    fseek(drive, sb.startOfDataRegion * BLOCKSIZE, SEEK_SET);
    for(i = 1; i < 32; i++){
        ldirent[i].inumber = 4;
    }
    fwrite(&ldirent, sizeof(lDirectoryEntry), 32 , drive);
    free(rootInode);
    getInodeFromNumber(1);
    //MAKE ROOT DIRECTORY()
    //lDir ldir;
    //ldir.inumber = 1;
    //ldir.numberOfEntries = 1;
    //ldir.
    /*
    fclose(drive);
    
    drive = fopen("Drive2MB", "r+b");
    if (drive == NULL){
        printf("Error loading file, exiting\n");
        return(-1);
    }
    if (fseek(drive, sb.startOfInodeTable * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    for(i = 0; i < INODESPERBLOCK  * (sb.startOfDataRegion - sb.startOfInodeTable); i++){
        inode readInode;
        fread(&readInode, sizeof(inode), 1, drive);
       // printInode(readInode);
    }
    
    getInodeFromNumber(12);
     
    for(int i = 0; i < 8; i++){
        markBitmapAllocated(i ,INODEBITMAP);
    }
    findFreeInodeFromBitmap();
    //markBitmapAllocated(4094, DATABITMAP);
    */
    //TO MAKE FILE SYSTEM
    //FIRST GENERATE AND WRITE SUPERBLOCK
    // MAKE INODE BITMAP (MARK AS OCCUPIED ALL INODES >= 1940(MAX NUMBER POSSIBLE))
    // MAKE DATA BITMAP (MARK AS OCCUPIED ALL DATA BLOCKS >= 3418 MAX NUMBER OF DATA BLOCKS
    //CREATE INODE TABLE
    //MAKE ROOT DIRECTORY (
    
    
    /*
    file = fopen("Drive2MB", "r+b");
    if (file == NULL){
        printf("Error loading file, exiting\n");
        return(-1);
    }
    
    superBlock sb2;
    fread(&sb2, sizeof(superBlock), 1, file);
  
    printf("Read from drive\n");
    printSuperBlock(sb2);
    */
    /*int c;
    do {
        c = fgetc(textFile);
        if( feof(textFile) ) {
            break ;
        }
        fprintf(file, "%c", c);
    } while(1);
    
    fputc('a', file);
    fputc('b', file);
    fputc('c', file);
    fputc('d', file);
    
    char* filePath = strdup("/directory/file");
    char* currentToken;
    if (fseek(file, BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", BLOCKSIZE);
        return(-1);
    }
    while( (currentToken = strsep(&filePath, "/"))!= NULL){
        fprintf(file, "%s\n", currentToken);
    }
    if (fseek(file, 2*BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", BLOCKSIZE);
        return(-1);
    }
    
    fputc('e', file);
    char* systemType = "LeoFileSystem";
    fprintf(file, "%s", systemType);
    
     while(1){
     c = fgetc(file);
     printf("read a char: %x\n", c);
     if(feof(file)){
     printf("we have reached the end of file\n");
     break;
     }
     }
     */
    
    //printSuperBlock();
    
    fclose(textFile);
    fclose(drive);
}
