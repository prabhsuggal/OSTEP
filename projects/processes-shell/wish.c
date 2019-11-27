#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>
#include <fcntl.h> 
#include<ctype.h>

char EXIT[]="exit\n";
char error_message[30] = "An error has occurred\n";

#define NUM_OF_ARGS 20

char **path, **args, *filePath;
int outFp = 1, numArgs;

// Removing trailing/leading spaces from a string 
char* RemoveSpaces(char* str){
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// Check If the output will be redirected toa file
void checkRedirect(char* str){
    char *arg1, *arg2;
    int i = 0;

    if(strlen(RemoveSpaces(str)) == 0)
        exit(1);
    strsep(&str,">");
    if(str != NULL){
        while((arg1 = strsep(&str," ")) != NULL){
            arg2 = RemoveSpaces(arg1);
            if((i > 0) && (strlen(arg2) > 0)){
                write(STDERR_FILENO, error_message, strlen(error_message)); 
                exit(1);
            }
            if(strlen(arg2)>0){
                outFp = open(arg2,O_RDWR | O_CREAT, 0666);
                if(outFp == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message)); 
                    exit(1);
                }
                i++;
            }
        }
        if(i!=1){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            exit(1);
        }
    }
    return;
}

//Separate all arguments and store in an array
void PrepArgs(char* buffer){
    int i=0;
    char *arg1, *arg2;
    
    if(strlen(RemoveSpaces(buffer)) == 0)
        return;

    buffer = strsep(&buffer,">");

    while((arg1 = strsep(&buffer," ")) != NULL){
        arg2 = RemoveSpaces(arg1); 
        if(strlen(arg2)>0){
            args[i++] = strdup(arg2);
        }
    }

    
    free(arg1);
    args[i] = NULL;
    numArgs = i;

    return;
}

// function for built-in command "path"
void changePath(){
    for(int x=1; x < numArgs; x++){
        path[x-1] = strdup(args[x]);
    }
    path[numArgs-1] = NULL;
    return;
}

// function for built-in command "cd"
void changeDir(){
    if(numArgs != 2){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return;
    }
    if(chdir(args[1])==-1){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        return;
    };
    return;
}

// Running the command in the child thread
void operate(){

    int i=0;
    if(numArgs == 0){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
    }
    while(path[i]!=NULL){
        filePath = strdup(path[i]);
        strcat(filePath,"/");
        strcat(filePath,args[0]);

        switch(access(filePath, X_OK)){
            case EACCES:{
                            printf("%s: Access Denied\n", args[0]);
                            break;
                        }
            case ENOENT:{
                            break;
                        }
            case EROFS:{
                            printf("%s: Can't write into this file\n", filePath);break;
                       }
            case 0:{
                       args[0] = filePath;
                       break;
                   }
            default:{
                        printf("%s: Error while running this command\n",filePath);break;
                    }
        }
        i++;
    }

    dup2(outFp, 1);
    if(execv(args[0],args)==-1){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
    }
    
    close(outFp);

    i=0;
    while(args[i]!=NULL)
        free(args[i++]);
    free(args);

    exit(0);

}

//bring the whole input string and check for parallel commands, and getting rid
//of newline character included in the getline(). fork() is called to break it
//into parent and child thread
void RunCmds(char *buffer){
    char *arg, *str;
    
   buffer[strlen(buffer)-1] = '\0'; 

    while((arg = strsep(&buffer,"&")) != NULL){
        str = strdup(arg);
        PrepArgs(arg);
        if(args[0]!=NULL){
            if(strcmp(args[0],"path") == 0){
                changePath();
                continue;
            }
            if(strcmp(args[0],"cd") == 0){
                changeDir();
                continue;
            }
        }
        int rc = fork();
        if(rc<0){
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            exit(1);
        }
        else if(rc > 0){
            continue;
        }
        checkRedirect(str);
        operate();
    }
    while(wait(NULL)>0);

    free(str);
    free(arg);

}

//in case only ./wish is used at the command line
void interactive(){

    char *buffer;
    size_t bufsize = 32,len;

    buffer = (char*)malloc(bufsize * sizeof(char));
    while(1){
        printf("wish> ");
        len = getline(&buffer, &bufsize, stdin);
        
        if(strcmp(buffer,"\n")==0)
            continue;
        if(strcmp(buffer,EXIT) == 0){
            exit(0);
        }
        RunCmds(buffer);
    }

    free(buffer);
    return;
}

//when a command file is used as arguments to ./wish
void batch(char* file){
    FILE *fp = fopen(file,"r");
    if(!fp){
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
    }

    size_t bufsize = 32;
    int len;
    char* buffer = (char*)malloc(sizeof(char)*bufsize);

    len = getline(&buffer, &bufsize, fp);
    while(len > 0){
        if(strcmp(buffer,EXIT) == 0){
            exit(0);
        }
        RunCmds(buffer);        

        len = getline(&buffer,&bufsize,fp);

    }
    
    free(buffer);
    return;

}

int main(int argc, char* argv[]){

    path = (char**)malloc(NUM_OF_ARGS*sizeof(char*));
    args = (char**)malloc(NUM_OF_ARGS*sizeof(char*));
    path[0] = strdup("/bin");
    path[1] = NULL;

    if(argc == 1){
        interactive();
    }
    else if(argc == 2)
        batch(argv[1]);
    else{
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
    }
    
    return 0;
}
