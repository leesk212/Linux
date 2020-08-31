///////////////////////////////////////////////////////////////////////
// File Name : cli.c 					     	     //
// Date : 2020/05/18 						     //
// Os : Ubuntu 12.04 LTS 64bits 				     //
// Author : Lee Suk Min						     //
// Student ID : 2016722013 					     //
// ----------------------------------------------------------------- //
// Title : System Programming practice #2-1 			     //
// Description : cli.c Only change command to srv version and send   //
// 		 value to socket	     			     //
///////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <unistd.h>  
#include <stdlib.h>

#include <sys/types.h>  //socket()
#include <sys/socket.h> //connect() socket()
#include <netinet/in.h> //inet_adder()
#include <arpa/inet.h>	//htonl , htons , ntohl , ntohs

#define	BUFFSIZE	1024

//////////////////////////////////////////////////////////////////////////
// conv_cmd								//
// =====================================================================//  
// Input : char * -> buf , char * -> cmd_buf				//
// Output : int success/fail (0/-1)		  			//
// Purpose : change command to server command				//
//////////////////////////////////////////////////////////////////////////
int conv_cmd(char *buf, char *cmd_buf){
	int n; //make to string
	char * ptr = NULL; // to use STRTOK
	n = read(STDIN_FILENO,buf,sizeof(buf)); //set buf value from string input
	buf[n] = '\0'; // buf is changed to string
	buf[n-1] = '\0'; // buf is changed to string, delete '\n'
	if(strncmp(buf,"ls",2)!=0)
		if(strncmp(buf,"quit",4)!=0)
			return -1;

	if(!strncmp(buf,"quit",4)){ //quit
		strcpy(cmd_buf,"QUIT");		
	}
	else{//ls
		if(buf[3] == '-'){//ls -a, ls -l, ls -al
			strcpy(cmd_buf,"NLST"); //ls is NLST
			strcat(cmd_buf," ");	//set NLST_
			ptr = strtok(buf," ");	//get after token value	
			ptr = strtok(NULL, " ");
			strcat(cmd_buf,ptr);	//set NLST_"?"
			cmd_buf[strlen(cmd_buf)] = '\0'; 
		}
		else strcpy(cmd_buf,"NLST"); //ls is NLST
				
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// process_result							//
// =====================================================================//  
// Input : char * -> rcv_buf						//
// Purpose : print result using write system call 			//
//////////////////////////////////////////////////////////////////////////
void process_result(char *rcv_buf){ //print out result of ls
	write(STDOUT_FILENO, rcv_buf,strlen(rcv_buf));
}
//////////////////////////////////////////////////////////////////////////
// main									//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  				//
// Purpose : Get argument from stdout of stdout				//
//	     Do function by instruction					//
//	     Set 2D value array like argv		 	 	//
//	     set socket descriptor and try to connect server  		//
//	     set command from user				        //
//////////////////////////////////////////////////////////////////////////
int main(int argc,char **argv)
{
	int socket_fd, n,i; //socket file descripter nubmer, and variable
	struct sockaddr_in server_addr; // to make socket file_descripter
	char PORTNO[1024] = { 0 };//changable port number
	char buf[BUFFSIZE];	//original command
	char cmd_buf[BUFFSIZE]; //change to srv command
	char rcv_buf[BUFFSIZE]; //value of result of command
	
	bzero( (char*)&server_addr, sizeof(server_addr)); //intialize struct server_addr
	bzero(buf,sizeof(buf) ); //initailize buf
	bzero(cmd_buf,sizeof(cmd_buf) ); //initailize cmd_buf
	bzero(rcv_buf,sizeof(rcv_buf) ); //initailize rcv_buf	

	strcpy(PORTNO,argv[2]); //set changable port number

	if( (socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) { //make socket file descripter
		write(STDOUT_FILENO, "ERR 0", 6 );//print prompt
		return -1;
	}
	//setting server variable in server_addr struct
	server_addr.sin_family = AF_INET; //set family
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //set ip
	server_addr.sin_port = htons(atoi(PORTNO)); //set port number
	
	//try to connect with server
	if( connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		write(STDOUT_FILENO, "ERR 1\n", 6 );//print prompt
		return -1;
	}


	write(STDOUT_FILENO, "> ", 2 );//print prompt
	while(1){//set infinite loop , if command is QUIT, exit
		if(conv_cmd(buf,cmd_buf) < 0 ){ //convert command to server command
			write(STDOUT_FILENO, "ERR A\n", 6 );//print prompt
			exit(1);
		}
		n = strlen(cmd_buf); //set cmd_buf length into n
		if(write(socket_fd, cmd_buf, n)!=n){//send cmd_buf to socket file descripter
			write(STDOUT_FILENO, "ERR B\n", 6 );//print prompt
			exit(1);
		}
		if((n = read(socket_fd,rcv_buf,sizeof(rcv_buf)) ) < 0 ){ 
			//set result value from socket
			write(STDOUT_FILENO, "ERR C\n", 6 );//print prompt
			exit(1);
		}
		rcv_buf[n] = '\0'; //result value is changed to string


		if(!strncmp(rcv_buf,"QUIT",4)){ //if result is QUIT, finish cli.c
			write(STDOUT_FILENO,"Program quit!!\n",15);
			close(socket_fd); //close socket file descripter
			return 0;
		}
		process_result(rcv_buf); //print out ls result
		memset(buf, 0x00, 1024);
		memset(cmd_buf, 0x00, 1024);
		memset(rcv_buf, 0x00, 1024);
		rewind(stdout); rewind(stdin); rewind(stderr);
		write(STDOUT_FILENO, "\n> ", 3 );//print prompt
	}		
}
