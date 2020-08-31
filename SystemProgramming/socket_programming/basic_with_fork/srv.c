#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//memset() function
#include <unistd.h>	//
#include <arpa/inet.h>	//for htons() function, inet_addr() function
#include <sys/types.h>	//for socket() function argument
#include <sys/socket.h> //for struct sockaddr and socket() function, bind() function, 
			//connect() function, listen() function, accept() function, close()function

#include <netinet/in.h>	//for struct sockaddr_in and 
#include <sys/wait.h>	//for wait() function
#include <signal.h>	//for signal() function

#define BUF_SIZE 256

void sh_chld(int);	//signal handler for SIGCHLD
void sh_alrm(int);	//signal handler for SIGALRM
//////////////////////////////////////////////////////////////////////////
// main									//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  				//
// Purpose : Get argument from stdout of stdout				//
//	     Do function by instruction					//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	char buf[BUF_SIZE]; //set buf to buffsize
	int n; //set variable
	struct sockaddr_in server_addr, client_addr; //set sockaddr by internet version
	int server_fd, client_fd; //set variable server_fd and client_fd
	int len; //set variable
	int port; //set variable
	
	signal(SIGALRM, sh_alrm); //catch signal that named SIGARLM presented by alarm function
	signal(SIGCHLD, sh_chld); //catch signal that named SIGCHLD presented by CHILD process state is changed

	server_fd = socket(PF_INET, SOCK_STREAM, 0 ); //set server file descriptor
	
	memset(&server_addr, 0, sizeof(server_addr)); //initialize server_addr to zero
	memset(&buf,0,sizeof(buf)); //initialize buf to zero
	server_addr.sin_family = AF_INET; //IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //host IP to network version long 
	server_addr.sin_port = htons(atoi(argv[1])); //host Port to network version short

	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //bind socket_file descirptor with server_addr 
	
	listen(server_fd, 5); //can wait five other cli at one server
	
	while(1){
		pid_t pid; //set variable to get folk nubmer
		len = sizeof(client_addr); //set len 
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len); //make file_descriptor for connecting with other client
		//print out currnet client pid
		printf("===========Client info===========\n"); //print prompt
		printf("client IP: %s\n\n",inet_ntoa(client_addr.sin_addr));// use inet_ntoa to see ip nubmer to host
		printf("client port: %d\n",ntohs(client_addr.sin_port)); // use ntohs to see port number to host
		printf("=================================\n");	
		
		if((pid = fork()) == 0){// child process of server
			close(server_fd); //close listen descriptor because 1 by 1 server "not need"
			char data[128] = { 0 };	//for returning data		
			sprintf(data,"%s%d\n","Child Process ID : ",getpid());	//for print out 			
			write(STDOUT_FILENO,data,sizeof(data));	//write data at STDOUT
			while(1){ //need to make infinite loop for 1 by 1 
				read(client_fd,buf,sizeof(buf));	
				len = strlen(buf);			
				buf[len-1] = '\0';//to make string without '\n'
				if(!strcmp(buf,"QUIT")){ //finish to cli
					alarm(1);
					sleep(1);
					exit(0); //end of connection with client	
				}
				write(client_fd,buf,sizeof(buf)); //write data 			
			}
			close(client_fd); //finish to connect descriptor		
		}	
		close(client_fd); //finish to connect descriptor
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// sh_scld								//
// =====================================================================//  
// Input : int -> signum						//
// Purpose : for argument of signal function 				//
//	     if child process state is changed, function will be	//
//	     operating							//
//////////////////////////////////////////////////////////////////////////
void sh_chld(int signum) {
	printf("Status of Child process was changed.\n");
	wait(NULL); //wait when QUIT is come 
}
//////////////////////////////////////////////////////////////////////////
// sh_alrm								//
// =====================================================================//  
// Input : int -> signum						//
// Purpose : for argument of signal function 				//
//	     if alrm function operated, function will be operating	//
//////////////////////////////////////////////////////////////////////////
void sh_alrm(int signum) {
	printf("Child Process(PID: %d) will be terminated.\n",getpid());
	exit(1);
}
