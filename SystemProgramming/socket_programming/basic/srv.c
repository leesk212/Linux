///////////////////////////////////////////////////////////////////////
// File Name : srv.c 						     //
// Date : 2020/05/18 						     //
// Os : Ubuntu 12.04 LTS 64bits 				     //
// Author : Lee Suk Min						     //
// Student ID : 2016722013 					     //
// ----------------------------------------------------------------- //
// Title : System Programming Practice #2-1 		             //
// Description : set srv command & functioned value     	     //
///////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h> 	 // diretory function
#include <pwd.h>    	 // user info
#include <grp.h> 	 // group info
#include <dirent.h> 	 // directory
#include <errno.h> 	 // error info
#include <sys/stat.h> 	 // stat info
#include <time.h> 	 // time info
#include <string.h> 	 // string function

#include <sys/types.h>   //socket()
#include <sys/socket.h>  //connect() socket()
#include <netinet/in.h>  //inet_adder()
#include <arpa/inet.h>	 //htonl , htons , ntohl , ntohs

#define BUFFSIZE	1024

int client_info(struct sockaddr_in *client_addr,int client_fd);
char * function_for_ls(int argc,char *argv[]);
char * Admin(unsigned long int t);
char ** print_out(int aflag,int lflag, char optopt,int argc);
int my_strcmp(char *name1, char *name2);
char * print_details(char * path);
char * chagne_to_zero_plus_nubmer(int number);

