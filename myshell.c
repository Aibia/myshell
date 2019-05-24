#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_LEN_LINE    40
#define LEN_HOSTNAME    30
#define LEN_CURRDIR     250
#define DELIM_CHARS    " ;"

void ls_Inode(struct stat buf){
	printf("%d    ", (unsigned int)buf.st_ino);
}

void ls_Mode(struct stat buf){
	printf("%o    ", (unsigned int)buf.st_mode);
}

void ls_FSize(struct stat buf){
	printf("%d    ", (int)buf.st_size);
}

void ls_option(struct stat buf, char *option){
	if(strcmp(option, "-l")==0){
		ls_Mode(buf);
		ls_Inode(buf);
		ls_FSize(buf);
	}
	else if(strcmp(option, "-i")==0){
		ls_Inode(buf);
	}
}

int ls(char *argv);
// show files and directories in current directory

int main(void)
{
    char command[MAX_LEN_LINE];
    char *args[] = {command, NULL};
    int ret, status;
    pid_t pid, cpid;
    char hostname[LEN_HOSTNAME + 1];
    char *currDir;
	char *ret_ptr;
	char *next_ptr;
    	
	memset(hostname, 0x00, sizeof(hostname));
    gethostname(hostname, LEN_HOSTNAME);
	currDir = getcwd(NULL, LEN_CURRDIR);

    while (true) {
        char *s;
        int len;
        
        printf("%s@%s:%s$ ", getpwuid(getuid())->pw_name, hostname, currDir);
        
        s = fgets(command, MAX_LEN_LINE, stdin);
        
		if (s == NULL) {
            fprintf(stderr, "fgets failed\n");
            exit(1);
        }

        len = strlen(command);
        
		if (command[len - 1] == '\n') {
            command[len - 1] = '\0'; 
        }
		if (!strcmp(command, "exit")){
			return 0;
		}
		if (!strncmp(command, "ls", 2)){
			ls(command);
		}

		ret_ptr = strtok_r(command, DELIM_CHARS, &next_ptr);

		while (ret_ptr){
	        pid = fork();
        
			if (pid < 0) {
        	    fprintf(stderr, "fork failed\n");
            	exit(1);
        	}
			if (pid != 0) {  /* parent */
            	cpid = waitpid(pid, &status, 0);
           		if (cpid != pid) {
            	    fprintf(stderr, "waitpid failed\n");        
           	 	}
            	printf("Child process terminated\n");
            	if (WIFEXITED(status)) {
                	printf("Exit status is %d\n", WEXITSTATUS(status)); 
            	}
        	}
        	else {  /* child */
            	args[0] = ret_ptr;
				ret = execve(args[0], args, NULL);
            	if (ret < 0) {
                	fprintf(stderr, "execve failed\n");   
                	return 1;
            	}
        	}
			ret_ptr = strtok_r(NULL, DELIM_CHARS, &next_ptr);
    	}
    }
	return 0;
}

int ls(char *argv){
	char *cwd = (char *)malloc(sizeof(char)* 1024);
	memset(cwd, 0, 1024);

	DIR *dir = NULL;
	struct dirent *entry;
	struct stat buf;

	getcwd(cwd, 1024);

	if ((dir = opendir(cwd))==NULL){
		printf("opendir() error\n");
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL){
		lstat(entry->d_name, &buf);
		
		if (S_ISREG(buf.st_mode))
			printf("FILE ");
		else if(S_ISDIR(buf.st_mode))
			printf("DIR  ");
		else
			printf("???  ");
		if (strlen(argv) > 1)
			ls_option (buf, (argv+1));
		
		printf("%s \n", entry->d_name);
	}

	free(cwd);
	closedir(dir);

	return 0;
}