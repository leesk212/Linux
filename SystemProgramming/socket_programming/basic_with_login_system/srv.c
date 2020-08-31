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
#include <pwd.h>	//for fgetpwent() function

#define MAX_BUF 256
char for_output[256] = { 0 };
int log_auth(int connfd);
int user_match(char *user, char *passwd);
//////////////////////////////////////////////////////////////////////////
// print_out_regection													//
// =====================================================================//  
// Input : char * -> user , char * -> passwd  							//
// Purpose : Printout regection of IP									//
//////////////////////////////////////////////////////////////////////////
void print_out_regection(int connfd){
	sprintf(for_output,"%s\n","** It is NOT authenticated client **");
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	write(connfd,"REJECTION",9);
}
//////////////////////////////////////////////////////////////////////////
// main																	//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  								//
// Purpose : Get argument from stdout of stdout							//
//	     Do function by instruction										//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	char set_IP_frm_cli[MAX_BUF] = { 0 }; //set buf to buffsize
	char * set_IP_frm_txt;
	char cut[3] = " .\0";
	int n,len,offset; //set variable
	int server_fd, client_fd; //set variable server_fd and client_fd
	struct sockaddr_in server_addr, client_addr; //set sockaddr by internet version
	FILE *fp_checkIP;	
	server_fd = socket(PF_INET, SOCK_STREAM, 0 ); //set server file descriptor
	memset(&server_addr, 0, sizeof(server_addr)); //initialize server_addr to zero
	server_addr.sin_family = AF_INET; //IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //host IP to network version long 
	server_addr.sin_port = htons(atoi(argv[1])); //host Port to network version short
	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //bind socket_file descirptor with server_addr 
	listen(server_fd, 5); //can wait five other cli at one server
	
	while(1){
		int index = 0;
		char *ptr_frm_txt = NULL;
		char ptr_frm_cli[3][10] = { 0, };
		len = sizeof(client_addr); //set len 
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len); //make file_descriptor for connecting with other client
		//print out currnet client IP and Port number			
		sprintf(for_output,"%s\n%s\t%s\n%s\t%d\n","** Client is trying to connect **"
			   									 ,"- IP:",	inet_ntoa(client_addr.sin_addr)
												 ,"- Port:",ntohs(client_addr.sin_port));
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
		/*check IP*/
		//set IP from client
		strcpy(set_IP_frm_cli,inet_ntoa(client_addr.sin_addr));
		strcpy(ptr_frm_cli[0]  , strtok(set_IP_frm_cli,cut));
		strcpy(ptr_frm_cli[1]  , strtok(NULL,cut));
		strcpy(ptr_frm_cli[2] , strtok(NULL,cut));
		strcpy(ptr_frm_cli[3]  , strtok(NULL,cut));
	  	
		
		//set IP from access.txt;
		fp_checkIP = fopen("access.txt","r");
		fseek(fp_checkIP,0,SEEK_END); //check length of all of data
		len = ftell(fp_checkIP); //set length of all data to len
		set_IP_frm_txt = malloc(len + 1); //set buffer size for data
		memset(set_IP_frm_txt,0,len + 1); //set zero to buffer
		fseek(fp_checkIP,0,SEEK_SET); //return to file offset from first to read file
		fread(set_IP_frm_txt,len,1,fp_checkIP); //read file
		set_IP_frm_txt[len-1] = '\0'; //earse \n in text file 
		ptr_frm_txt  = strtok(set_IP_frm_txt,cut); //get first data
		
		//compare IP with txt and client
		if(ptr_frm_txt == NULL){
			print_out_regection(client_fd); //print out rejection of accessing client
			exit(1);
		}
		while(ptr_frm_txt != NULL){
			if(strcmp(ptr_frm_txt,"*")!= 0){ //if data is *, pass
				if(strcmp(ptr_frm_txt,ptr_frm_cli[index])!= 0){	//compare with client data
					print_out_regection(client_fd); //print out rejection of accessing client		
					exit(1);
				}	
			}
			ptr_frm_txt  = strtok(NULL,cut);//move to next txt data
			index ++;
		}
		
		//success to connect
		write(client_fd,"ACCEPTION",9);
		sprintf(for_output,"%s\n","** Client is connectted **");
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
		
		if(log_auth(client_fd) == 0){ //operate log_auth
			sprintf(for_output,"%s\n","** Fail to log - in **");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			close(client_fd);
			continue; //get other client
		}
		printf("** Success to log-in **\n");
		close(client_fd);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// log_auth																//
// =====================================================================//  
// Input : int -> connfd 					  							//
// Output : return 1 -> success(OK) , return -> 0 disconnecton			//
// Purpose : print out User trying to log-in count						//
//			 and send OK or FAIL or DISCONNECTION to client 			//
//////////////////////////////////////////////////////////////////////////
int log_auth(int connfd){	
	int n = 0 , count = 1;	
	while(1) {
		n = 0 ;
		char user[MAX_BUF] = { 0 };
		char passwd[MAX_BUF] = { 0 };
		fflush(stdout); fflush(stdin); //erase buffer of STDIN and STDOUT
		n = read(connfd,user,sizeof(user)); //set client user
		user[n] = '\0'; // make to string
		n = read(connfd,passwd,sizeof(passwd)); //set client password
		passwd[n] = '\0'; //make to string
		for(int i =0 ;i<5;i++)	
		sprintf(for_output,"%s%d%s\n","** User trying to log-in (",count,"/3) **");
		write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
				
		n = user_match(user,passwd); //compare user and passward with passwd file
		if(n == 1){	//success	
			write(connfd,"OK",2);
			return 1;
		}
		else if(n == 0){//fail
			if(count >= 3){//Disconnect
				write(connfd,"DISCONNECTION",13);
				return 0;
			}
			write(connfd,"FAIL",4);
			sprintf(for_output,"%s\n","** Log-in failed **");
			write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
			count++;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// user_match															//
// =====================================================================//  
// Input : char * -> user , char * -> passwd 					  		//
// Output : return 1 -> success , return -> 0 fail						//
// Purpose : match user and passwd with client user and passwd			//
//////////////////////////////////////////////////////////////////////////	
int user_match(char *user, char *passwd)
{
	FILE *fp = NULL;
	struct passwd *pw;	
	fp = fopen("passwd","r"); //set file discriptor with passwd file
	while((pw = fgetpwent(fp))!=NULL) { // if 0, finish to read
		if(!strcmp(pw->pw_name,user)){ //compare user with passwd file
			if(!strcmp(pw->pw_passwd,passwd)){ //compare passwd with passwdfile
				fclose(fp);//success
				return 1;	
			}
		}
	}
	//fail
	fseek(fp,0,SEEK_SET);//set offset at first
	fclose(fp);
	return 0;
}
