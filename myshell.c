#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>

#define MAX_LEN_LINE    40
#define LEN_HOSTNAME    30
#define LEN_CURRDIR     250
#define DELIM_CHARS    " ;"

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
        
		printf("[%s]\n", command);
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
