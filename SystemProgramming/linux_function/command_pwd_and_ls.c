
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
void print_pwd(char* pathname){
	DIR * dir = opendir(pathname); //use opendir to open dir path in current path
	struct dirent * dp; 		  //to contain dirent type
	while(dp = readdir(dir)){ 	  //read current path directory to step by step 
		if(dp->d_ino != 0)   //if d_ino == 0, file or directory is deleted
			printf("%s\n",dp->d_name); //print directory and file name
	} //end of while 
	closedir(dir); //closed current path directory
} //end of 
void print_recursive(char* pathname, int tab_cnt){
	DIR * dir = opendir(pathname); //open current directory
	struct dirent * dp;  	  //to contain dirent type
	char newpath[100] = "/0";	  //to set next path in program	
	int i=0;			  //for printing tab_count easily 	
	if(!dir) return;		  //if there are error to open directory finish function
	chdir(pathname);		  //change directory to newpath
	while(dp = readdir(dir)){	  //read current path directory to step by step
		if(dp->d_ino != 0){   //if d_ino == 0, file or directory is deleted
			for(i=0; i<tab_cnt; i++) //print tab
				printf("    ");
			printf("%s\n",dp->d_name);
			if(dp->d_ino > 400000){ //if file is directory, move to sub directory 
				if(strncmp(dp->d_name, ".", 1)){ //if file name is ‘.’ or ‘..’ , don’t move
					strcpy(newpath,dp->d_name); //make new path		 			
					print_recursive(newpath,tab_cnt+1); //recursive function	
				} //end of third if					
			} // end of second if
		} //end of first if
	} //end of while
	chdir(".."); //return to parent’s directory
	closedir(dir); //finish current directory
}
int main() { //main function

	// don't touch this section.
	printf("<FTP_Pratice1 No.1>\n"); 
	print_pwd(".");
	printf("=============================\n");
	printf("<FTP_Pratice1 No.2>\n"); 
	print_recursive(".", 0);
	return 0;
}
