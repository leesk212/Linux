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

#define BUF_SIZE 2048
#define MAX_BUF 2048

char for_output[256] = { 0 };
int Make_to_ftp_cmd(int listen_fd, int control_fd, char * buf, char *cmd_buf,char *argv_from_m[],struct sockaddr_in server_addr);
char * convert_addr_to_str(unsigned long ip_addr, unsigned int port);
void log_in(int sockfd);
void open_data_connection(int listen_fd, int control_fd, char * cmd_buf, char *argv[],struct sockaddr_in server_addr);
//////////////////////////////////////////////////////////////////////////
// main																	//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  								//
// Purpose : Get argument from stdout of stdout							//
//	     set socket descriptor and try to connect server  				//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	int n,first = 0,len,len_of_result;  //set variable
	int control_fd,data_fd = 0,listen_fd; //set socket file descriptor variable
	struct sockaddr_in server_addr;
	
	control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
	memset(&server_addr, 0, sizeof(server_addr)); //initialize struct to zero
	
	server_addr.sin_family = AF_INET; //set sin_family of socket to be IPV4
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); //set IP to be user's wanted value
	server_addr.sin_port = htons(atoi(argv[2])); //set PORT to be user want balue
/**/if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor

	log_in(control_fd);
	
	while(1){
		char buf[1024] = { 0 };
		char ftp_buf[1024] = { 0 };
		int Are_you_open_data_fd;
		if(first != 0){
			control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
			if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor
		}
		write(STDOUT_FILENO,"ftp> ",5); //make prompt
		n = read(STDIN_FILENO,buf,sizeof(buf)); //set instruction into buf
		buf[n-1] = '\0'; //remove \n
		//change to ftp instruction
	
		Are_you_open_data_fd = Make_to_ftp_cmd(listen_fd,control_fd,buf,ftp_buf,argv,server_addr);
		if( Are_you_open_data_fd < 0 ){
			sprintf(for_output,"%s: Wrong Instruction\n",buf);
			write(STDOUT_FILENO, for_output,sizeof(for_output) ); memset(for_output,0x00,sizeof(for_output)); //print out
			write(STDOUT_FILENO,"\n",1);
			first ++;
			continue;
		}
		if( Are_you_open_data_fd > 0 ){
			first ++;
			continue;
		}		
		memset(buf,0x00,sizeof(buf));
		
		//send ftp command to server
		write(control_fd,ftp_buf,sizeof(ftp_buf)); 
		
		//receive reaction of command from server
		n = read(control_fd,for_output,sizeof(for_output)); 
		len_of_result = strlen(for_output);
		for_output[len_of_result] = '\0';
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
	
		if(strncmp(ftp_buf,"RNFR",4)==0){
			n = read(control_fd,for_output,sizeof(for_output)); 
			len_of_result = strlen(for_output);
			for_output[len_of_result] = '\0';
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
		}
		if(strncmp(ftp_buf,"QUIT",4)==0){
			exit(0);
		}
		first ++;
		close(control_fd);
	}	
		
}

//////////////////////////////////////////////////////////////////////////
// opeb_data_connection													//
// =====================================================================//  
// Input : unsigned long -> ip_addr , unsigned int -> port 				//
// Purpose : for making PORT instruction with ipa and port				//
//////////////////////////////////////////////////////////////////////////
void open_data_connection(int listen_fd, int control_fd, char * cmd_buf,char *argv[],struct sockaddr_in server_addr){
	char buf_2[1024] = { 0 }; //send port Instruction to 
	struct sockaddr_in temp;
	struct sockaddr_in from_server_addr;
	char *hostport;
	int n,len,data_fd,len_of_result; 
	memset(&from_server_addr, 0, sizeof(from_server_addr)); //initialize struct to zero
	char buf_L[1024] = { 0 };

	memset(&temp, 0, sizeof(temp)); //initialize struct to zero
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
	
	
	/*150*///client to be server for data connection
	n = read(control_fd,for_output,sizeof(for_output)); //read reaction of data connection from server 
	for_output[n] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
	close(control_fd);
	
	//client will be data server
	bind(listen_fd,(struct sockaddr *)&temp,sizeof(temp));
	listen(listen_fd,5);
	len = sizeof(from_server_addr);
	data_fd = accept(listen_fd,(struct sockaddr *)&from_server_addr, &len);	
	
	
	//set result value of ls from server
	n = read(data_fd,for_output,sizeof(for_output)); 
	len_of_result = strlen(for_output);	  
	for_output[len_of_result] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out	  
	close(data_fd);
	
	/* send SUCCESS*/ 
	control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
	if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor
	write(control_fd,"SUCCESS",7); 
	
	/* receive reaction*/
	n = read(control_fd,buf_L,strlen(buf_L));  
	buf_L[strlen(buf_L)] = '\0';//to make string
	write(STDOUT_FILENO,buf_L,sizeof(buf_L)); memset(buf_L,0x00,sizeof(buf_L)); //print out	  

	/*print OK*/
	sprintf(for_output,"%s%d%s\n","OK. ",len_of_result," bytes is received.");
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));

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

