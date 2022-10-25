#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <utmp.h>

bool isLogged = false;

bool isLoginCommand(char* str){
    if(strlen(str) >= 9){
        char s[80];
        strcpy(s, str);
        s[8] = '\0';

        if(strcmp(s, "login : ") == 0)
            return true;
    }
    return false;
}

bool isGetProcCommand(char* str){
    if(strlen(str) >= 17){
        char s[80];
        strcpy(s, str);
        s[16] = '\0';

        if(strcmp(s, "get-proc-info : ") == 0)
            return true;
    }
    return false;
}

void intToChar(int x, char* str){
    int x1 = x;
    int range = 0;
    while(x1){
        x1 /= 10;
        range++;
    }
    str[range] = '\0';
    while(x){
        str[--range] = x%10 + 48;
        x /= 10;
    }
}

void utmpAppend(char* str, struct utmp *log) {
    
    // printf("\n ut_user: %s \n ut_host: %s \n ut_tv: %d \n", log->ut_user, log->ut_host, log->ut_tv.tv_sec);

    strcat(str, "\n username: ");
    strcat(str, log->ut_user);

    strcat(str, " \n hostname: ");
    strcat(str, log->ut_host);

    char tmp[100];
    tmp[0] = '\0';
    intToChar(log->ut_tv.tv_sec, tmp);

    strcat(str, " \n entry time: ");
    strcat(str, tmp);
    strcat(str, " seconds");

    strcat(str, " \n");

}

void getfirst4ch(char* first4ch, char* str){
    for(int i=0; i<4; i++){
        first4ch[i] = str[i];
    }
    first4ch[4] = '\0';
}

char* handleCommand(char* str){

    char *result = (char *) malloc(512);

    strcpy(result, "Wrong command. Use 'help' for clarification");

    if(isLoginCommand(str)){
        
        char username[20];
        strcpy(username, str + 8);
        FILE* fp = fopen("users.txt", "r");

        if(!fp){
            strcpy(result, "Error opening file");
        }
        char buf[20];
        while(fscanf(fp, "%s", buf) != EOF){

            if(strcmp(username, buf) == 0){

                if(isLogged){

                    strcpy(result, "Already logged in!");
                    break;
                }

                else{
                    strcpy(result, "Logged in!");
                    isLogged = true;
                    break;
                }
            }
        }

        if(!isLogged)
            strcpy(result, "Wrong username!");
    }

    if(strcmp(str, "logout") == 0){
        if(isLogged)
            strcpy(result, "Logged out!");
        else
            strcpy(result, "Already logged out!");

        isLogged = false;
    }

    if(strcmp(str, "get-logged-users") == 0){

        if (!isLogged){
            strcpy(result, "You have to login first!");
            return result;
        }

        int file;

        struct utmp log[1];
        int i = 0;
        char res[490];
        res[0] = '\0';

        file = open("/var/run/utmp", O_RDONLY);  

        if( file < 0 ) {  
           strcpy(result, "Failed to open utmp file");
           return result;
        }

        if (file) {

                while(read(file, log, sizeof(log)) != 0){
                    
                    utmpAppend(res, log);
                }

                close(file);
        }

        strcpy(result, res);
    }
    
    if(isGetProcCommand(str)){
        
        if (!isLogged){
            strcpy(result, "You have to login first!");
            return result;
        }
        char pid[8];
        strcpy(pid, str + 16);

        char path[16] = "/proc/";
        strcat(path, pid);
        strcat(path, "/status");
        FILE *pf;
        pf = fopen(path, "r");
        if(pf == NULL){
            strcpy(result, "That process doesn't exist!");
            return result;
        }

        char buf[100];
        while (fgets(buf, sizeof(buf), pf) != NULL) {
            char first4ch[5];
            getfirst4ch(first4ch, buf);
            
            if(strcmp(first4ch, "Name") == 0){
                strcpy(result, "\nProcess name: ");
                strcat(result, buf + 6);
            }

            if(strcmp(first4ch, "Stat") == 0){
                strcat(result, "\nState: ");
                strcat(result, buf + 7);
            }

            if(strcmp(first4ch, "PPid") == 0){
                strcat(result, "\nPPid: ");
                strcat(result, buf + 6);
            }

            if(strcmp(first4ch, "Uid:") == 0){
                strcat(result, "\nUid: ");
                strcat(result, buf + 5);
            }

            if(strcmp(first4ch, "VmSi") == 0){
                strcat(result, "\nVmSize: ");
                strcat(result, buf + 11);
            }
        }
    }

    return result;
}

int main()
{
    int fd1;
    char * myfifo = "/tmp/myfifo";

    // rw
    mkfifo(myfifo, S_IRWXU);

    char str1[80];
    while (1)
    {
        int sockets[2], child;
        socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
        
        fd1 = open(myfifo,O_RDONLY);
        read(fd1, str1, 80);

        if(strcmp(str1, "quit") == 0)
            exit(0);

        child = fork();

        if(child){
            // parent
            close(sockets[0]);

            // primeste raspunsul final de la fiu
            char str[512];
            read(sockets[1], str, 512);

            int len = strlen(str);
            if(str[len - 1] == '1')
                isLogged = 1;

            else
                isLogged = 0;

            str[len - 1] = '\0';

            close(fd1);
            // write result
            fd1 = open(myfifo,O_WRONLY);
            write(fd1, str, strlen(str)+1);
            close(fd1);

            close(sockets[1]);
        }

        else{
            // child
            close(sockets[1]);

            // executa comanda
            char *result = handleCommand(str1);

            int resLen = strlen(result);
            if(isLogged)
                result[resLen] = '1';
            
            else
                result[resLen] = '0';
            
            result[resLen + 1] = '\0';

            // trimite razultatul la tata
            write(sockets[0], result, strlen(result)+1);
            free(result);

            close(sockets[0]);
            exit(0);
        }

    }
    return 0;
}