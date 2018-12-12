//
//  lfsLog.c
//  
//
//  Created by Leo Vergnetti on 12/4/18.
//

#include "lfsLog.h"


inode * newEmptyInode(int i){
    inode * emptyInode = malloc(sizeof(inode));
    emptyInode->fileType = -1;
    emptyInode->inumber = i;
    emptyInode->numberOfBlocks = 0;
    emptyInode->created = 0;
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
openFileTable OFT;
unsigned int currentWorkingDirectory;
char *currentDirectoryName;
/***************************************************
 ***************************************************/
int mountLFS(char* driveName){
    
    drive = fopen(driveName, "r+b");
    if(drive ==NULL){
        printf("Unable to open drive file\n");
        return(-1);
    }
    if(fseek(drive, 0 * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
    }
    fread(&sb, sizeof(superBlock), 1, drive);
    printf("Drive successfully mounted\n");
    
    //initialize openFileTable
    int i;
    for(i = 0; i < MAXNUMOPENFILES; i++){
        OFT.openFiles[i].occupied = 0;
    }
    OFT.numberOfEntries = 0;
    currentWorkingDirectory = sb.rootInumber;
    currentDirectoryName = malloc(sizeof(char)*12);
    currentDirectoryName = strdup("/");
    return(0);
}

int unmountLFS(){
    if(fseek(drive, 0 * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
    }
    fwrite(&sb, sizeof(superBlock), 1, drive);
    
    if(fclose(drive) != 0){
        printf("ERROR unmounting drive\n");
        return(-1);
    }
    return 0;
}

/***************************************************
 METHOD: makeLFS
 INPUT: file : FILE*
 OUTPUT: 0 on success, -1 else
 DESCRIPTION: makeLFS creates an LFS on the specified
 file pointer.  MUST BE 2MB.
 ***************************************************/
int formatDrive(char* driveName){
    sb.size = DRIVESIZE;
    sb.startOfSuperBlock = 0; //block
    sb.startOfInodeBitmap = INODEBITMAP; //block
    sb.startOfDataBitmap = DATABITMAP; //block
    sb.startOfInodeTable = STARTOFINODETABLE; //block
    sb.startOfDataRegion = STARTOFDATAREGION; //block
    sb.sizeOfInode = INODESIZE; //bytes
    sb.rootInumber = ROOTDIRECTORY;
    sb.availableDataBlocks = DRIVESIZE - STARTOFDATAREGION - 1; //USING ONE FOR ROOT INODE
    drive = fopen(driveName, "r+b");
    if(drive == NULL){
        printf("UNABLE TO OPEN FILE %s\n", driveName);
        return(-1);
    }
    if(fseek(drive, 0 * BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
    }
    
    fwrite(&sb, sizeof(superBlock), 1, drive);
    
    
    
    //CREATE AND POPULATE INODE TABLE
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
    
    printf("INODE TABLE CREATED\n");
    //MARK UNUSED INODE BITMAP SPACE
    for(i = INODESPERBLOCK * (STARTOFDATAREGION-STARTOFINODETABLE); i < BLOCKSIZE * 8; i++){
        markBitmapAllocated(i, INODEBITMAP);
    }
    printf("INODE BITMAP UPDATED\n");
    
    //MARK UNUSED DATA BITMAP SPACE
    for(i = DRIVESIZE; i < BLOCKSIZE * 8; i++){
        markBitmapAllocated(i, DATABITMAP);
    }
    //CREATE ROOT DIRECTORY AND ADD TO INODETABLE
    inode * rootInode = newEmptyInode(1);
    rootInode->fileType = 0;
    rootInode->length = 1;
    rootInode->numberOfBlocks = 1;
    rootInode->directPointer[0] = sb.startOfDataRegion;
    rootInode->accessed = time(NULL);
    rootInode->modified = time(NULL);
    writeInodeToDrive(rootInode);
    markBitmapAllocated(0, INODEBITMAP);
    markBitmapAllocated(1, INODEBITMAP);
    markBitmapAllocated(0, DATABITMAP);
    lDirectoryEntry ldirent[32];
    ldirent[0].fileName[0] = '.';
    ldirent[0].inumber = 1;
    ldirent[0].namelen = 1;
    fseek(drive, sb.startOfDataRegion * BLOCKSIZE, SEEK_SET);
    for(i = 1; i < 32; i++){
        ldirent[i].namelen = 0;
    }
    fwrite(&ldirent, sizeof(lDirectoryEntry), 32 , drive);
    free(rootInode);
    fclose(drive);
    return(0);
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



int deallocateBitmap(int bitNumber, int bitmap){
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
    if((bytes[byteNumber] & mask ) != 0){

        if(bitmap == DATABITMAP){
                sb.availableDataBlocks += 1;
        }
        bytes[byteNumber] = bytes[byteNumber] ^ mask;
    }
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
                    return((i*8)+j);
                }
            }
        }
    }
    printf("ERROR unable to find empty inode\n");
    return -1;
}
int findFreeDataBlockFromBitmap(){
    unsigned char bytes[BLOCKSIZE];
    //GO TO INODE BITMAP
    if (fseek(drive, DATABITMAP * BLOCKSIZE, SEEK_SET)!= 0){
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
                    sb.availableDataBlocks = sb.availableDataBlocks - 1;
                    return((i*8)+j);
                }
            }
        }
    }
    printf("ERROR unable to find empty dataBlock\n");
    return -1;
}

