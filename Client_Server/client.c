#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

char *getLine(int fd)
{
	int i = 0;
	char *buffer = (char *)malloc(sizeof(char) * 100);
	for (; read(fd, &buffer[i], 1) != 0 && buffer[i] != '\n'; i++)
		;
	if (i == 0)
	{
		free(buffer);
		return NULL;
	}
	buffer[i] = '\0';
	buffer = (char *)realloc(buffer, i + 1);
	return buffer;
}

void handler(int signum){    
    char* pid = (char*)malloc(sizeof(char)*6);
    pid_t clientPID = getpid();
    char* fileName = (char*)malloc(sizeof(char)*100);
    strcpy(fileName, "toClient");
    sprintf(pid, "%d", clientPID);
    pid = (char*)realloc(pid,strlen(pid));  
    strcat(fileName, pid);
    strcat(fileName, ".txt");
    fileName = (char*)realloc(fileName, strlen(fileName)+1);
    
    int toClientFile = open(fileName, O_RDONLY);
    if(toClientFile < 0){
        printf("ERROR: toClient is not reachable\n");
        exit(-1);
    }
    char* solution = getLine(toClientFile);
    printf("Solution= %s\n", solution);    
    char *argument_list[] = {"rm", fileName, NULL};
	execvp("rm", argument_list);
    
    exit(0);
}

int main(int argc, char* argv[]){
    signal(SIGINT, handler);    

    int toServerFile;
    int sleepTime;
    pid_t clientPid = getpid();
    
    // Try 10 times (withwait for several seconds) to create toServer.txt if exists
    for(int i = 0; i< 10; i++){
        toServerFile = open("toServer.txt", O_CREAT | O_RDONLY  | O_WRONLY, 0666);
        if(toServerFile < 0){
            // sleepTime = rand()%6;
            // sleep(sleepTime);
        }
        else
            break;
    }
    if(toServerFile<0){
        printf("ERROR: Cannot connect to server\n");
        exit(-1);
    }    
    //Write to toServer.txt     
    char* args = (char*)malloc(sizeof(char)*100);
    char* pid = (char*)malloc(sizeof(char)*100);
    sprintf(pid, "%d", clientPid);
    strcpy(args, pid);
    strcat(args, " ");
    strcat(args, argv[1]);
    strcat(args, " ");
    strcat(args, argv[2]);
    strcat(args, " ");
    strcat(args, argv[3]);
    args = (char*)realloc(args, strlen(args)+1);
    write(toServerFile, args, strlen(args));   
    
    // Send signal to server    
    pid_t serverPID = atoi(argv[4]);
    kill(serverPID,SIGINT);//(PID, SIGNAL=1)

    //wait for server to compute (max of 30 seconds)    
    sleep(30);        
    printf("Client closed because no response was recieved from the server for 30 seconds\n");
    exit(0);
}