int Make_to_ftp_cmd(int listen_fd, int control_fd, char * buf, char *cmd_buf,char *argv_from_m[],struct sockaddr_in server_addr){
	int n,i,j; //make to string
	char * ptr = NULL; // to use STRTOK
	int argc = 0;
	char ** argv = (char **)malloc(sizeof(char *) * 1024); //make 2D array in memory
	for (i = 0; i<1024; i++) {
		argv[i] = (char *)malloc(sizeof(char) * 1024);
		for(j =0; j<1024; j++) argv[i][j] = 0; 
	}
	char write_value[256] = { 0 }; //for write to stdin
	char pre_write_value[256] = { 0 }; //for already make write value
	write_value[0] = 'e'; //for catching error when there is not input in argv
	ptr = strtok(buf," ");	//get after token value	
	while(ptr != NULL){
		strcpy(argv[argc],ptr);
		argc++;
		ptr = strtok(NULL, " ");
	}
	if(argc == 1){ //ls,pwd,quit,dir instruction 
		if(strcmp(argv[0],"ls")==0){ // change ls to NLST
			strcpy(write_value,"NLST"); 	
		}
		if(strcmp(argv[0],"dir")==0){ // change dir to LIST
			strcpy(write_value,"LIST");
		}
		if(strcmp(argv[0],"pwd") == 0){ // change pwd to PWD
			strcpy(write_value,"PWD");
		}
		if(strcmp(argv[0],"quit") == 0){ // change quit to QUIT
			strcpy(write_value,"QUIT");
		}
		
	}//end of if when functioning argc is 2
	else{//there is two paramater at least 
		if(strcmp(argv[0],"rename") == 0){
			strcpy(pre_write_value,"RNFR");
			strcat(pre_write_value," ");
			strcat(pre_write_value,argv[1]);
			strcat(pre_write_value,"\n");
			strcat(pre_write_value,"RNTO");
			strcat(pre_write_value," ");
			strcat(pre_write_value,argv[2]);
		}  //change "rename [old_file] [new_file]" to "RNFR [old_file] \n RNTO [new_file]" 
		else{
			if(strcmp(argv[0],"get")==0){
				strcpy(pre_write_value,"RETR");
			}
			if(strcmp(argv[0],"put")==0){
				strcpy(pre_write_value,"STOR");				
			}
			if(strcmp(argv[0],"ls")==0){ // ls -a , ls -l, ls -al
				strcpy(pre_write_value,"NLST");
			}// change ls to NLST with other value
			if(strcmp(argv[0],"cd") == 0){ //
				if(strcmp(argv[1],"..") == 0){
					strcpy(pre_write_value,"CDUP");	
				}					
				else
					strcpy(pre_write_value,"CWD");
			} // change "cd [path], cd.. [path]" to "CWD" and "CDUP"
			if(strcmp(argv[0],"mkdir") == 0){ //mkdir [file_name]
				strcpy(pre_write_value,"MKD");
			} // change "mkdir" to "MKD"
			if(strcmp(argv[0],"delete") == 0){ //delete [file_name]
				strcpy(pre_write_value,"DELE");
			} // change "rdmir" to "RMD"
			if(strcmp(argv[0],"rmdir") == 0){ //rmdir [file_name]
				strcpy(pre_write_value,"RMD");
			}
			for(i=0;i<argc-1;i++){//with other value such as option or argument 
				strcat(pre_write_value," ");
				strcat(pre_write_value,argv[i+1]);
			}//end of for
		}//end of else
		strcpy(write_value,pre_write_value); //if finish to make prevalue, decide real value 	
	}//end of large else
		
	strcpy(cmd_buf,write_value);	
	
	if(write_value[0] == 'e') return -1;
	if(strncmp(cmd_buf,"NLST",4)==0||strncmp(cmd_buf,"LIST",4)==0||strncmp(cmd_buf,"RETR",4)==0||strncmp(cmd_buf,"STOR",4)==0 ){
		write(control_fd,cmd_buf,strlen(cmd_buf));
		open_data_connection(listen_fd,control_fd,cmd_buf,argv_from_m,server_addr);
		return 1;
	}

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
		char buf[2048] = { 0 };
		char cut[3] = " ";
	
		/* check if the ip is acceptable */
		n =	read(sockfd,buf,sizeof(buf)); //ACCEPTION OR REJECTION
		buf[n] = '\0';
		write(STDOUT_FILENO,buf,sizeof(buf));
		if(strcmp(strtok(buf,cut),"220")!=0){
			
			//print out prompt
			sprintf(for_output,"%s\n","** Connection refused **");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			exit(0);
		}
		
		//print out prompt
		sprintf(for_output,"%s\n","** Connected to sswlab.kw.ac.kr **");
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			
		memset(user,0x00,sizeof(user));
			
		/* pass username and password to server */
		sprintf(for_output,"%s","Input ID : ");
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
		n = read(STDIN_FILENO,user,MAX_BUF);
		user[n-1] = '\0'; //delete \n
		sprintf(for_output,"%s %s","USER",user);
		write(sockfd,for_output,MAX_BUF); memset(for_output,0x00,sizeof(for_output));
		n = read(sockfd,for_output,MAX_BUF); //reply comment from server
		for_output[n] = '\0';
		if(strcmp(strtok(for_output,cut),"331") != 0){
			write(STDOUT_FILENO,for_output,sizeof(for_output));
			exit(0);
		}
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	
		while(1){
			memset(buf,0x00,sizeof(buf));
			
			passwd = getpass("Input passwd: ");
			sprintf(for_output,"%s %s","PASS",passwd);
			write(sockfd,for_output,MAX_BUF); memset(for_output,0x00,sizeof(for_output));
			
			//getpass function is hiding password
			/* reaction at Server */
			n = read(sockfd,buf,MAX_BUF);
			buf[n] = '\0';
			write(STDOUT_FILENO,buf,sizeof(buf));
			if(!strcmp(strtok(buf,cut),"230")){
				// login success
				sprintf(for_output,"%s%s%s\n","** User `[",user,"]' logged in **");
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				return;
			}
			else if(!strcmp(strtok(buf,cut),"430")){
				// login fail
				sprintf(for_output,"%s (%d/3)","Log-in-failed",count);
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				count ++;
			}
			else if(!strcmp(strtok(buf,cut),"530")){
				/// buf is "DISCONNECTION"
				sprintf(for_output,"%s\n","** Connection closed **");
				write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				exit( 0 );
			}
			
		}
}
	   
	
/*	
	/*set client to be server
	listen_fd = socket(AF_INET, SOCK_STREAM, 0 );
	temp.sin_family = AF_INET; // set temp.sin_family
	temp.sin_addr.s_addr = inet_addr(argv[1]); //set temp.sin_addr.s_addr
	temp.sin_port = htons(random_port); //set temp.sin_port
	hostport = convert_addr_to_str(temp.sin_addr.s_addr, temp.sin_port); 	//eg) hostport = 127,0,0,1,48,57
	
	/*make PORT Instruction
	sprintf(buf_2,"%s %s","PORT",hostport);
	write(control_fd,buf_2,sizeof(buf_2)); //send PORT instruction to server

	/*200
	n = read(control_fd,for_output,sizeof(for_output)); //read reaction of PORT instruction from server
	for_output[n] = '\0';//to make string
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output)); //print out
	write(STDOUT_FILENO,"\n",1);	  
	write(control_fd,ftp_buf,sizeof(buf)); //send ftp command to server
	
	/*to be server
	bind(listen_fd,(struct sockaddr *)&temp,sizeof(temp));
	listen(listen_fd,5);
	len = sizeof(from_server_addr);
	data_fd = accept(listen_fd,(struct sockaddr *)&from_server_addr, &len);
	
	/*150//client to be server for data connection
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
	
	/* send 226
	sprintf(buf_L,"%s\n","226 Result is sent successfully");
	control_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set socket file descriptor	
	if(connect(control_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit(2); // connect with server to use socket file descriptor
	write(STDOUT_FILENO,buf_L,strlen(buf_L));
	write(control_fd,buf_L,strlen(buf_L));
	
	/*print OK
	sprintf(for_output,"%s%d%s\n","OK. ",len_of_result," bytes is received.");
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	return 0;  
	close(control_fd); //close server connect used by socket file descriptor
*/	   
