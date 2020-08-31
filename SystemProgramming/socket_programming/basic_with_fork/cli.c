#include <stdio.h>
#include <stdlib.h>	//abort() function
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

//////////////////////////////////////////////////////////////////////////
// main									//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  				//
// Purpose : Get argument from stdout of stdout				//
//	     set socket descriptor and try to connect server  		//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	char buff[BUF_SIZE]; //set bufsize
	int n;  //set variable
	int sockfd; //set socket file descriptor variable
	struct sockaddr_in server_addr; //set sockaddr by internet version

	sockfd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor
	
	memset(&server_addr, 0, sizeof(server_addr)); //initialize struct to zero
	memset(&buff,0,sizeof(buff)); //intialize buf to zero
	server_addr.sin_family = AF_INET; //set sin_family of socket to be IPV4
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //set IP to be user's wanted value
	server_addr.sin_port = htons(atoi(argv[2])); //set PORT to be user want balue
	
	if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2);
	// connect with server to use socket file descriptor
	while(1){
		write(STDOUT_FILENO,"> ",2); //print out prompt
		if(n = (read(STDIN_FILENO, buff, BUF_SIZE)) > 0){ //read buff from standard input
			if(write(sockfd, buff, BUF_SIZE) > 0 ){ //write buff to socket file descriptor
				if(n = read(sockfd, buff, BUF_SIZE) > 0 ){ // read buff from socket file descriptor			
					char data[128] = { 0 }; //for print prompt			
					sprintf(data,"%s%s\n","from server: ",buff);//set buf to write at STDOUT
					write(STDOUT_FILENO,data,sizeof(data)); //write at STDOUT
				}
				else
					{ break;} //if fail to read value from server 
			}
			else
			{ break;} //if fail to write value to server
			memset(buff,0,sizeof(buff)); //initialize buff
		}else{ break; }
	}
	close(sockfd); //close server connect used by socket file descriptor
	return 0;
}
	
