//
//  lfsFinalTest.c
//  
//
//  Created by Leo Vergnetti on 12/10/18.
//

#include "lfsFinalTest.h"
#include "lfsLog.h"
#include <stdio.h>

int main(){
    char buffer[512];
    int i;
    //MAKE A NEW LFS
    printf("Creating a new drive from 'drive2mb' file\n");
    makeDrive("drive2mb");
    printf("attaching fs\n");
    formatDrive("drive2mb");
    printf("mounting drive2mb\n");
    mountLFS("drive2mb");
    printf("mount complete\n");
    printAvailableDataBlocks();
    printf("making directory\n");
    lmkDir("dir1");
    printf("directory structure:\n");
    ls();
    printAvailableDataBlocks();
    printf("preparing to create 'file1' in root directory\n");
    lCreateFile("textFile1");
    printAvailableDataBlocks();
    printf("Type some text to populate textFile1\n");
    fgets(buffer, 512, stdin);
    printf("getting file descriptor\n");
    int fd = lOpenFile("textFile1", "w");
    printf("preparing to write to file\n");
    printAvailableDataBlocks();
    for(i = 0; i < strlen(buffer); i++){
        lWriteFile(fd, buffer[i]);
    }
    lCloseFile(fd);
    printAvailableDataBlocks();
    printf("file1 written: moving to dir1\n");
    changeCurrentWorkingDirectory("dir1");
    printf("creating text_file_2\n");
    lCreateFile("textFile2");
    int fd2 = lOpenFile("textFile2", "w");
    printAvailableDataBlocks();
    FILE * file = fopen("textfile.txt", "r");
    
    while(1){
       char c = fgetc(file);
        if (feof(file)){
            break;
        }else{
            lWriteFile(fd2, c);
        }
    }

    fclose(file);
    lCloseFile(fd2);
    printAvailableDataBlocks();
    ls(); //Print Directory Tree
    unmountLFS(); 
}
