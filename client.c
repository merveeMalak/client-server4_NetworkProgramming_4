#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVERPORT 8888
#define MAXBUF 1024

/*Checks if the given file exists,
 returns 0 if there is, or -1.
*/ 
int checkFileExist(char* fileName){
    if (access(fileName, F_OK) == 0){
        return 0;
    }
    return -1;
}
int main(int argc, char* argv[]){
    int sockd;
    int counter;
    int fd;
    struct sockaddr_in xferServer;
    char buf[MAXBUF];
    int returnStatus;
    if (argc < 3){
        fprintf(stderr, "Usage: %s <ip address> <filename> [dest filename]\n",
        argv[0]);
        exit(1);
    }

    /* create a socket */
    sockd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockd == -1){
        fprintf(stderr, "Could not create socket!\n");
        exit(1);
    }

    /* set up the server information */
    xferServer.sin_family = AF_INET;
    char* ip = argv[1];
    xferServer.sin_addr.s_addr = inet_addr(ip);
    xferServer.sin_port = htons(SERVERPORT);
    
    /* connect to the server */
    returnStatus = connect(sockd,(struct sockaddr*)&xferServer,sizeof(xferServer));
    if (returnStatus == -1){
        fprintf(stderr, "Could not connect to server!\n");
        exit(1);
    }

    /* send the name of the file we want to the server */
    returnStatus = write(sockd, argv[2], strlen(argv[2])+1);
    if (returnStatus == -1){
        fprintf(stderr, "Could not send filename to server!\n");
        exit(1);
    }
    /* call shutdown to set our socket to read-only */
    shutdown(sockd, SHUT_WR);
    char rsp[5];
    int flag = read(sockd, rsp, sizeof(rsp));
    if (flag == -1){
         fprintf(stderr, "Could not read file from socket!\n");
        exit(1);
    }
    /*There is no access to the file or 
     the file name is not valid. 
    */
    if (strncmp("false", rsp, 5) == 0){
        printf("Could not open %s file!\n", argv[2]);
        exit(1);
    }

    if (checkFileExist(argv[3]) == 0){ // Destination file already exists.
        printf("File exists\n");
        char response;
        printf("Please press y to append to %s file: ", argv[3]);
        scanf("%c", &response);
        if (response == 'y'){ //append to the destination file. 
            
            /* open up a handle to our destination file to receive the contents from the server */
            fd = open(argv[3], O_WRONLY | O_APPEND, 0666);
            if (fd == -1){
                fprintf(stderr, "Could not open destination file, using stdout.\n");
                fd = 1;
            }
         /* read the file from the socket as long as there is data */
            while ((counter = read(sockd, buf, MAXBUF)) > 0){
            /* send the contents to stdout */
                printf("The data is appended to %s file...\n", argv[3]);
                write(fd, buf, counter);
            }
            if (counter == -1){
                fprintf(stderr, "Could not read file from socket!\n");
                exit(1);
            }
        }else{ //not append to the destination file 
            printf("No changes were made to the %s file.\n", argv[3]);
        }
    }
    else{ //Destination file does not exist.
        printf("File is not exist\n");
        printf("Creating a %s file...\n", argv[3]);

        /* open up a handle to our destination file to receive the contents from the server */
        fd = open(argv[3], O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1){
            fprintf(stderr, "Could not open destination file, using stdout.\n");
            fd = 1;
        }
        /* read the file from the socket as long as there is data */
        while ((counter = read(sockd, buf, MAXBUF)) > 0){
        /* send the contents to stdout */
            printf("The data is writed to %s file...\n", argv[3]);
            write(fd, buf, counter);
        }
        if (counter == -1){
            fprintf(stderr, "Could not read file from socket!\n");
            exit(1);
        }
        
    }

    close(sockd);
    return 0;
}