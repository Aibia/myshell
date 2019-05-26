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
#include <grp.h>
#include <time.h>

#define MAX_LEN_LINE    40
#define LEN_HOSTNAME    30
#define LEN_CURRDIR     250
#define DELIM_CHARS    " ;"

void rwx(mode_t file_mode);

int ls(char *arg);
// show files and directories in current directory

void ls_option(struct stat buf, char *option);

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
            ls(command+2);
        }
        // ls를 입력받은 경우 option에 따라 현재 디렉토리, 파일들의 권한, 사이즈, 내용 출력
        if (!strcmp(command, "pwd")){
            printf("%s\n", currDir);
        }
        // pwd를 입력받은 경우 현재 경로 출력

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

int ls(char *arg){
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
        
        if (strlen(arg) > 1)
            ls_option (buf, (arg+1));
        
        printf("%s \n", entry->d_name);
    }

    free(cwd);
    closedir(dir);

    return 0;
}

void rwx(mode_t file_mode){
    //파일의 종류와 접근권한 표시
    if (S_ISDIR(file_mode)){
        printf("d");
    }
    else
        printf("-");

    if (file_mode & S_IRUSR){
        printf("r");
    }
    else
        printf("-");
    
    if (file_mode & S_IWUSR){
        printf("w");
    }
    else
        printf("-");
    
    if (file_mode & S_IXUSR){
        printf("x");
    }
    else
        printf("-"); 
    
    if (file_mode & S_IRGRP){
        printf("r");
    }
    else
        printf("-");
    
    if (file_mode & S_IWGRP){
        printf("w");
    }
    else
        printf("-");
    
    if(file_mode & S_IXGRP){
        printf("x");
    }
    else
        printf("-"); 
    
    if (file_mode & S_IROTH){
        printf("r");
    }
    else
        printf("-");
    
    if (file_mode & S_IWOTH){
        printf("w");
    }
    else
        printf("-");
    
    if(file_mode & S_IXOTH){
        printf("x");
    }
    else
        printf("-");

    printf(" ");
}

void ls_option(struct stat buf, char *option){
    char time_str[26];

    if(strcmp(option, "-l")==0){
        rwx(buf.st_mode);
        printf(" %ld  ", buf.st_nlink);
        //링크 수 출력
        printf("%s", getpwuid(getuid())->pw_name);
        //소유자 출력
        printf("  %s  ", getgrgid(getgid())->gr_name);
        //group 출력
        printf("%ld  ", buf.st_size); 
        //크기 출력
        ctime_r(&buf.st_mtime, time_str);
        if (time_str[24] == '\n') {
            time_str[24] = '\0'; 
        }
        printf("%s    ", time_str);
        //수정된 시간 출력
    }
    else if(strcmp(option, "-i")==0){
        printf("%d    ", (unsigned int)buf.st_ino);
    }
}