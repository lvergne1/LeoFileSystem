//
//  shellLfs.c
//  
//
//  Created by Leo Vergnetti on 12/11/18.
//
#include <stdio.h>
#include "lfsLog.h"
#include <stdlib.h>
#include "shellLfs.h"

#define BUFFERSIZE 100
int shellLfsRunning;
//char * currentDirectory;
int driveMounted;
void printCommands(){
    printf("WELCOME to the Leo File System shell(ish)\n");
    printf("******************************************\n");
    printf("You may interact with the FS via the following commands\n");
    printf("1.  exit: Exits the shell\n");
    printf("2.  format: requests a file name to format for the fileSystem (WARNING: will erase current contents of file!\n");
    printf("3.  make: requests a file name to format and make a 2mb version of LFS (WARNING: will erase contents of file!)\n");
    printf("4.  mount:  mounts the specified drive file\n");
    printf("5.  contents: prints the contents of the file system in a tree\n");
    printf("6.  chdir: change to a new directory\n");
    printf("7.  mkdir: make a directory in current directory\n");
    printf("8.  unmount: unmount current directory, saving state\n");
    printf("9.  help: review commands\n");
    printf("10. import: imports a user-specified text file from directory from which file system was launched\n");
    printf("11. cat: displays contents of a text file to the screen\n");
    printf("12. more: displays the contents of a text file to the screen one screen at a time\n");
    printf("13. text: makes a new text file \n");
    printf("14. freespace: Displays the amount of current free space on drive\n");
    printf("15. rm: deletes a file from the file system\n");
    printf("******************************************\n");
    printf("\n\n");
    
}
void executeCommand(char * userInput){
    //remove newline char
    userInput[strlen(userInput)-1] = '\0';
    if(strcmp(userInput, "exit")== 0){
        shellLfsRunning = 0;
        if(driveMounted == 1){
            unmountLFS();
            driveMounted = 0;
        }
        printf("exiting File System\n");
    }
    
    else if(strcmp(userInput, "make")== 0){
        printf("Enter the name of the drive you want to create\n");
        fgets(userInput, BUFFERSIZE, stdin);
        int i;
        userInput[strlen(userInput)-1] = '\0';
        makeDrive(userInput);
        printf("'%s' created for 2mb LFS\n", userInput);
        printf("use 'format' to install LFS\n");
    }
    
    else if(strcmp(userInput, "format") == 0){
        printf("Enter the drive file to install file system (or use 'make' command to create one)\n");
        fgets(userInput, BUFFERSIZE, stdin);
        int i;
        userInput[strlen(userInput)-1] = '\0';
        formatDrive(userInput);
        printf("drive %s successfully installed\n", userInput);
    }
    
    else if(strcmp(userInput, "mount") == 0){
        if(driveMounted == 0){
            printf("Enter the LFS drive you wish to mount\n");
            fgets(userInput, BUFFERSIZE, stdin);
            int i;
            userInput[strlen(userInput)-1] = '\0';
            if(mountLFS(userInput)==0){
                driveMounted = 1;
                printf("%s successfully mounted\n", userInput);
            }
        }else{
            printf("A drive is already mounted\n");
        }
    }
    else if(strcmp(userInput, "unmount") == 0){
        if(driveMounted == 1){
            unmountLFS();
            driveMounted = 0;
        }else{
            printf("Must mount a drive first\n");
        }
    }
    else if(strcmp(userInput, "rm")== 0){
        if(driveMounted== 0){
            printf("Must mount a drive first\n");
        }else{
            printf("Enter the text file you wish to delete\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            lDeleteFile(userInput);
        }
    }
    else if(strcmp(userInput, "contents") == 0){
        if(driveMounted == 0){
            printf("No drive currently mounted, first mount a drive using 'mount'\n");
        }else{
            printf("\nDirectory Tree\n");
            ls();
            printf("\n");
        }
    }
    
    else if(strcmp(userInput, "freespace")== 0){
        if(driveMounted == 1){
            printAvailableDataBlocks();
        }else{
            printf("Must mount a drive first\n");
        }
    }
    
    else if(strcmp(userInput, "mkdir") == 0){
        if(driveMounted == 1){
            printf("Enter the name of the directory you wish to create\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            lmkDir(userInput);
            printf("Directory created\n");
        }else{
            printf("Must mount a drive first\n");
        }
    }
    
    else if(strcmp(userInput, "chdir") == 0){
        if(driveMounted == 1){
            printf("Enter the name of the directory to enter\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            changeCurrentWorkingDirectory(userInput);
        }else{
            printf("Must mount a drive first\n");
        }
    }
    
    else if(strcmp(userInput, "cat") == 0){
        if(driveMounted == 1){
            printf("Enter the name of the file to view\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            int fd = lOpenFile(userInput, "r");
            printf("Contents of %s:\n\n", userInput);
            char c;
            int i;
            while((c = lReadFile(fd))!= -1){
                printf("%c", c);
            }
            lCloseFile(fd);
            printf("\n\n");
        }else{
            printf("Must mount a drive first\n");
        }
    }
    
    else if(strcmp(userInput, "more") == 0){
        if(driveMounted ==  1){
            printf("Enter the name of the file to view\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            int fd = lOpenFile(userInput, "r");
            printf("Contents of %s:\n\n", userInput);
            char c;
            int i = 0;
            while((c = lReadFile(fd))!= -1){
                if( i == 1600){
                    printf("press enter for next page\n");
                    fgets(userInput, BUFFERSIZE, stdin);
                }
                printf("%c", c);
                i++;
            }
            lCloseFile(fd);
            printf("\n\n");
        }else{
            printf("Must mount a drive first\n");
        }
    }
    else if(strcmp(userInput, "help") == 0){
        printCommands();
    }
    
    else if(strcmp(userInput, "text")==0){
        if(driveMounted == 1){
            printf("Enter the name of the text file to create\n");
            fgets(userInput, BUFFERSIZE, stdin);
            userInput[strlen(userInput)-1] = '\0';
            char fileInput[BUFFERSIZE];
            lCreateFile(userInput);
            int fd = lOpenFile(userInput, "w");
            printf("Enter contents of %s:\n", userInput);
            fgets(fileInput, BUFFERSIZE, stdin);
            int i;
            for(i = 0; i < strlen(fileInput); i++){
                lWriteFile(fd, fileInput[i]);
            }
            lCloseFile(fd);
        }else{
            printf("Must mount a drive first\n");
        }
    }
    
    
    else if(strcmp(userInput, "import") == 0){
        if(driveMounted == 1){
            char sourceName[BUFFERSIZE];
            char destName[BUFFERSIZE];
            printf("Enter text file to import into system (must be directory file system was launched from)\n");
            fgets(sourceName, BUFFERSIZE ,stdin);
            sourceName[strlen(sourceName) -1] = '\0';
            printf("Enter the desired name of new text file\n");
            fgets(destName, BUFFERSIZE ,stdin);
            destName[strlen(destName) -1] = '\0';
            FILE *sourceFile = fopen(sourceName, "r");
            if(sourceFile == NULL){
                printf("Unable to locate '%s' file! (must be in directory file system was launched from\n", sourceName);
            }else if (lCreateFile(destName) < 0){
                printf("Error creating file %s\n", destName);
            }else{
                int fd = lOpenFile(destName, "w");
                while(1){
                    char c = fgetc(sourceFile);
                    if (feof(sourceFile)){
                        break;
                    }else{
                        lWriteFile(fd, c);
                    }
                }
                lCloseFile(fd);
                fclose(sourceFile);
            }
        }else{
            printf("Must mount a drive first\n");
        }
    }
}

int main(int argc, char** argv){
    FILE * file;
    
    shellLfsRunning = 1;
    driveMounted = 0;
    //get user command
    //parse user command
    //execute user command
    printCommands();
    if(argc > 1){
        file = fopen(argv[1], "r");
    }
    
    while(shellLfsRunning){
        char userInput[BUFFERSIZE];
        if (driveMounted == 1){
            printCurrentWorkingDirectory();
        } else{
            printf("Enter a command. (No drive currently mounted)\n");
        }
        /*
        if(argc > 0){
            fgets(userInput, BUFFERSIZE, file);
        }else{
         */
            fgets(userInput, BUFFERSIZE, stdin);
        //}
        executeCommand(userInput);
    }
    
}
