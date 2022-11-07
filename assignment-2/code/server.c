// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#define PORT 80

int main(int argc, char const *argv[])
{

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    struct passwd* pwd_ptr;

    if (argc == 1) { //parent running
        printf("Parent running \n");
        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Attaching socket to port 8080
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
        &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address,
        sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        //privilege separation starts here
        pid_t sub_process = fork(); //fork the child process

        int status = 0; 
        if(sub_process < 0) //sub_process < 0 is error
        {
            perror("Fork failed"); 
            exit(EXIT_FAILURE);  
        }   
        if (sub_process > 0) 
        {
            waitpid(sub_process, &status, 0);
            exit(0);
        }
        if(sub_process == 0) // child process as sub_process == 0
        { 
            printf("Child forked \n");
        
            pwd_ptr = getpwnam("nobody"); //fetch correct user id for nobody user
            int sid = setuid(pwd_ptr->pw_uid); //drop its privileges to nobody user using setuid
            if(sid == -1){ //if setuid return value is -1, it's an error
                printf("Error no=%ld\n",(long) pwd_ptr->pw_uid);
            }
            if(sid != 0){
                perror("Failed to set id of child process");
                exit(EXIT_FAILURE);
            } 
            else{ //successfully completed setuid as return value is 0
                printf("Child process uid: %u \n", pwd_ptr->pw_uid);

                char *fname = "./server";
                char *arg[3];
                char targ[10];
                sprintf(targ, "%d", server_fd);
                arg[0] = fname;
                arg[1] = targ;
                arg[2] = NULL;
                
                printf("Starting the re-exec of forked child... \n");
                //re exec the server's child process (after fork)
                execvp(fname, arg);

                printf("----- Child process is done -----\n");
            }
        } 
    }
    //re-exec'ed code of listening the incoming client request starts from here
    //attaching socket file descriptor to new exec'ed child
    printf("Re-execed forked child listening... \n");
    server_fd = atoi(argv[1]); 

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                    (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    //new exec'ed child processing the incoming child request
    valread = read( new_socket , buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");

    return 0;

}
