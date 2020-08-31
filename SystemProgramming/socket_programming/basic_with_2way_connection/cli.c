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
#include <sys/poll.h>
#include <sys/stat.h>

#define random_port 38448

#define BUF_SIZE 256
#define MAX_BUF 20

char for_output[256] = { 0 };
char * convert_addr_to_str(unsigned long ip_addr, unsigned int port);
//////////////////////////////////////////////////////////////////////////
// main																	//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  								//
// Purpose : Get argument from stdout of stdout							//
//	     set socket descriptor and try to connect server  				//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	int n,len,len_of_result;  //set variable
	int control_fd,data_fd,listen_fd; //set socket file descriptor variable
	char *hostport;
	char buf[1024] = { 0 };
	char buf_2[1024] = { 0 }; //send port Instruction to 
	char buf_L[1024] = { 0 };
	struct sockaddr_in temp;
	struct sockaddr_in server_addr;
	struct sockaddr_in from_server_addr;
	
	struct pollfd fds[1];
	
	control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
	memset(&temp, 0, sizeof(temp)); //initialize struct to zero
	memset(&from_server_addr, 0, sizeof(from_server_addr)); //initialize struct to zero
	memset(&server_addr, 0, sizeof(server_addr)); //initialize struct to zero
	
	server_addr.sin_family = AF_INET; //set sin_family of socket to be IPV4
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //set IP to be user's wanted value
	server_addr.sin_port = htons(atoi(argv[2])); //set PORT to be user want balue
	if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor

	write(STDOUT_FILENO,"> ",2); //make prompt
	n = read(STDIN_FILENO,buf,sizeof(buf)); //set instruction into buf
	buf[n-1] = '\0'; //remove \n
	//change to ftp instruction
	if(!strcmp(buf,"ls")){
		memset(buf,0x00,sizeof(buf));
		strcpy(buf,"NLST");
	}
	else{ printf("It is other instruction\n"); exit(0); }

	/*set client to be server*/
	listen_fd = socket(AF_INET, SOCK_STREAM, 0 );
	temp.sin_family = AF_INET; // set temp.sin_family
	temp.sin_addr.s_addr = inet_addr(argv[1]); //set temp.sin_addr.s_addr
	temp.sin_port = htons(random_port); //set temp.sin_port
	hostport = convert_addr_to_str(temp.sin_addr.s_addr, temp.sin_port); 	//eg) hostport = 127,0,0,1,48,57
	
	/*make PORT Instruction*/
	sprintf(buf_2,"%s %s","PORT",hostport);
	write(control_fd,buf_2,sizeof(buf_2)); //send PORT instruction to server

	/*200*/
	n = read(control_fd,for_output,sizeof(for_output)); //read reaction of PORT instruction from server
	for_output[n] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
	write(STDOUT_FILENO,"\n",1);	  
	write(control_fd,buf,sizeof(buf)); //send ftp command to server
	
	/*to be server*/
	bind(listen_fd,(struct sockaddr *)&temp,sizeof(temp));
	listen(listen_fd,5);
	len = sizeof(from_server_addr);
	data_fd = accept(listen_fd,(struct sockaddr *)&from_server_addr, &len);
	
	/*150*///client to be server for data connection
	n = read(control_fd,for_output,sizeof(for_output)); //read reaction of data connection from server 
	for_output[n] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
	write(STDOUT_FILENO,"\n",1);
	close(control_fd);
	
	//set result value of ls from server
	n = read(data_fd,for_output,sizeof(for_output)); 
	len_of_result = strlen(for_output);	  
	for_output[len_of_result] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out	  
	close(data_fd);
	
	/* send 226 */
	sprintf(buf_L,"%s\n","226 Result is sent successfully");
	control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
	if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor
	write(STDOUT_FILENO,buf_L,strlen(buf_L));
	write(control_fd,buf_L,strlen(buf_L));
	
	/*print OK*/
	sprintf(for_output,"%s%d%s\n","OK. ",len_of_result," bytes is received.");
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	return 0;  

	close(control_fd); //close server connect used by socket file descriptor
	
}
//////////////////////////////////////////////////////////////////////////
// convert_addr_to_str													//
// =====================================================================//  
// Input : unsigned long -> ip_addr , unsigned int -> port 				//
// Purpose : for making PORT instruction with ipa and port				//
//////////////////////////////////////////////////////////////////////////
char * convert_addr_to_str(unsigned long ip_addr, unsigned int port){
	char * return_data = (char *)malloc(1024);
	/*for ip*/
	char c_ip[10] = { 0 };
	struct sockaddr_in temp;
	/*for port*/
	char c_port[10] = { 0 };
	char * c_b_port = (char *)malloc(15);
	memset(c_b_port,0x00,sizeof(c_b_port));
	char c_port_R1[10] = { 0 };
	char c_port_R2[10] = { 0 };
	char c_o_1_port[7] = { 0 };
	char c_o_2_port[7] = { 0 };
	/*other variable*/
	int decimal,index = 15;
	char cut[3] = " .\0";
	/*set ip to decimal*/
	temp.sin_addr.s_addr = ip_addr;
	strcpy(c_ip,inet_ntoa(temp.sin_addr)); //127.0.0.1
	/*set port to decimal*/
	sprintf(c_port,"%u",ntohs(port)); //38448
	/*make ip with ","*/
	strcpy(return_data,strtok(c_ip,cut));		//127
	for(int i = 0; i<3; i++){
		strcat(return_data,",");				//127,	127,0,	127,0,0,
		strcat(return_data,strtok(NULL,cut));	//127,0	127,0,0	127,0,0,1
	}
	strcat(return_data,","); //127,0,0,1,
	/*type of port will change to binary*/
	decimal = atoi(c_port);
	while(decimal != 0){
		if((decimal % 2) == 0) c_b_port[index] = '0';
		else c_b_port[index] = '1';
		decimal = decimal / 2;	
		index--;
	}
	c_b_port[16] = '\0';
	/*cut to half*/
	strncpy(c_o_1_port,c_b_port,8); //10010110
	sprintf(c_port_R1,"%ld",strtol(c_o_1_port,NULL,2)); //10010110-> 150
	strcat(return_data,c_port_R1); //127,0,0,1,150
	strcat(return_data,",");	   //127,0,0,1,150,
	/*cut to other half*/
	for(int i = 7; i<16; i++) c_o_2_port[i-7] = c_b_port[i]; //00110000
	sprintf(c_port_R2,"%ld",strtol(c_o_2_port,NULL,2));//00110000->48
	strcat(return_data,c_port_R2);//127,0,0,1,150,48
	/*return data*/
	return return_data;
}  
