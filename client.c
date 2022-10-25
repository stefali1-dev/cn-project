
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    int fd;
    char * myfifo = "/tmp/myfifo";

    // rw
    mkfifo(myfifo, S_IRWXU);

    char arr1[512], arr2[80];
    while (1)
    {
        fd = open(myfifo, O_WRONLY);

        printf("Client command: ");
        // save terminal input in arr2
        fgets(arr2, 80, stdin);
        arr2[strlen(arr2) - 1] = '\0';

        if(strcmp(arr2, "help") == 0){
            printf("The avalable commands are:\n    - login : <username>    | It is used to log in\n");
            printf("    - get-logged-users  | Shows info about logged users\n");
            printf("    - get-proc-info : <pid> | Shows info about indicated process\n");
            printf("    - logout    | Logs you out\n");
            printf("    - quit  | Exits aplication\n\n");
            close(fd);
            continue;
        }

        // write to server
        write(fd, arr2, strlen(arr2)+1);
        close(fd);

        fd = open(myfifo, O_RDONLY);

        // save result in arr1
        read(fd, arr1, sizeof(arr1));

        // Print the read message
        printf("%s\n\n", arr1);
        close(fd);
    }
    return 0;
}