//
//  lfsFinalTest2.c
//  
//
//  Created by Leo Vergnetti on 12/10/18.
//

#include "lfsFinalTest2.h"
#include "lfsLog.h"
#include <stdio.h>

int main(){
    mountLFS("drive2mb");
    printAvailableDataBlocks();
    int fd = lOpenFile("textFile1", "r");
    char rval;
    printf("printing contents of text_file_1\n");
    while((rval = lReadFile(fd))!= -1){
        printf("%c", rval);
    }
    ls();
    printAvailableDataBlocks();
    changeCurrentWorkingDirectory("dir1");
    printf("printing contents of text_file_2\n");
    fd = lOpenFile("textFile2", "r");
    printOpenFileTable();
    while((rval = lReadFile(fd))!= -1){
        printf("%c", rval);
    }
    printAvailableDataBlocks();
    lDeleteFile("textFile2");
    printAvailableDataBlocks();
    ls();
    
    
}
