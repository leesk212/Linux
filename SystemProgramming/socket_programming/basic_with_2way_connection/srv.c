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

#include <sys/time.h>	//clock_t,time_t
#include <time.h>	//clock_t,time_t
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>    	 // user info
#include <grp.h> 	 // group info

#define MAX_BUF 256
char for_output[MAX_BUF] = { 0 };

/*ls function*/
int client_info(struct sockaddr_in *client_addr,int client_fd);
char * function_for_ls(int argc,char *argv[]);
char * Admin(unsigned long int t);
char ** print_out(int aflag,int lflag, char optopt,int argc);
int my_strcmp(char *name1, char *name2);
char * print_details(char * path);
char * chagne_to_zero_plus_nubmer(int number);
int cmd_process(char * buf,char *result_buf);


char * convert_str_to_addr(char *str,unsigned int *port);
//////////////////////////////////////////////////////////////////////////
// main																	//
// =====================================================================//  
// Input : char ** -> argv , int -> argc  								//
// Purpose : Get argument from stdout of stdout							//
//	     Do function by instruction										//
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
	char set_IP_frm_cli[MAX_BUF] = { 0 }; //set buf to buffsize
	char PORT_INST[1025] = { 0 };
	char result_buf[1024];
	char temp[25]; //FTP_Instruciton
	int a = 0;
	char * host_ip;
	
	unsigned int port_num;
	char * set_IP_frm_txt;
	char cut[3] = " .\0";
	
	int n,len,offset; //set variable
	int listen_fd,control_fd,data_fd; //set variable server_fd and client_fd
	
	struct sockaddr_in server_addr, client_addr; //set sockaddr by internet version
	struct sockaddr_in data_addr;
	memset(&server_addr, 0, sizeof(server_addr)); //initialize server_addr to zero
	memset(&data_addr, 0, sizeof(data_addr)); //initialize server_addr to zero
	
	listen_fd = socket(PF_INET, SOCK_STREAM, 0 ); //set server file descriptor
	server_addr.sin_family = AF_INET; //IPV4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //host IP to network version long 
	server_addr.sin_port = htons(atoi(argv[1])); //host Port to network version short
	
	bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //bind socket_file descirptor with server_addr 
	listen(listen_fd, 5); //can wait five other cli at one server	
	len = sizeof(client_addr); //set len 
	control_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len); //make file_descriptor for connecting with other client
	
	n = read(control_fd,PORT_INST,sizeof(PORT_INST)); //recieve PORT Instruction from client
	PORT_INST[n] = '\0'; //make to string
	write(STDOUT_FILENO,PORT_INST,sizeof(PORT_INST));
	write(STDOUT_FILENO,"\n",1);
	
	/*change port instruction to port and ip to being needed by us*/
	host_ip = convert_str_to_addr(PORT_INST,(unsigned int *)&port_num);
	//connect with client(changed to server) as data_connection
	
	/*200*/
	sprintf(for_output,"%s","200 Port command successful");
	write(control_fd,for_output,sizeof(for_output)); //send reaction to client
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	write(STDOUT_FILENO,"\n",1);
	
	/*NLST*/
	n = read(control_fd,for_output,sizeof(for_output)); //receive ftp instruction
	for_output[n] = '\0';
	strcpy(temp,for_output);
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	write(STDOUT_FILENO,"\n",1);
	
	/*150*/
	sprintf(for_output,"%s","150 Opening data connection for directory list");
	write(control_fd,for_output,sizeof(for_output)); //send reaction to client
	write(STDOUT_FILENO,for_output,sizeof(for_output)); memset(for_output,0x00,sizeof(for_output));
	write(STDOUT_FILENO,"\n",1);
	close(control_fd);
	
	/*make socket for data connection*/
	data_fd = socket(AF_INET, SOCK_STREAM, 0 ); //set server file descriptor
	data_addr.sin_family = AF_INET;
	data_addr.sin_addr.s_addr = inet_addr(host_ip);
	data_addr.sin_port = htons(port_num);
	
	/*connect with client by using other port*/
	connect(data_fd,(struct sockaddr *) &data_addr, sizeof(data_addr));
	
	//send result using by data_connection
	cmd_process(temp,result_buf);
	write(data_fd,result_buf,strlen(result_buf));
	close(data_fd);
	
	/*receive 226*/	
	control_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len); //make file_descriptor for connecting with other client
	n = read(control_fd,for_output,sizeof(for_output));
	for_output[n] = '\0';
	write(STDOUT_FILENO,for_output,sizeof(for_output));
	return 0;
	
}
//////////////////////////////////////////////////////////////////////////
// convert_str_to_addr													//
// =====================================================================//  
// Input :  -> ip_addr , unsigned int -> port 							//
// Purpose :  PORT instruction change to ip and port that we need	 	//
//////////////////////////////////////////////////////////////////////////
char * convert_str_to_addr(char *str,unsigned int *port){
	char * addr;
	char * ip_buf = (char * )malloc(1024);
	char port_buf[1024] = { 0 };
	char port_buf_1[10] = { 0 };
	char port_buf_2[10] = { 0 };
	char port_buf_c_b1[8] = { '0','0','0','0','0','0','0','0' };
	char port_buf_c_b2[8] = { '0','0','0','0','0','0','0','0' };
	char port_buf_c_b[16] = { 0 };
	char cut[4] = " ,.\0";
	int decimal_1,decimal_2;
	int index_1 = 7,index_2 = 7;
	
	strtok(str,cut);//remove PORT
	
	/*set ip with "."*/
	for(int i = 0; i<4; i++){
		strcat(ip_buf,strtok(NULL,cut)); //127	127.0	127.0.0	127.0.0.1
		if(i == 3) break;
		strcat(ip_buf,".");				 //127.	127.0.	127.0.0.
	}
	addr = ip_buf;
	
	/*port part 1 will be binary number */
	strcat(port_buf_1,strtok(NULL,cut));//pb1 = 150
	decimal_1 = atoi(port_buf_1);
	while(decimal_1!=0){
		if((decimal_1 % 2)==0) port_buf_c_b1[index_1] = '0';
		else port_buf_c_b1[index_1] = '1';
		index_1--;
		decimal_1 = decimal_1/2;	
	}
	/*port part 2 will be binary number */
	strcat(port_buf_2,strtok(NULL,cut));//pb2 = 48
	decimal_2 = atoi(port_buf_2);
	while(decimal_2!=0){
		if((decimal_2 % 2)==0) port_buf_c_b2[index_2] = '0';
		else port_buf_c_b2[index_2] = '1';
		index_2--;
		decimal_2 = decimal_2/2;	
	}
	/*assemble part 1 and part 2 of port*/
	sprintf(port_buf_c_b,"%s%s",port_buf_c_b1,port_buf_c_b2);	 //100101100110000
	/*change to decimal*/
	sprintf(port_buf,"%ld",strtol(port_buf_c_b,NULL,2)); //100101100110000->38448
	/*set port*/
	*port = (unsigned int)atoi(port_buf);
	
	return addr;
}
//////////////////////////////////////////////////////////////////////////
// cmd_process															//
// =====================================================================//  
// Input : char * -> buf , char * -> result_buf						  	//
// Output : int success/fail (0/-1)		  								//
// Purpose : set process result into result_buf						 	//
//////////////////////////////////////////////////////////////////////////
int cmd_process(char * buf,char *result_buf){
	char *ptr = NULL;
	int i,j,index = 0;
	char *free_ptr = NULL;	
	char ** value = (char **)malloc(sizeof(char *) * 1024); //make 2D array for ls
	for (i = 0; i<1024; i++) {
		value[i] = (char *)malloc(sizeof(char) * 1024);
		for(j =0; j<1024; j++) value[i][j] = 0; 
	}
	char cut[] = "\n "; //delimeter to using strtok
	char pre_return_buf[1024] = { 0 };
	char getStr[128] = { 0 };
	
	ptr = strtok(buf,cut); //delimete string to "  " or "\n"	
	while(ptr != NULL){//set value from setStr
		strcpy(value[index],ptr);
		index++;	
		ptr = strtok(NULL,cut);	
	}//end of while

	
	if( strcmp(value[0],"PWD") == 0 ){ //print current directory
			if (value[1][0] != 0) { //if pwd has argument
				write(STDERR_FILENO,"ERRP0",5); //print err
				return 0;			
			}//end of err if
			strcpy(result_buf,getcwd(getStr,128)); //print current directory
			strcat(result_buf,"\n");
		}//end of pwd if			
	if( strcmp(value[0],"CWD") == 0 || strcmp(value[0],"CDUP") == 0 ){
		if (value[1][0] == 0) { //if there is argument 
				write(STDERR_FILENO,"ERRC0",5); //print err
				return 0;			
			}//end of err if
			if (chdir(value[1]) == -1) {//start chdir
				write(STDERR_FILENO,"ERRC1",5); //print err
				return 0;			
			}//end of chdir if
			sprintf(pre_return_buf,"%s%s\n","Current working Directory: ",getcwd(getStr, 128));
			strcpy(result_buf,pre_return_buf);
	}//end of cwd, cdup if
	if( strcmp(value[0],"MKD") == 0 ){
			if (value[1][0] == 0) { // if there are not presented argument , 
				write(STDERR_FILENO,"ERRM0",5); //print err
				return 0;	
			}//end of err if
			for(i = 0; i<index-1; i++){
				if (mkdir(value[i+1], 0755) != 0) { //execute mkdir 
					write(STDERR_FILENO,"ERRM1",5); //print err
					return -1;
				} //end of mkdir err if
			}// end of for

	}//end of mkdir if
	if( strcmp(value[0],"RMD") == 0){ //Only remove directory
			struct stat state_data;
			if (value[1][0] == 0) { 
				write(STDERR_FILENO,"ERRR0",5); //print err
				return -1;
			}//end of argument err if
				
			for(i = 0; i<index-1; i++){		
				stat(value[i+1],&state_data); //set data_info
				if(S_ISREG(state_data.st_mode)){ // if file is regular file
					write(STDERR_FILENO,"ERRR1",5); //print err
					continue;
				}
				if(rmdir(value[i+1]) == -1){ // excute remove directory
					write(STDERR_FILENO,"ERRR2",5); //print err
				}	
			}
			
	}//end of RMD, DELE function if
	if( strcmp(value[0],"DELE") == 0){ //only remvoe regular_file
			struct stat state_data;			
			if (value[1][0] == 0) { 
				write(STDERR_FILENO,"ERRD2",0); //print err
				return -1;
			}//end of argument err if
				
			for(i = 0; i<index-1; i++){		
				stat(value[i+1],&state_data); //set data_info
				if(S_ISDIR(state_data.st_mode)){ //if file is directory file
					write(STDERR_FILENO,"ERRD1",5); //print err
					continue;
				}
				if(unlink(value[i+1]) == -1){ //excute remove regular file
					write(STDERR_FILENO,"ERRD2",5); //print err
				}	
			}
	}
	if( strcmp(value[0],"RNFR") == 0 ){
			if (rename(value[1], value[3]) == -1) { // if rename is fail
				write(STDERR_FILENO,"ERR_rename_2",10); //print err
				return -1;
			}//end of rename functoin if
	}//end of RNFR
	
	if( strcmp(value[0],"NLST") == 0 || strcmp(value[0],"LIST") == 0 ){// dir and ls
			if(strcmp(value[0],"LIST") == 0){//for dir function
				strcpy(value[1],"-al"); // change dir to dir -al
				index++;
			}//end of list
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
