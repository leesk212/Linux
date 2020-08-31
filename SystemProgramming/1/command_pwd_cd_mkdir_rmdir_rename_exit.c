#include <unistd.h> // for chdir(), getcwd(), rmdir()
#include <stdio.h> //for rename()
#include <sys/stat.h> //for mkdir()
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>


int main(int argc, char **argv){ //input value: argv ,count of input value: argc 


	char getStr[128]; 
	char setStr[128];
	char *ptr;
	int exit_flag=0;
	int rename_err_flag=0;

	fgets(setStr,sizeof(setStr),stdin); //set all input value with spacing.
	setStr[strlen(setStr)-1] ='\0'; // last value is '\n' in fgets input function, so we have to change to '\0'
	
	ptr = strtok(setStr, " "); //set next value
	if (strcmp(ptr, "exit")==0) { // if first value is exit
		exit_flag++;
		ptr = strtok(NULL, " ");
		if (ptr!=NULL) { //even though first value is exit, the case that has other argument is err
			exit_flag++;
			printf("err\n");
		}

	}
	while(exit_flag!=1){ //if input value is exit, finish while loop
		switch (ptr[0]){
			case 'c'://chdir function to make "cd" 
				ptr = strtok(NULL, " ");
				if(ptr==NULL){ printf("err\n"); break; }
				if( chdir(ptr) == -1 ) { printf("err\n"); break; }
				
				break;
			case 'p'://getcwd function to make "pwd"
				ptr = strtok(NULL, " ");
				if(ptr!=NULL){ printf("err\n"); break; }
				getcwd(getStr,128); 			
 				printf("%s\n",getStr);
			
				break;

			case 'm'://mkdcir function to make "mkdir"
			{	
				mode_t temp = 0755; 				    //default directory authority mode is 0755
				int i,check = 0;
				
				// to make -m option // making 2D array	
				char ** value = (char **) malloc (sizeof(char *) * 1024); 
				for(i=0; i<1024; i++){				
					value[i] = (char *) malloc(sizeof(char) * 1024);				
				}
	
				ptr = strtok(NULL, " "); 			    //get next value
				if(ptr==NULL){ printf("errA\n"); break; } 	    //if there are not argument, err
				while(ptr != NULL){ 
					//to make -m option
					if( strcmp(ptr,"-m") == 0 ){ 		    //when mkdir -m 0777 A B
						char * for_mode = strtok(NULL, " ");//set mode value
						if(for_mode == NULL) { printf("There is not mode type value\n"); break; }
						temp = strtoul(for_mode,NULL,8);    //change char * to Octal number
						if(check != 0){ 		    //when mkdir A B -m 0777
							for(i =0; i<check; i++){    //change all directory's mode
								chmod(value[i],temp);
							}// end of for
							break;
						}//end of if(
						ptr = strtok(NULL," "); 	    //set directory name
					}//end of if
					if(mkdir(ptr,temp) != 0) { 		    //make new directory with mode
						printf("errB\n"); 
						break; 
					}//end of if
					value[check] = ptr; 			    //to make -m option
					check++;	    	
					ptr = strtok(NULL, " ");	            //set next value
				}//end of while
				free(value);				
				break;
			}
			case 'r':
				if(ptr[1] == 'm'){ //rmdir function to make "rmdir"
					ptr = strtok(NULL, " ");
					if(ptr==NULL){ printf("err\n"); break; }
					while(ptr != NULL){
						if( rmdir(ptr) == -1 ) {
							 printf("err\n");
							 break;
					 	}
						ptr = strtok(NULL, " ");
					 }
				}
				else if(ptr[1] == 'e'){ //rename function to make "rename"
					char * old_file = strtok(NULL," "); //original value
					if(old_file == NULL){ printf("err\n"); break; }
					char * new_file = strtok(NULL," "); //change value
					if(new_file == NULL){ printf("err\n"); break; }
					ptr = strtok(NULL," ");
					if(ptr != NULL ) {
						 printf("err\n"); 
						 break; 
					}
					if(rename(old_file,new_file) == -1) {
						 printf("err\n"); 
						 break; 
					}
				}
				break;
			case 'e':
				exit_flag = 1;
				break;

			default:
				printf("It is not correct instruction\n");
				break;
				
		}

		if(exit_flag != 1){ // if instruction is not exit
			fgets(setStr,sizeof(setStr),stdin); //set all input value with spacing.
			setStr[strlen(setStr)-1] ='\0';
			ptr = strtok(setStr," ");
		}
		
	}

	return 0;

}
