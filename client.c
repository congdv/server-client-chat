#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

void error(const char *msg){
    fprintf(stderr,"%s : %s\n",msg,strerror(errno));
    exit(1);
}

int open_socket_connect(const char *host, int port){
    int socket_d = socket(PF_INET,SOCK_STREAM,0);
    if(socket_d == -1){
        error("Can't create socket");
    
    }

    //Create a socket address for host and port
    struct sockaddr_in si;
    memset(&si,0,sizeof(si));
    si.sin_family = PF_INET;
    si.sin_addr.s_addr = inet_addr(host);
    si.sin_port = htons(port);
    int c =connect(socket_d,(struct sockaddr *)&si,sizeof(si));
    if(c == -1){
        error("Can't connect to socket");
    }
    return socket_d;
}

void say(int socket, const char *msg){
   int result = send(socket,msg,strlen(msg),0);
   if(result == -1){
        char buff[255];
        sprintf(buff,"Can't send msg: \"%s\" client queue",msg);
        error(buff); 
   }
}

int read_msg_from_server(int socket,char *buf,int len){
   char *s = buf;
   int slen = len;
   int c = recv(socket,s,slen,0);
   //until end of line
   while( c > 0 && s[c -1]!='\n' ){
       //next character for store
      s += c;
        //min size for store
      slen -= c;
      c = recv(socket,s,slen,0);
   }

    if(c < 0)
        return c;
    else if (c == 0)
        buf[0] = '\0';
    else 
        s[c-1] = '\0';
    return len - slen;

}

int main(){
    int d_sock = open_socket_connect("127.0.0.1",30000);
    puts("Connected to server");

    char recv_msg[255];
    char msg[255];
    while(1){
       //fflush(stdin);
       printf("You say: ");
       fgets(msg,255,stdin);
       //fscanf(stdin,"%s",msg); Receive enter char for end on fgets, so cannot use fscanf
       say(d_sock,msg);
       read_msg_from_server(d_sock,recv_msg,sizeof(recv_msg));
       fprintf(stdout,"Server: ");
       puts(recv_msg);
    }

    close(d_sock);
}
