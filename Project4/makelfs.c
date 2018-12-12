//
//  makelfs.c
//  
//
//  Created by Leo Vergnetti on 12/9/18.
//

#include "makelfs.h"
#include "lfsLog.h"
#include <stdio.h>
#include <stdlib.h>

/*
 *LAYOUT OF DISK
 *TOTAL SECTORS 3906
 *NUMBER OF INODES 1940
 *TOTAL DATA BLOCKS 3418
 *SUPERBLOCK = BLOCK 0
 *INODE BITMAP = BLOCK 1
 *DATA REGION BITMAP = BLOCK 2
 *INODE REGION = BLOCK 3 - 487
 *DATA REGION = BLOCK 488 - 3418
 
 * MAX SIZE W DIRECT POINTERS : 16*512 = 8,192 bytes
 * MAX SIZE WITH INDIRECT POINTER : 512 bps * 128 = 65,536 + 8,192 = 73,728 bytes
 * MAX SIZE WITH DOUBLE INDIRECT : 8,388,608 bytes (larger than 2MB disk)
 */


/***************************************************
 METHOD: makeLFS
 INPUT: file : FILE*
 OUTPUT: 0 on success, -1 else
 DESCRIPTION: makeLFS creates an LFS on the specified
 file pointer.  MUST BE 2MB.
 ***************************************************/

int main(int argc, char** argv){
    
    
   formatDrive("drive2mb");
    makeLFS("drive2mb");
    
}

