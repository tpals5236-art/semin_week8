/* $begin tinymain */
/*
 * tiny.c - GET 메서드를 사용해 정적/동적 콘텐츠를 제공하는
 *     단순한 반복형 HTTP/1.0 웹 서버
 *
 * 2019년 11월 수정
 *   - serve_static()와 clienterror()의 sprintf() 별칭 문제 수정
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* 명령행 인자 확인 */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}
void doit(int fd){
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;
  int is_static;

  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s",method,uri,version);
//대소문자를 무시하고 문자열을 비교하는 함수 strcasecmp()
  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "501","Not Implemented","Tiny는 이 메서드를 구현하지 않았습니다..");
    return;
  }
  read_requesthdrs(&rio);
  is_static=parse_uri(uri, filename, cgiargs);
  if(stat(filename, &sbuf)<0){
    clienterror(fd, filename, "404","Not found", "Tiny에서 파일 찾지 못했습니다.");
    return;
  }
  if(is_static){
    if(!(S_ISREG(sbuf.st_mode))|| !(S_IRUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tiny는 이 파일을 읽을 수 없습니다.");
      return;
    }
    serve_static(fd,filename, sbuf.st_size);
    return;
  }
  else{
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tny는 프로그램 실행 권한이 없습니다.");
      return;
    }
    serve_dynamic(fd, filename,cgiargs);
    return;
  }
}

void read_requesthdrs(rio_t *rp){
  char buf[MAXLINE];
  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf,"\r\n")){
    printf("%s", buf);
    Rio_readlineb(rp, buf, MAXLINE);
  }
}

int parse_uri(char *uri, char *filename, char *cgiargs){
  if(!strstr(uri, "cgi-bin")){
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if(uri[strlen(uri)-1]=='/'){
      strcat(filename,"home.html" );
      return 1;
    }
  return 1;
  }
  else{
    char *ptr = strchr(uri, '?');
    if(ptr){
      strcpy(cgiargs, ptr+1);
      *ptr='\0';
      strcpy(filename, ".");
      strcat(filename, uri);
      return 0;
    }
    else{
      strcpy(cgiargs, "");
      strcpy(filename,".");
      strcat(filename, uri);
      return 0;
    }
  }
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
  char buf[MAXLINE], body[MAXBUF];
  strcpy(body,"");
  strcat(body, "<html><title>Tiny Error</title>");
  strcat(body,"<body bgcolor=\"00ff00\">\r\n");
  strcat(body, errnum);
  strcat(body, " ");
  strcat(body, shortmsg);
  strcat(body, "<p>");
  strcat(body, longmsg);
  strcat(body, ": ");
  strcat(body, cause);
  strcat(body, "\r\n<hr><em>The Tiny Web server</em>\r\n");
  sprintf(buf, "HTTP/1.0 %s %s\r\n",errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void serve_static(int fd, char *filename, int filesize){
  
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  strcat(buf, "Server: Tiny Web Server\r\n");
  snprintf(buf + strlen(buf), MAXBUF - strlen(buf), "Content-length: %d\r\n", filesize);
  snprintf(buf + strlen(buf), MAXBUF - strlen(buf), "Content-type: %s\r\n\r\n", filetype);
  Rio_writen(fd, buf, strlen(buf));
  srcfd=Open(filename, O_RDONLY, 0);
  srcp=Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype){
  if(strstr(filename, ".html")){
    strcpy(filetype, "text/html");
  }
  else if(strstr(filename, ".gif")){
    strcpy(filetype, "image/gif");
  }
  else if(strstr(filename, ".png")){
    strcpy(filetype, "image/png");
  }
  else if(strstr(filename, ".jpg")){
    strcpy(filetype, "image/jpeg");
  }
  else if(strstr(filename, ".mpg")){
    strcpy(filetype, "video/mpeg");
  }
  else{
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char *filename, char *cgiargs){
  char buf[MAXLINE], *emptylist[]={NULL};
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf,"Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));
  if(Fork()==0){
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd,STDOUT_FILENO);
    Execve(filename, emptylist, environ);
  }
  Wait(NULL);
}