//////////////////////////////////////////////////////////////////////////
// main									//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  				//
// Purpose : Get argument from stdout of stdout				//
//	     Do function by instruction					//
//	     Set 2D value array like argv		 	 	//
//////////////////////////////////////////////////////////////////////////
int main(int argc,char **argv){
	
	struct sockaddr_in server_addr, client_addr; //make two different struct one is server for socket, the other one is client for socket
	int socket_fd, client_fd; //socket file descripter and client file descripter
	int i,len, len_out;
	char buf[BUFFSIZE];	//original value
	char result_buf[BUFFSIZE]; //result value
	char PORTNO[1024] = { 0 };//changable port number
	
	if( (socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) { //make socket file descripter
		
			write(STDOUT_FILENO, "ERR D\n", 6 );//print prompt("ERR D\n");
		return -1;
	}
	
	bzero( (char*)&server_addr, sizeof(server_addr)); //intialize struct server_addr
	bzero(buf,sizeof(buf) ); //initailize buf
	bzero(result_buf,sizeof(result_buf) ); //initailize result_buf
	
	strcpy(PORTNO,argv[1]); //set changable port number

	server_addr.sin_family = AF_INET; //set family
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //set ip
	server_addr.sin_port = htons(atoi(PORTNO)); //set port number
	
	//bind with socket_file descripter and server setting
	if( bind(socket_fd, (struct sockaddr *)&server_addr,sizeof(server_addr) ) < 0){
		
			write(STDOUT_FILENO, "ERR E\n", 6 );//print prompt("ERR E\n");
		return -1;
	}
	listen(socket_fd,5); //server can wait 5 people's aquirement

	while(1){
		len = sizeof(client_addr);
		//connect socket with client
		if( (client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &len)) < 0){
			
			write(STDOUT_FILENO, "ERR F\n", 6 );//print prompt("ERR F\n");
			return -1;
		}
		if( client_info(&client_addr,client_fd) < 0 ) //printout client information
			
			write(STDOUT_FILENO, "ERR G\n", 6 );//print prompt("ERR G\n");
		while(1){
			len_out = read(client_fd,buf,sizeof(buf)); //set buf from client command
			buf[len_out] = '\0'; //make to string
			write(STDOUT_FILENO,buf,sizeof(buf)); //write buf into standard output 
			write(STDOUT_FILENO,"\n",1); //write "\n" into standard output

			if(cmd_process(buf,result_buf)<0) { //excute command
				write(STDOUT_FILENO, "ERR H\n", 6 );//print prompt("ERR H\n"); // excute process
			}
			write(client_fd, result_buf, sizeof(result_buf)); //write result_buff to socket
			if(!strncmp(result_buf,"QUIT",4)){ //if you waant to finish program, command is QUIT
//				write(STDOUT_FILENO,"QUIT\n",5); //printout prompt
				close(client_fd);//close client_file descripter
				close(socket_fd);//close socket_file descirpter
				return 0;
			}
			bzero(buf,sizeof(buf) ); //initailize buf
			bzero(result_buf,sizeof(result_buf) ); //initailize buf
			rewind(stdout); rewind(stdin); rewind(stderr); //initialzie standard input output error
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// client_info								//
// =====================================================================//  
// Input : struct sockaddr_in * -> client_ddr , int -> client_fd  	//
// Output : int success/fail (0/-1)		  			//
// Purpose : Get client data from main function 			//
//	     print out client info	 				//
//////////////////////////////////////////////////////////////////////////
int client_info(struct sockaddr_in * client_addr,int client_fd){
	if(client_fd<0) return -1; //if client_fd is little than 0, there is error to open socket 
	printf("===========Client info===========\n"); //print prompt
	printf("client IP: %s\n\n",inet_ntoa(client_addr->sin_addr));// use inet_ntoa to see ip nubmer to host
	printf("client port: %d\n",ntohs(client_addr->sin_port)); // use ntohs to see port number to host
	printf("=================================\n");
	return 0; //success
}
//////////////////////////////////////////////////////////////////////////
// cmd_process								//
// =====================================================================//  
// Input : char * -> buf , char * -> result_buf			  	//
// Output : int success/fail (0/-1)		  			//
// Purpose : set process result into result_buf			 	//
//////////////////////////////////////////////////////////////////////////
int cmd_process(char * buf,char *result_buf){
	char *ptr = NULL;
	int i,j,index = 1;
	char *free_ptr = NULL;	
	char ** value = (char **)malloc(sizeof(char *) * 1024); //make 2D array for ls
	for (i = 0; i<1024; i++) {
		value[i] = (char *)malloc(sizeof(char) * 1024);
		for(j =0; j<1024; j++) value[i][j] = 0; 
	}
	if(buf[0] != 'N')//if there are not cmd that I had make
		if(buf[0] != 'Q')
			return -1;

	if(!strncmp(buf,"QUIT",4)){ //if cmd is QUIT, 
		strcpy(result_buf,"QUIT"); //do QUIT
	}
	else{		
		ptr = strtok(buf," ");	//get after token value	
		strcpy(value[0],ptr);	//set NLST to value[0]				
		ptr = strtok(NULL, " ");			
		if(ptr!=NULL){ //ls -l, ls -a, ls -al		
			strcpy(value[index],ptr);	//set -al,-l to value[1]
			index++;
		}
		free_ptr = function_for_ls(index,value);//setting result_buf
		strcpy(result_buf,free_ptr); //eventaully finish to excute ls , setting result_buf
		free(free_ptr);//delete memory to not use
	}	
	for(i = 0; i<1024; i++)
		free(value[i]);//delete memory to not use
	free(value); //delete memory to not use
	return 0;
}
//////////////////////////////////////////////////////////////////////////
// function_for_ls							//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  				//
// Output : char * (return result 1D value of ls) 		  	//
// Purpose : Get argument from main 					//
//	     parsing argument by option	 				//
//////////////////////////////////////////////////////////////////////////
char * function_for_ls(int argc,char *argv[])
{
	int aflag = 0; int lflag =0; // initialize flag
	int c = 0; //for getopt function
	char *rest_data = NULL; //argument
	char *return_value = (char *)malloc(9999); //make 1D return value
	char **pre_value=NULL; //for pre_value
	char * free_ptr = NULL; //for deleting memory to not use
	int index = 1; //for index
	if(argv[1] == NULL) aflag = 0,lflag =0; //ls or dir
	else{
		while ((c = getopt(argc, argv, "al")) != -1) //do getopt function
		{
			switch (c) //return value
			{
			case 'a': 
				aflag++; //increase aflag
				break;
			case 'l': 
				lflag++; //increas lflag
				break;
			case'?': //there is not -a and -l
				rest_data = optarg; // call the character after it.
				break;
			} 
		}
	}
	if(optopt == '?') optopt = 0; //change optopt to 0
	
	optind = 0; //initialize flag while doing program

	pre_value = print_out(aflag,lflag,optopt, argc); //set value to pre_value 2D
	strcpy(return_value, pre_value[0]); //set first value
	free(pre_value[0]); //for deleting memory to not use
	while(pre_value[index][0] != 0){ //2D->1D and set type of example result
		if(lflag == 0 ) strcat(return_value, "\n"); //set type of example result
		strcat(return_value,pre_value[index]); //set 2D->1D
		free_ptr = pre_value[index]; //for deleting memory to not use
		free(free_ptr); //delete memory to not use
		index++; //make high to index
	}
	if(lflag == 0 ) strcat(return_value, "\n"); //set type of example result
	free(pre_value);
	return return_value;		
}


//////////////////////////////////////////////////////////////////////////
// print_out								//
// =====================================================================//  
// Input : char * -> rest_data , int -> aflag, int -> lflag , 		//
//	   char -> optopt , int -> argc  				//
// Output : char ** (return result 2D value of ls) 		  	//
// Purpose : Get argument from main 					//
//	     parsing argument by option					//
// 	     print out value related by ls option		 	//
//////////////////////////////////////////////////////////////////////////
char ** print_out(int aflag,int lflag, char optopt, int argc)
{
	struct stat state_data; //for storing state
	DIR *dirp; // for storing directory data
	struct dirent *dir; // for storing dir data
	int i,j,num = 0; //for using for statement and index
	char * free_ptr = NULL;

	char ** file_name = (char **)malloc(sizeof(char *) * 1024); //make 2D array in memory
	for (i = 0; i<1024; i++) {
		file_name[i] = (char *)malloc(sizeof(char) * 1024);
		for(j =0; j<1024; j++) file_name[i][j] = 0; 
	}
	char ** return_value = (char **)malloc(sizeof(char *) * 1024); //make 2D array in memory
	for (i = 0; i<1024; i++) {
		return_value[i] = (char *)malloc(sizeof(char) * 1024);
		for(j =0; j<1024; j++) return_value[i][j] = 0; 
	}
	char *cwd = getcwd(NULL, 1024); // get current dir in cwd
		
	dirp = opendir(cwd); // save changed directory in DIR struct.
	stat(cwd, &state_data); // call current directory

	while ((dir = readdir(dirp)) != NULL){ // until dir is ended
		strcpy(file_name[num], dir->d_name); // copy filename in file_name variable
		num++;
	}
	closedir(dirp); // close directory

	// sort file file_name using bubble sort
	char temp[1024] = { 0 }; // using temporary for bubble sort	
	for (i = 0; i < num; i++){ // use bubble sort
		for (j = 0; j < num - i - 1; j++){
			if (my_strcmp(file_name[j], file_name[j + 1]) == 1){ // if file_name[j] is bigger, execute
				strcpy(temp, file_name[j]); // temp <- file_name[j]
				strcpy(file_name[j], file_name[j + 1]); // file_name[j] <- file_name[j+1]
				strcpy(file_name[j + 1], temp); // file_name[j+1] <- temp
			}
		}
	}

	if (lflag == 0){ // if l is not called -> do not print in details
		if(aflag == 0){//ls		
			for (i = 2; i < num; i++){//only print file_name 
				strcpy(return_value[i-2],file_name[i]); //set data to return value
				free_ptr = file_name[i];//for deleting memory to not use
				free(free_ptr);//delete memory to not use
			}
		}
		else{//ls -a
			for (i = 0; i < num; i++){//only print file_name 
				strcpy(return_value[i],file_name[i]);//set data to return value
				free_ptr = file_name[i]; //for deleting memory to not use
				free(free_ptr);//delete memory to not use
			}
		}
	}
	else // print in details
	{
		if(aflag == 0){//ls -l		
			for (i = 2; i < num; i++){//only print file_name 
				strcpy(return_value[i-2],print_details(file_name[i]) ); //set data to return value
				free_ptr = print_details(file_name[i]); //for deleting memory to not use
				free(free_ptr);//delete memory to not use
				free_ptr = file_name[i]; //for deleting memory to not use
				free(free_ptr);	//delete memory to not use	
			}
		}
		else{//ls -al
			for (i = 0; i < num; i++){//only print file_name 
				strcpy(return_value[i],print_details(file_name[i]) ); //set data to return value
				free_ptr = print_details(file_name[i]); //for deleting memory to not use
				free(free_ptr);//delete memory to not use
				free_ptr = file_name[i]; //for deleting memory to not use
				free(free_ptr);	//delete memory to not use	
			}
		}
	}
	free(file_name);//delete memory to not use
	return return_value;
}

//////////////////////////////////////////////////////////////////////////
// print_detail								//
// =====================================================================//  
// Input : char * -> path , int -> not_print_dir, 		  	//
//	   struct stat -> state_data  					//
// Output : char * (return result value of detail of path) 		//
// Purpose : print detail of file 					//
//	     if file is dir, print out by not_print dir variable	//
//////////////////////////////////////////////////////////////////////////
char * print_details(char * path){
	struct stat state_data;//for getting detail data of state
	struct tm *t; // set variable that print time
	struct passwd *pw; // contain user struct
	struct group *gr; // contain group struct
	char * return_data = (char *)malloc(1024);//for return
	char * free_ptr = NULL; //for deleting memory
	char data[1024] = { 0 }; //for gathering data
	stat(path,&state_data); //set detial data of path

	strcpy(return_data,Admin((unsigned long)state_data.st_mode)); // print file permission
	free_ptr = Admin((unsigned long)state_data.st_mode); //delete memory to not use
	free(free_ptr); //delete memory to not use
	strcat(return_data," ");
		
	sprintf(data,"%ld ", (long)state_data.st_nlink); // set number of link to return value
	strcat(return_data,data); memset(data, 0x00, 1024);
		
	pw = getpwuid(state_data.st_uid); // set user info into pw
	strcat(return_data,pw->pw_name); // set user_name to return value
	strcat(return_data," ");

	gr = getgrgid(state_data.st_gid); // set group info into gr			
	strcat(return_data,gr->gr_name); // set group_name to return value
	strcat(return_data," ");
		
	sprintf(data,"%5lld ", (long long)state_data.st_size); //set data size ti return value
	strcat(return_data,data); memset(data, 0x00, 1024); 

	t = localtime(&state_data.st_mtime); // get last modification time
	sprintf(data,"%2d%s %3d%s %s%s%s\t", t->tm_mon+1, "월", t->tm_mday,"일", chagne_to_zero_plus_nubmer(t->tm_hour),":",chagne_to_zero_plus_nubmer(t->tm_min)); //set time data to return value
	strcat(return_data,data); memset(data, 0x00, 1024);
	
	sprintf(data,"%s\n",path);//set path data to return value
	strcat(return_data,data); memset(data, 0x00, 1024);
	return return_data;

}
//////////////////////////////////////////////////////////////////////////
// chagne_to_zero_plus_nubmer						//
// =====================================================================//  
// Input : int -> number						//
// Output : char * (change of number or origin nubmer)		  	//
// Purpose : change number to zero + number if number is under 10	//
//////////////////////////////////////////////////////////////////////////
char * chagne_to_zero_plus_nubmer(int number){//5 -> 05
	char * return_value = (char *)malloc(10);	
	if(number<10){//if nubmer is under 10
		sprintf(return_value,"%d%d",0,number); //plus 0 in front of number
	}
	else{//if number is not nunder 10
		sprintf(return_value, "%d", number); //return origin value
	}
	return return_value;
}


//////////////////////////////////////////////////////////////////////////
// Admin								//
// =====================================================================//  
// Input : unsigned long int -> t  					//
// Output : char * (return result value of detail of Admin of path) 	//
// Purpose : print detail of admin of file				//
//////////////////////////////////////////////////////////////////////////
char * Admin(unsigned long int t)
{
	int i;//for for statement
	char * data = (char *)malloc(11); //make memory in processor
	memset(data, 0x00, 11); //intialize to zero
	int mode[3]; // three mode number

	if (t > 8 * 8 * 8 * 8 * 8) data[0] = '-'; // t is not directory
	else	data[0] = 'd'; //it is directory
	
	t = t % (8 * 8 * 8); // set distinct file mode number

	for (i = 0; i < 3; i++){ // insert three mode number(user, group, other) 
		mode[2 - i] = t % 8; //set user/group/other 
		t /= 8; //set number
	}
	for (i = 0; i < 3; i++){ 
		if (mode[i] >= 4){ // if number is bigger than 4 -> readable
			data[3*i+1] = 'r'; 
			mode[i] -= 4; 
		}
		else data[3*i+1] = '-'; 
		if (mode[i] >= 2){ // if number is bigger than 2 -> writable
			data[3*i+2] = 'w'; 
			mode[i] -= 2; 
		}
		else data[3*i+2] = '-'; 
		if (mode[i] >= 1){ // if number is bigger than 1 -> executable
			data[3*i+3] = 'x'; 
			mode[i] -= 1; 
		}
		else data[3*i+3] = '-'; // not executable
	}
	
	return data;
}

//////////////////////////////////////////////////////////////////////////
// my_strcmp								//
// =====================================================================//  
// Input : char * -> name1, char * -> name2	 		  	//  		
// Output : 0 (first_name == second_name), -1 (first_name > second_name)//	
//	    , 1 (first_name < second_name)				//
// Purpose : compare string name1 and name2 ignored Alphametic		//
//////////////////////////////////////////////////////////////////////////
int my_strcmp(char *name_1, char *name_2)
{
	char first_name[1024], second_name[1024]; // use another variable for conserving name1, name2 value
	int i,j;
	strcpy(first_name, name_1); // initialize first_name
	strcpy(second_name, name_2); // initialize second_name

	for (i = 0; first_name[i] != 0; i++) // change upper case to lower case
		if (64 < first_name[i] && first_name[i] < 91) // if upper case
			first_name[i] += 32; // change alphabet using ascII code

	for (i = 0; second_name[i] != 0; i++) // change upper case to lower case
		if (64 < second_name[i] && second_name[i] < 91) // if upper case
			second_name[i] += 32; // change alphabe tusing ascII code

	//compare first_name, second_name using strcmp
	if (strcmp(first_name, second_name) < 0) // if first_name > second_name
		return -1; // return -1

	else if (strcmp(first_name, second_name) > 0) // if first_name < second_name
		return 1; // return 1

	return 0; // else return 0
}