/*************************************
 METHOD: mklDir() makes a directory
 *************************************/


int writeDirectoryEntryToDrive(lDirectoryEntry entryToBeWritten, unsigned int inumberOfDirectory){
    inode *Inode = getInodeFromNumber(inumberOfDirectory);
    Inode->length = Inode->length + 1;
    int i;
    for(i = 0; i < Inode->numberOfBlocks; i++){
        lDirectoryEntry ldirent[DIRENTSPERBLOCK];
        
        if (fseek(drive, Inode->directPointer[i] * BLOCKSIZE, SEEK_SET)!= 0){
            printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
            return(-1);
        }
        fread(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
        int j;
        for(j = 0; j < DIRENTSPERBLOCK; j++){
            if(ldirent[j].namelen == 0){
                ldirent[j] = entryToBeWritten;
                if (fseek(drive, Inode->directPointer[i] * BLOCKSIZE, SEEK_SET)!= 0){
                    printf("Error finding location blocksize %d in file\n", Inode->directPointer[i] * BLOCKSIZE);
                    return(-1);
                }
                fwrite(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK , drive);
                writeInodeToDrive(Inode);
                return(0);
            }
        }
    }
    return(-1);
}

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
inode * getInodeFromNumber(int inodeNumber){
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
    inode * inodeToReturn = malloc(sizeof(inode));
    inodeToReturn->inumber = inodeBlock[offset].inumber;
    inodeToReturn->fileType = inodeBlock[offset].fileType;
    inodeToReturn->length = inodeBlock[offset].length;
    inodeToReturn->numberOfBlocks =  inodeBlock[offset].numberOfBlocks;
    inodeToReturn->created = inodeBlock[offset].created;
    inodeToReturn->modified = inodeBlock[offset].modified;
    inodeToReturn->accessed = time(NULL);
    int i;
    for(i = 0; i < NUMDIRECTPOINTERS; i++){
        inodeToReturn->directPointer[i] = inodeBlock[offset].directPointer[i];
    }
    inodeToReturn->indirectPointer = inodeBlock[offset].indirectPointer;
    inodeToReturn->doubleIndirectPointer = inodeBlock[offset].doubleIndirectPointer;
    return inodeToReturn;
}


/**************************************************
 **************************************************/
void printSuperBlock(){
    printf("Drive Size: %u\n", sb.size);
    printf("Superblock number: %u\n", sb.startOfSuperBlock);
    printf("Inode Bitmap Block: %u\n", sb.startOfInodeBitmap);
    printf("Data Bitmap Block: %u\n", sb.startOfDataBitmap);
    printf("Start of Inode Table: %u\n", sb.startOfInodeTable);
    printf("Start of Data Region: %u\n", sb.startOfDataRegion);
    printf("Size of Inode: %u\n", sb.sizeOfInode);
    printf("Root Inode Inumber: %u\n", sb.rootInumber);
    printf("Number of Available Data Blocks: %u\n", sb.availableDataBlocks);
}

/**************************************************
 **************************************************/

void printInode(inode in){
    printf("inumber: %u\n", in.inumber);
    printf("inode type: %d\n", in.fileType);
    printf("Inode length : %d\n", in.length);
    printf("Inode number of Blocks : %d\n", in.numberOfBlocks);
    struct tm tm = *localtime(&in.created);
    printf("Created: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    tm = *localtime(&in.accessed);
    printf("Last Accessed: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    tm = *localtime(&in.modified);
    printf("Last Modified: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int i;
    for(i = 0; i < NUMDIRECTPOINTERS; i++){
        printf("Direct pointer[%d]: %u\n", i, in.directPointer[i]);
    }
    printf("Indirect pointer: %u\n", in.indirectPointer);
    printf("Double indirect pointer: %u\n", in.doubleIndirectPointer);
}

/**************************************************
 **************************************************/
int addFileToTable(inode * inodeOfFile, unsigned int offset){
    // allocate a FTENTRY
    openFileTableEntry * FTE = malloc(sizeof(openFileTableEntry));
    // initialize ftentry with occupied, block, offset 0, inode
    
    FTE->occupied = 1;
    FTE->Inode = inodeOfFile;
    FTE->offset = offset;
    //FTE->blockNumber = blockNumber;
    // find vacant slot in open file table
    int i;
    if(OFT.numberOfEntries == MAXNUMOPENFILES){
        //NO VACANT SLOTS
        printf("Error: no vacant slots in file table \n");
        return(-1);
    }
    for(i = 0; i < MAXNUMOPENFILES; i++){
        if(OFT.openFiles[i].occupied == 0){
            // OFT.openFiles[i] = *FTE;
            OFT.openFiles[i].occupied = 1;
            OFT.openFiles[i].Inode = inodeOfFile;
            OFT.openFiles[i].offset = offset;
            OFT.numberOfEntries = OFT.numberOfEntries + 1;
            return(i);
        }
    }
    return(-1);
}


/**************************************************
 **************************************************/
int removeFileFromTable(int fd){
    if (OFT.openFiles[fd].occupied != 0){
        OFT.openFiles[fd].occupied = 0;
        OFT.numberOfEntries = OFT.numberOfEntries - 1;
        free(OFT.openFiles[fd].Inode);
        return(0);
    }
    return(-1);
}
/**************************************************
 **************************************************/

//CURRENTLY CREATES FILE IN ROOT DIR
int lCreateFile(char* fileName){
    if(searchDirByFileName(fileName)){
        printf("file named %s already exists\n", fileName);
        return -1;
    }
    
    unsigned int inumber;
    //FIND FREE INODE
    inumber = findFreeInodeFromBitmap();
    if(inumber == -1){
        printf("Unable to find free inode\n");
        return(-1);
    }
    //CREATE AND INIT INODE FOR FILE
    /*inode inodeOfFile;
     inodeOfFile.inumber = inumber;
     inodeOfFile.fileType = 1;
     inodeOfFile.numberOfBlocks = 1;
     inodeOfFile.created = time(NULL);
     inodeOfFile.modified = time(NULL);
     */
    inode *inodeOfFile = getInodeFromNumber(inumber);
    inodeOfFile->fileType = 1;
    inodeOfFile->numberOfBlocks = 1;
    inodeOfFile->created = time(NULL);
    inodeOfFile->modified = time(NULL);
    //MARK INODE BITMAP AS ALLOCATED
    markBitmapAllocated(inumber, INODEBITMAP);
    //FIND FREE DATABLOCK
    int blockNumber = findFreeDataBlockFromBitmap();
    //ADD DATABLOCK POINTER TO INODE
    inodeOfFile->directPointer[0] = blockNumber + sb.startOfDataRegion;
    //MARK DATA BITMAP AS ALLOCATED
    markBitmapAllocated(blockNumber, DATABITMAP);
    //WRITE INODE TO INODE REGION
    writeInodeToDrive(inodeOfFile);
    //OPEN TARGET DIRECTORY (ROOT*)
    getInodeFromNumber(currentWorkingDirectory);
    //CREATE AND INIT DIRECTORY ENTRY
    lDirectoryEntry ldirent;
    ldirent.namelen = strlen(fileName);
    int i;
    for(i = 0; i < strlen(fileName); i++){
        ldirent.fileName[i] = fileName[i];
    }
    ldirent.inumber = inumber;
    //TODO: ADD DIRECTORY ENTRY TO TARGET DIRECTORY
    writeDirectoryEntryToDrive(ldirent, currentWorkingDirectory);
    return(0);
}
//TODO: Method to Add directory entry to block
/**************************************************
 **************************************************/
void printDirectoryContents(int inumber, int parentInumber, int level){
    if(level == 0){
        printf("/\n");
    }
    inode * Inode = getInodeFromNumber(inumber);
    Inode->accessed = time(NULL);
    int i;
    //GO TO FIRST BLOCK
    for(i = 0; i< Inode->numberOfBlocks; i++){
        lDirectoryEntry ldirent[DIRENTSPERBLOCK];
        if (fseek(drive, Inode->directPointer[i] * BLOCKSIZE, SEEK_SET)!= 0){
            printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
            
        }
        fread(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
        int j;
        
        for(j = 0; j < DIRENTSPERBLOCK; j++){
            if(ldirent[j].namelen > 0){
                char string[ldirent[j].namelen+1];
                for(i = 0; i < ldirent[j].namelen; i++){
                    string[i] = ldirent[j].fileName[i];
                }
                
                string[ldirent[j].namelen] = '\0';
                int k;
                for(k = 0; k < level+1; k++){
                    printf("|-- ");
                }
                printf("%s\n", string);
                inode * childInode = getInodeFromNumber(ldirent[j].inumber);
                if(ldirent[j].inumber != inumber&&ldirent[j].inumber !=parentInumber && childInode->fileType == 0){
                    printDirectoryContents(ldirent[j].inumber, inumber, level + 1);
                }
            }
        }
    }
}

//OPEN LOGIC
//FOR WRITING
//IF FILE EXISTS
//1. DELETE IT
//DELETE FILE
//CREATE NEW FILE
//2. APPEND TO IT
//OPEN FILE
//SET OFFSET AND BLOCKNUMBER IN TABLE
//IF NO FILE EXISTS
//1. CREATE IT
//FOR READING

/**************************************************
 **************************************************/
int lOpenFile(char* fileName, char * flag){
    //r for read only (begins at start of file)
    //w for write and erase
    //a for append to end of file
    int mode;
    if(strcmp(flag, "r") == 0){
        mode = 1;
    }else if (strcmp(flag, "w") == 0){
        mode = 2;
    }else if (strcmp(flag, "a") == 0){
        mode = 3;
    }else {
        printf("Error: %s is not a valid flag, use 'r', 'w', 'a'\n", flag);
        return(-1);
    }
    inode* Inode = getInodeFromNumber(searchDirByFileName(fileName));
    if(Inode->fileType == -1){
        printf("could not find file : %s\n", fileName);
        return -1;
    }
    
    return(addFileToTable(Inode, 0));
}

/**************************************************
 **************************************************/
int lCloseFile(int fd){
    writeInodeToDrive(OFT.openFiles[fd].Inode);
    removeFileFromTable(fd);
    return 0;
}

/**************************************************
 **************************************************/
void printOpenFileTable(){
    int i;
    if(OFT.numberOfEntries == 0){
        printf("FILE TABLE IS EMPTY\n");
        
    }
    for(i = 0; i < OFT.numberOfEntries || i < MAXNUMOPENFILES; i++){
        if(OFT.openFiles[i].occupied == 1){
            printf("%d entries : OFT[%d] is occupied by inumber %d, next read at offset:%d\n",OFT.numberOfEntries, i, OFT.openFiles[i].Inode->inumber,  OFT.openFiles[i].offset);
        }
    }
}

//makeNewDirectoryBlock
/**************************************************
 **************************************************/
unsigned int makeEmptyDirectoryBlock(){
    //1. FIND EMPTY DATABLOCK
    unsigned int dataBlock = findFreeDataBlockFromBitmap();
    
    markBitmapAllocated(dataBlock, DATABITMAP);
    //2. CREATE ARRAY OF DIRECTORY ENTRIES
    lDirectoryEntry ldirent[DIRENTSPERBLOCK];
    int i;
    for(i = 0; i < DIRENTSPERBLOCK; i++){
        ldirent[i].inumber = 0;
        ldirent[i].namelen = 0;
    }
    //3. GO TO LOCATION IN DRIVE
    if (fseek(drive, (dataBlock + sb.startOfDataRegion)* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(0);
    }
    //4. WRITE ARRAY TO DIRECTORY
    fwrite(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK , drive);
    //
    return(dataBlock);
}

//MAKE NEW DIRECTORY
/**************************************************
 **************************************************/
int lmkDir(char* dirName){
    if(searchDirByFileName(dirName)){
        printf("directory named %s already exists\n", dirName);
        return -1;
    }
    //1 Find free inode
    int inumber = findFreeInodeFromBitmap();
    inode *newInode = getInodeFromNumber(inumber);
    newInode->fileType = 0;
    //2. Mark Inode allocated
    markBitmapAllocated(inumber, INODEBITMAP);
    //3. Make Empty Directory Block
    int dataBlockNumber = makeEmptyDirectoryBlock();
    //4. Set first direct pointer to data block
    newInode->directPointer[0] = dataBlockNumber + sb.startOfDataRegion;
    newInode->numberOfBlocks = 1;
    newInode->length = 0;
    writeInodeToDrive(newInode);
    //5. Set . entry to inumber and write to block
    lDirectoryEntry ldirent;
    ldirent.inumber = inumber;
    ldirent.namelen = 1;
    ldirent.fileName[0] = '.';
    writeDirectoryEntryToDrive(ldirent,inumber);
    //6. Set .. entry to current working directory and write to block
    ldirent.inumber = currentWorkingDirectory;
    ldirent.namelen = 2;
    ldirent.fileName[1] = '.';
    writeDirectoryEntryToDrive(ldirent, inumber);
    
    //7. Write entry in CWD
    ldirent.inumber = inumber;
    ldirent.namelen = strlen(dirName);
    int i;
    for(i = 0; i < strlen(dirName); i++){
        ldirent.fileName[i] = dirName[i];
    }
    
    writeDirectoryEntryToDrive(ldirent, currentWorkingDirectory);
    return 0;
}


/*****************************************************
 METHOD: searchDirByFileName
 INPUT: inumber of directory:int , filename : char *
 OUTPUT: inumber of file/directory if found, 0 else
 *****************************************************/
int searchDirByFileName(char* fileName){
    inode *currentInode = getInodeFromNumber(currentWorkingDirectory);
    lDirectoryEntry ldirent[DIRENTSPERBLOCK];
    if (fseek(drive, (currentInode->directPointer[0])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    
    fread(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
    int i;
    free(currentInode);
    for(i = 0; i < DIRENTSPERBLOCK; i++){
        if(ldirent[i].namelen > 0){
            char string[ldirent[i].namelen]; //ldirent[i].namelen, i);
            int j;
            for(j = 0; j < ldirent[i].namelen; j++){
                string[j] = ldirent[i].fileName[j];
            }
            string[j] = '\0';
            
            if(strcmp(fileName, string)== 0){
                return ldirent[i].inumber;
            }
        }
    }
    return 0;
}

/**************************************************
 **************************************************/
int removeDirentByFileName(char* fileName){
    inode *currentInode = getInodeFromNumber(currentWorkingDirectory);
    lDirectoryEntry ldirent[DIRENTSPERBLOCK];
    if (fseek(drive, (currentInode->directPointer[0])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fread(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
    int i;
    free(currentInode);
    for(i = 0; i < DIRENTSPERBLOCK; i++){
        if(ldirent[i].namelen > 0){
            char string[ldirent[i].namelen];
            int j;
            for(j = 0; j < ldirent[i].namelen; j++){
                string[j] = ldirent[i].fileName[j];
            }
            string[j] = '\0';
            if(strcmp(fileName, string)== 0){
                ldirent[i].namelen = 0;
                ldirent[i].fileName[0] = '\0';
            }
        }
    }
    if (fseek(drive, (currentInode->directPointer[0])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fwrite(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
    return 0;
}

//CHANGE CURRENT WORKING DIRECTORY
/**************************************************
 **************************************************/
int changeCurrentWorkingDirectory(char* dirName){
    //int numberOfNewDir = searchDirByFileName(dirName);
    inode* Inode = getInodeFromNumber(searchDirByFileName(dirName));
    if(Inode->fileType == 0){
        currentWorkingDirectory = Inode->inumber;
        free(Inode);
        //printf("%s>:", dirName);
        currentDirectoryName = getDirNameByInumber(currentWorkingDirectory);
        return currentWorkingDirectory;
    }
    return -1;
}

/**************************************************
 **************************************************/
void printCurrentWorkingDirectory(){
    inode* Inode = getInodeFromNumber(currentWorkingDirectory);
    
    free(Inode);
    //if ROOT return "/"
    //else get parent inode
    //find inumber in parent directory
    //return name
    //rintf("Inumber of CurrentWorkingDirectory: %d\n", currentWorkingDirectory);
    printf("%s>:", currentDirectoryName);
}



//WRITE TO FILE
/**************************************************
 **************************************************/
int lWriteFile(int fd, char c){
    unsigned int offset = OFT.openFiles[fd].offset;
    unsigned int augmentedOffset = OFT.openFiles[fd].offset % BLOCKSIZE;
    unsigned int blockNumber = OFT.openFiles[fd].offset / BLOCKSIZE;
    char buffer[BLOCKSIZE];
    if(OFT.openFiles[fd].Inode->directPointer[blockNumber] == 0){
        
        OFT.openFiles[fd].Inode->directPointer[blockNumber] = findFreeDataBlockFromBitmap();
        markBitmapAllocated(OFT.openFiles[fd].Inode->directPointer[blockNumber], DATABITMAP);
        OFT.openFiles[fd].Inode->directPointer[blockNumber] =  OFT.openFiles[fd].Inode->directPointer[blockNumber] + sb.startOfDataRegion;
    }
    if (fseek(drive, (OFT.openFiles[fd].Inode->directPointer[blockNumber])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fread(&buffer, sizeof(char), BLOCKSIZE, drive);
    buffer[augmentedOffset] = c;
    if (fseek(drive, (OFT.openFiles[fd].Inode->directPointer[blockNumber])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fwrite(&buffer, sizeof(char), BLOCKSIZE, drive);
    
    if(offset + 1 % BLOCKSIZE == 0){
        //allocate a new block
        unsigned int newBlockPtr = blockNumber + 1;
        unsigned int newBlock = findFreeDataBlockFromBitmap();
        markBitmapAllocated(newBlock, DATABITMAP);
        //update inode in fd
        OFT.openFiles[fd].Inode->numberOfBlocks = OFT.openFiles[fd].Inode->numberOfBlocks + 1;
        OFT.openFiles[fd].Inode->directPointer[newBlockPtr] = newBlock + sb.startOfDataRegion;
        
        
    }
    //update offset in fd
    OFT.openFiles[fd].offset = offset + 1;
    OFT.openFiles[fd].Inode->length = OFT.openFiles[fd].Inode->length + 1;
    //save inode
    // writeInodeToDrive(OFT.openFiles[fd].Inode);
    return 0;
}

//READ FROM FILE
/**************************************************
 **************************************************/
char lReadFile(int fd){
    char retc;
    inode * Inode = OFT.openFiles[fd].Inode;
    int fileSize = Inode->length;
    int offset = OFT.openFiles[fd].offset % BLOCKSIZE;
    int currentBlock = OFT.openFiles[fd].offset / BLOCKSIZE;
    char bytes[BLOCKSIZE];
    if (OFT.openFiles[fd].offset >= fileSize){
        return -1;
    }
    //GO TO BLOCK
    if (fseek(drive, (Inode->directPointer[currentBlock])* BLOCKSIZE, SEEK_SET)!= 0){
        printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
        return(-1);
    }
    fread(&bytes, sizeof(char), BLOCKSIZE, drive);
    retc = bytes[offset];
    OFT.openFiles[fd].offset += 1;
    return retc;
}

void makeDrive(char * fileName){
    FILE * file = fopen(fileName, "w+b");
    int i;
    for (i = 0 ; i < 2000000; i++){
        fputc(0, file);
    }
    fclose(file);
}

//MAKE DIR DATA BLOCK
/*****************************************************
 METHOD: printAvali
 INPUT:
 *****************************************************/
void printAvailableDataBlocks(){
    printf("Free blocks: %d\nFree bytes: %d\n\n", sb.availableDataBlocks, sb.availableDataBlocks*BLOCKSIZE);
}

/**************************************************
 **************************************************/
void ls(){
    printDirectoryContents(1,1,0);
}

/**************************************************
 **************************************************/
void lDeleteFile(char* fileToBeDeleted){
    //GET INUMBER OF FILE
    int fileInumber = searchDirByFileName(fileToBeDeleted);
    inode * InodeOfFileToBeDeleted = getInodeFromNumber(fileInumber);
    inode * InodeOfParentFile = getInodeFromNumber(currentWorkingDirectory);
    //GET INUMBER OF PARENT FILE
    int i;
    for(i = 0; i < NUMDIRECTPOINTERS; i++){
        if(InodeOfFileToBeDeleted->directPointer[i] != 0){
            deallocateBitmap(InodeOfFileToBeDeleted->directPointer[i]-sb.startOfDataRegion, DATABITMAP);
            InodeOfFileToBeDeleted->directPointer[i] = 0;
            InodeOfFileToBeDeleted->numberOfBlocks -= 1;
        }
    }
    //GO TO PARENT
    removeDirentByFileName(fileToBeDeleted);
    //FREE ASSOCIATED DATA BLOCKS FOR REUSE (ALSO IN BITMAP)
    deallocateBitmap(fileInumber, INODEBITMAP);
    //Go TO FILE to BE DELETED
    //SET DIRECTORY ENTRY TO UNOCCUPIED
    InodeOfFileToBeDeleted->fileType = -1;
    InodeOfFileToBeDeleted->length = 0;
    writeInodeToDrive(InodeOfFileToBeDeleted);
    InodeOfParentFile->length = InodeOfParentFile->length - 1;
    InodeOfFileToBeDeleted->numberOfBlocks = 0;
    writeInodeToDrive(InodeOfParentFile);
    
}
//TODO: method to print only contents of directory


//TODO: method to get filename by Inumber
char* getDirNameByInumber(int inumber){
    //if root
    char * retVal;
    retVal = NULL;
    if (inumber == 1){
        retVal = strdup("/");
        return retVal;
    }else{
        //get inode
        inode * inodeOfFile = getInodeFromNumber(inumber);
        //get parent inode
        int parentInumber = searchDirByFileName("..");
        //get contents of parent directory
        inode * inodeOfParent = getInodeFromNumber(parentInumber);
        //find my inode in directory
        lDirectoryEntry ldirent[DIRENTSPERBLOCK];
        if (fseek(drive, (inodeOfParent->directPointer[0])* BLOCKSIZE, SEEK_SET)!= 0){
            printf("Error finding location blocksize %d in file\n", sb.startOfInodeTable * BLOCKSIZE);
            return retVal;
        }
        fread(&ldirent, sizeof(lDirectoryEntry), DIRENTSPERBLOCK, drive);
        int i;
        for(i = 0; i < DIRENTSPERBLOCK; i++){
            if(ldirent[i].namelen > 0){
                if(ldirent[i].inumber == inumber){
                    char string[ldirent[i].namelen];
                    int j;
                    for(j = 0; j < ldirent[i].namelen; j++){
                        string[j] = ldirent[i].fileName[j];
                    }
                    string[j] = '\0';
                    retVal = strdup(string);
                    return retVal;
                }
            }
            //return string of directory
        }
    }
    return retVal;
}
