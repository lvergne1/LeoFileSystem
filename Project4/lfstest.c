//
//  lfstest.c
//  
//
//  Created by Leo Vergnetti on 12/9/18.
//

#include "lfstest.h"
#include "lfsLog.h"
#include <stdio.h>



int main(int c, char** argv){

    char *  string = "1a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,2.a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, 3.a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, 4.a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, 5a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, 6a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,7.a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,8.a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,9a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,10a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,11a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,";
    int i;
    mountLFS("drive2mb");
    printDirectoryContents(1,1,0);
    lCreateFile("file1");
    lCreateFile("file2");
    printDirectoryContents(1,1,0);
    int fd = lOpenFile("file1", "r");
    int fd2 = lOpenFile("file2", "r");
    lCloseFile(fd2);
    int fd3 = lOpenFile("file1", "r");
    lCloseFile(fd3);
    lCloseFile(fd);
    lmkDir("Dir1");
    printDirectoryContents(1,1,0);
    changeCurrentWorkingDirectory("Dir1");
    lmkDir("Dir2");
    lCreateFile("file3");
    fd = lOpenFile("file3", "w");
    for(i = 0; i < strlen(string); i++){
        lWriteFile(fd, string[i]);
    }
    lCloseFile(fd);
    fd = lOpenFile("file3", "r");
    char rval;
    while((rval = lReadFile(fd))!= -1){
        printf("%c", rval);
    }
    printDirectoryContents(1,1,0);
    printAvailableDataBlocks();
    unmountLFS();
    return 0;
}

