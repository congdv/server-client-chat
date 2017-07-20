#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>


void error(const char *msg){
    fprintf(stderr,"%s : %s\n",msg,strerror(errno));
    exit(1);
}

int create_listen_socket(){
   int s = socket(PF_INET,SOCK_STREAM,0);
   if(s == -1){
        error("Can't create listen socket");
   }
   return s;
}

void bind_to_port_listen(int socket, int port){
   struct sockaddr_in server;
   server.sin_family = PF_INET;
   server.sin_port = (in_port_t)htons(port);
   server.sin_addr.s_addr = htonl(INADDR_ANY);

   //set restart reuse port of server
   int reuse = 1;
   if(setsockopt(socket,SOL_SOCKET,SO_REUSEADDR,(char *)&reuse,sizeof(int)) == -1){
       error("Can't set reuse port after restarting server"); 
   }
    
   //Bind port
    int bind_port = bind(socket,(struct sockaddr *)&server,sizeof(server));  

    if(bind_port == -1){
        char buff[255];
        sprintf(buff,"Can't bin to port %d of server",port);
        error(buff); 
    }
}

void listen_queue(int socket,int queue_num){
   if( listen(socket,queue_num) == -1){
        char buff[255];
        sprintf(buff,"Can't create %d client queue",queue_num);
        error(buff); 
   }
}

void say(int socket, const char *msg){
   int result = send(socket,msg,strlen(msg),0);
   if(result == -1){
        char buff[255];
        sprintf(buff,"Can't send msg: \"%s\" client queue",msg);
        error(buff); 
   }
}

int read_msg_from_client(int socket,char *buf,int len){
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

int listen_socket_server;
void handle_shutdown(int sig){
    if (listen_socket_server){
        close(listen_socket_server);
    }
    fprintf(stderr,"Bye!\r\n");
    exit(0);
}



int catch_signal(int sig,void (*handler) (int)){
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig,&action,NULL);
}
int main(){

    if(catch_signal(SIGINT,handle_shutdown) == -1)
        error("Can't set the interrupt handler");

    listen_socket_server = create_listen_socket();

    bind_to_port_listen(listen_socket_server,30000);

    listen_queue(listen_socket_server,10);  
    
    puts("Waiting client connect...");

    while(1){
       struct sockaddr_storage client_addr;
       unsigned int address_size = sizeof(client_addr); 
       int connect_d = accept(listen_socket_server,(struct sockaddr *) &client_addr, &address_size);
       if(connect_d == -1){
            error("Can't open secondary socket");
        }
       char msg[255];
       char recv_msg[255];
       do {
           read_msg_from_client(connect_d,recv_msg,sizeof(recv_msg));
           fprintf(stdout,"Client:");
           puts(recv_msg);
           fprintf(stdout,"Server say: ");
           fflush(stdin);
           fgets(msg,255,stdin);
           say(connect_d,msg);
       }while(strncmp(msg,"exit",4));
       close(connect_d);
       close(listen_socket_server);
       return 0;

    }

    return 0;

}
