#include "csapp.h"  //tiny/csapp.h파일 불러오기

void echo(int connfd);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;


  if(argc !=2){
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);

  }

  listenfd=Open_listenfd(argv[1]);
  
  while(1){
    clientlen=sizeof(clientaddr);
    connfd=Accept(listenfd, (SA *)&clientaddr, &clientlen);
    echo(connfd);
    Close(connfd);
  }
} 

void echo(int connfd){
  size_t n;
  char buf[MAXLINE];
  rio_t rio;
  
  Rio_readinitb(&rio, connfd);

  while((n=Rio_readlineb(&rio, buf, MAXLINE))!=0){
    Rio_writen(connfd, buf, n);
    printf("server received %d bytes: %s", (int)n, buf);
  }
}






