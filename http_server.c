/*
 * A simple server in the internet domain using TCP The port number is
 * passed as an argument
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>

#define BUFFSIZE 4096
void error(char *);	// prototype

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFFSIZE];	// for communicating with client
    pid_t pid;
    FILE *F;

    struct stat S;		// to find file length
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    // some buffering for the child process
    char *TmpBuffer, *SavePtr, *FileName, *GetToken;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    while (1) {
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)	// exit server
            error("ERROR on accept");
        // otherwise let's fork
        pid = fork();
        if (pid < 0)
            error("bad fork\n");
        if (pid > 0) /* parent */
            continue;
        if (pid == 0) /* child */
            break;		// to handle talking with client below
    }

    memset(buffer, 0, BUFFSIZE);
    n = read(newsockfd, buffer, BUFFSIZE - 1);
    if (n < 0)
        error("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);
    printf("First Token: %s\n", (TmpBuffer = strtok_r(buffer, "\n", &SavePtr)));
    GetToken = strtok_r(TmpBuffer, " ", &SavePtr);
    printf("Second Token: %s\n", GetToken);

    GetToken = strtok_r(NULL, " ", &SavePtr);
    printf("Final Token: %s After Get\n", GetToken);
    GetToken++;
    int len = strlen(GetToken);
    const char *last_three = &GetToken[len-3];

    // now open the file and send it to client ?
    if ((F = fopen(GetToken, "r")) == NULL)
        error("Bad Dog\n");
    else
        printf("Good Dog\n");
    int FileSize;
    if ((fstat(fileno(F), &S) == -1))
        error("failed fstat\n");	// Need file size
    FileSize = S.st_size;

    // Looks ok -- now let's write the request header
    // Let's just fill a buffer with header info using sprintf()
    char Response[BUFFSIZE];
    int HeaderCount = 0;
    HeaderCount = 0; // Use to know where to fill buffer with sprintf
    HeaderCount += sprintf(Response + HeaderCount, "HTTP/1.0 200 OK\r\n");
    HeaderCount += sprintf(Response + HeaderCount, "Server: Flaky Server/1.0.0\r\n");
    if(strcmp(last_three,"mp3")==0){
    HeaderCount += sprintf(Response + HeaderCount, "Content-Type: audio/mpeg\r\n");
    }
    if(strcmp(last_three,"jpg")==0){
    HeaderCount += sprintf(Response + HeaderCount, "Content-Type: image/jpg\r\n");
    }
    if(strcmp(last_three,"tml")==0){
    HeaderCount += sprintf(Response + HeaderCount, "Content-Type: text/html\r\n");
    }
    if(strcmp(last_three,"pdf")==0){
    HeaderCount += sprintf(Response + HeaderCount, "Content-Type: application/pdf\r\n");
    }

    if(strcmp(last_three,"txt")==0){
    HeaderCount += sprintf(Response + HeaderCount, "Content-Type: text/plain\r\n");
    }


    //HeaderCount += sprintf(Response + HeaderCount, "Content-Disposition: attachment; filename=my-file.mp3\r\n"); //download audio option
    HeaderCount += sprintf(Response + HeaderCount, "Content-Length:%d\r\n", FileSize);
    // And finally to delimit header
    HeaderCount += sprintf(Response + HeaderCount, "\r\n");

    // Let's echo that to stdout to be sure !
    fprintf(stdout, "HeaderCount %d and Header\n", HeaderCount);
    write(STDERR_FILENO, Response, HeaderCount);
    write(newsockfd, Response, HeaderCount);	// and send to client
    // Now Serve That File in one write to socket
    char *BigBuffer = malloc(FileSize + 2);	// Just being OCD
    fread(BigBuffer, 1, FileSize, F);
    write(newsockfd, BigBuffer, FileSize);

    
    free(BigBuffer);
    // Now close up this client's shop
    close(newsockfd);
    return 0;
}

void
error(char *msg)
{
    perror(msg);
    exit(1);
}
