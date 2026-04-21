/*
 * adder.c - 두 수를 더하는 최소한의 CGI 프로그램
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
  char *buf, *p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1=0, n2=0;
  buf=getenv("QUERY_STRING");
  p=strchr(buf, '&');
  *p='\0';
  strcpy(arg1, buf);
  strcpy(arg2, p+1);
  n1=atoi(arg1);
  n2=atoi(arg2);
  printf("Content-type: text/html; charset=UTF-8\r\n\r\n");
  sprintf(content, "<html><body>\r\n");
  sprintf(content + strlen(content), "두 수의 합: %d + %d = %d\r\n", n1, n2, n1+n2);
  sprintf(content + strlen(content), "</body></html>\r\n");
  printf("%s", content);
  exit(0);
}
/* $end adder */
