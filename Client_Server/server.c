#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

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
    int toServerFile =  open("toServer.txt", O_RDONLY);
    if(toServerFile < 0){
        printf("ERROR: toServer.txt failed to open\n");
        return;
    }
    int calcPid = fork();
    if(calcPid != 0){ // Parent process returns to main              
        return;
    }else{
        char* toServerArgs = getLine(toServerFile);   
        char delim[] = " ";
        char *ptr = strtok(toServerArgs, delim);
        int clientPID = atoi(toServerArgs);
        int leftNum = atoi(toServerArgs);
        int operationNum = atoi(toServerArgs);
        int rightNum = atoi(toServerArgs);
        int i = 1;
        // Delete toServer.txt 
        int delPid = fork();
        if(delPid == 0){ //Child process
            char *argument_list[] = {"rm", "toServer.txt", NULL};
			exit(execvp("rm", argument_list));
        }
        // Split words by " "
        while(ptr != NULL){
            switch(i){
                case 1:
                clientPID = atoi(ptr);
                break;
                case 2:
                leftNum = atoi(ptr);
                break;
                case 3:
                operationNum = atoi(ptr);
                break;
                case 4:
                rightNum = atoi(ptr);
                break;
            }		
            ptr = strtok(NULL, delim);
            i++;
        }
        // Read args        
        printf("Args recieved: %d %d %d %d\n", clientPID, leftNum, operationNum, rightNum);
        //Filename creation toClient.txt
        char* toClientFileName = (char*)malloc(sizeof(char)*19); //toClient.txt = 12 chars 
        char* pid = (char*)malloc(sizeof(char)*6);
        strcpy(toClientFileName, "toClient");
        sprintf(pid, "%d", clientPID);    
        strcat(toClientFileName, pid);
        strcat(toClientFileName, ".txt");
        toClientFileName[strlen(toClientFileName)] = '\0';    
        printf("%s\n", toClientFileName);

        int toClientFile = open(toClientFileName, O_CREAT | O_RDONLY  | O_WRONLY, 0666);
        if(toClientFile < 0){
            printf("ERROR: toClient is not reachable\n");
            exit(-1);
        }
        // Calculate result 
        char* result = (char*)malloc(sizeof(char)*100);
        int res;
        char* resC = (char*)malloc(sizeof(char)*100);
        switch(operationNum){
            case 1:
                res = leftNum+rightNum;
                sprintf(resC, "%d", res); 
                resC = (char*)realloc(resC, strlen(resC));
                strcpy(result, resC);
                break;
            case 2:
                res = leftNum-rightNum;
                sprintf(resC, "%d", res); 
                resC = (char*)realloc(resC, strlen(resC));
                strcpy(result, resC); 
                break;
            case 3:
                res = leftNum*rightNum;
                sprintf(resC, "%d", res); 
                resC = (char*)realloc(resC, strlen(resC));
                strcpy(result, resC);
                break;
            case 4:
                if(rightNum != 0){
                    res = leftNum/rightNum;
                    sprintf(resC, "%d", res); 
                    resC = (char*)realloc(resC, strlen(resC));            
                    strcpy(result, resC);
                }                
                else
                    strcpy(result, "Can't divide by zero!");
                break;
            default:
                strcpy(result ,"Illegal operation!");
                break;

        }
        // Write to toClient file
        printf("res= %d\n", res);       
        if(res == 0) sleep(100);
        result = (char*)realloc(result, strlen(result));
        write(toClientFile, result, strlen(result));        
        kill(clientPID, SIGINT);

        exit(0);
    }     
}

void child_handler(int signum){
    while (waitpid(-1, NULL, WNOHANG) > 0) {}    
}


int main(int argc, char* argv[]){
    // Call handler function when there is a signal from client       
    signal(SIGINT, handler);
    signal(SIGCHLD, child_handler);
    // Server keeps running and waiting for signals for max 60 seconds idle
    while(1){
        if(sleep(60)>0) continue;
        printf("The server was closed because no service request was recieved for the last 60 seconds\n");
        exit(0);
    } 
}
