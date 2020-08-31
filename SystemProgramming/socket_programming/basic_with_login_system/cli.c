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
#define MAX_BUF 20
#define CONT_PORT 20001

char for_output[256] = { 0 };
void log_in(int sock_fd);

//////////////////////////////////////////////////////////////////////////
// main																	//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  								//
// Purpose : Get argument from stdout of stdout							//
//	     set socket descriptor and try to connect server  				//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	int n;  //set variable
	int sockfd; //set socket file descriptor variable
	struct sockaddr_in server_addr; //set sockaddr by internet version

	sockfd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor
	
	memset(&server_addr, 0, sizeof(server_addr)); //initialize struct to zero
	server_addr.sin_family = AF_INET; //set sin_family of socket to be IPV4
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //set IP to be user's wanted value
	server_addr.sin_port = htons(atoi(argv[2])); //set PORT to be user want balue
	
	// connect with server to use socket file descriptor
	if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2);
	
	log_in(sockfd);
	
	close(sockfd); //close server connect used by socket file descriptor
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// log_in																//
// =====================================================================//  
// Input : int -> sockfd  												//
// Purpose : set sockfd and communicate with sever 						//
//			 and functioning programming								//
//////////////////////////////////////////////////////////////////////////
void log_in(int sockfd){
		int n,count = 1;
		char user[MAX_BUF] = { 0 };
		char *passwd  = NULL;
		char buf[MAX_BUF] = { 0 };
	
		/* check if the ip is acceptable */
		n =	read(sockfd,buf,sizeof(buf)); //ACCEPTION OR REJECTION
		buf[n] = '\0';
		if(strcmp(buf,"ACCEPTION")!=0){
			
			//print out prompt
			sprintf(for_output,"%s\n","** Connection refused **");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			exit(0);
		}
		
		//print out prompt
		sprintf(for_output,"%s\n","** It is connected to Server **");
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			
		while(1){
			memset(buf,0x00,sizeof(buf));
			memset(user,0x00,sizeof(user));
			
			/* pass username and password to server */
			sprintf(for_output,"%s","Input ID : ");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			n = read(STDIN_FILENO,user,MAX_BUF);
			user[n-1] = '\0'; //delete \n
			
			
			write(sockfd,user,MAX_BUF);
			sprintf(for_output,"%s","Input Password : ");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			n = read(STDIN_FILENO,buf,MAX_BUF); 
			buf[n-1] = '\0'; //delete \n
			passwd = buf; 
			write(sockfd,passwd,MAX_BUF); memset(buf,0x00,sizeof(buf));
				
			//getpass function is hiding password
			/* reaction at Server */
			n = read(sockfd,buf,MAX_BUF);
			buf[n] = '\0';
			if(!strcmp(buf,"OK")){
				// login success
				sprintf(for_output,"%s%s%s\n","** User `[",user,"]' logged in **");
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				exit(1);
			}
			else if(!strcmp(buf,"FAIL")){
				// login fail
				sprintf(for_output,"%s\n","Log-in-failed");
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				count ++;
			}
			else if(!strcmp(buf,"DISCONNECTION")){
				/// buf is "DISCONNECTION"
				sprintf(for_output,"%s\n","** Connection closed **");
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				exit(0);
			}
			
		}
}
