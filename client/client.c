/* Program for a simple TCP echo client */
# include <sys/socket.h>
# include <arpa/inet.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <math.h>
#include<stdlib.h>
# define SERVERADDRESS "127.0.0.1"
# define SERVERPORT 8003
# define BUFFERSIZE 1024

int getWords(char *base, char target[3][20])
{
	int n=0,i,j=0;

	for(i=0;;i++)
	{
		if(base[i]!=' '){
			target[n][j++]=base[i];
		}
		else{
			target[n][j++]='\0';//insert NULL
			n++;
			j=0;
		}
		if(base[i]=='\0')
		    break;
	}
	return n;
}

int main(int argc, char **argv)
{
    if (chdir("/home/suneel/Desktop/os/client-2") != 0)
        perror("chdir() to /usr failed");
    int conn_socket , bytes_received , bytes_sent , sock_len , conn_status , fsize;
    struct sockaddr_in sock_server , sock_client ;
    char recv_buffer[BUFFERSIZE] , send_buffer[BUFFERSIZE], command[50], name[50], size[50];
    sock_len = sizeof(sock_server) ;

    // Create a connection socket for client
    conn_socket = socket( AF_INET,SOCK_STREAM, 0) ;
    if (conn_socket == -1)
    {
        printf ("Socket creation failed .\n") ;
        close (conn_socket) ;
        return 1;
    }
    else
    {
        printf("Socket creation successful with descriptor %d .\n " , conn_socket) ;
    }

    // Requesting for a TCP connection to the server
    memset (&sock_server , 0 , sizeof (sock_server) ) ;
    sock_server . sin_family = AF_INET ;
    inet_aton (SERVERADDRESS , &sock_server . sin_addr ) ;
    sock_server . sin_port = htons (SERVERPORT) ;
    conn_status = connect(conn_socket ,(struct sockaddr*)&sock_server , sock_len ) ;

    if ( conn_status == -1)
    {
        printf ("Connection to server failed .\n " ) ;
        close (conn_socket ) ;
        return 1;
    }
    else
    {
        printf ("Connected to the server .\n " ) ;
    }
    while (1)
    {
        char opt[50];
        char com[3][20];
        for(int i=0;i<3;i++)
             memset(com[i], 0, 20);
        char delim[] = " ";
        printf("\n$FTP-Client$");
        gets(opt);
        getWords(opt, com);

        // Sending the message to the server

        if(strcmp(com[0],"lcd")==0)
        {
            if (strcmp(com[1], "")==0)
            {
                chdir(getenv("HOME"));
                printf("\ncurrent directory ::%s\n\n",getcwd(NULL, 0));
            }
            else
            {
                int temp=0;
                temp = chdir(com[1]);
                if(temp==-1)
                {
                    printf("Entered path does not exist in the directory\n");
                }
                printf("\ncurrent directory ::%s\n\n",getcwd(NULL, 0));
            }
        }
        else if(strcmp(com[0],"lls")==0||strcmp(com[0],"lchmod")==0)
        {
            char path[50];
            int exec_ret;
            strcpy(path,"/bin/");
            strcat(path,com[0]+1);
            sleep(2);
            pid_t process_id = fork();
            if(process_id < 0)
            {
                perror("\nInternal error: cannot fork.\n");
                return -1;
            }
            else if(process_id == 0)
            {
                // Child Process.
                if(strcmp(com[0],"lls")==0)
                {
                    exec_ret=execlp(path, com[0]+1, NULL);
                }
                else
                {
                    int exec_ret=execlp(path, com[0]+1,com[1],com[2],NULL);
                    printf("%d: else fail",exec_ret);
                }
            }
        }
        else if(strcmp(com[0],"ls")==0)
        {
            printf("\nServer is in command execution");
            bytes_sent = send(conn_socket,com,60, 0);
            sleep(2);
            printf("%d bytes sent : %s \n " , bytes_sent , com[0]);
            int bytes_read;
            char recv_buf[1024];
            bytes_read = recv(conn_socket , recv_buf , BUFFERSIZE , 0);
            printf("\n====================");
            printf("\n%s \n", recv_buf);
        }
        else if(strcmp(com[0],"cd")==0)
        {
            printf("\nServer is in command execution");
            bytes_sent = send(conn_socket,com,60, 0);
            sleep(2);
            printf("%d bytes sent : %s \n " , bytes_sent , com[0]);
        }
        else if(strcmp(com[0],"chmod")==0)
        {
            printf("\nServer is in command execution");
            bytes_sent = send(conn_socket,com,60, 0);
            sleep(2);
            printf("%d bytes sent : %s \n " , bytes_sent , com[0]);
        }
        else if(strcmp(com[0],"close")==0)
        {
            int bytes_read;
            bytes_sent = send(conn_socket,com,60, 0);
            sleep(2);
            char bit[10];
            strcpy(bit,"fin");
            bytes_sent = send(conn_socket,bit,10, 0);
            sleep(2);
            bytes_read = recv(conn_socket , bit , 10 , 0);
            printf("\n%s", bit);
            sleep(2);
            if(strcmp(bit,"ack")==0)
            {
                bytes_read = recv(conn_socket , bit , 10 , 0);
                printf("\n%s", bit);
                sleep(2);
                if(strcmp(bit,"fin")==0)
                {
                    strcpy(bit,"ack");
                    bytes_sent = send(conn_socket,bit,10, 0);
                    sleep(2);
                    close(conn_socket);
                    printf("\nConnection closed from client\n");
                    exit(0);
                }
            }
        }
        else if(strcmp(com[0],"get")==0||strcmp(com[0],"put")==0)
        {
            bytes_sent = send(conn_socket,com,60, 0);
            printf("%d bytes sent : %s \n " , bytes_sent , com[0]);
            sleep(2);
            printf("\nEnter the name of the file: ");
            scanf("%s",name);

            bytes_sent = send (conn_socket,name,strlen(name)+1, 0) ;
            sleep(2);
            printf ( "\n %d bytes sent : %s \n " , bytes_sent , name) ;

            if(strcmp(com[0],"get")==0)
            {
                //waiting for size if it is get
                FILE *fp;
                fp = fopen (&name, "w");
                bytes_received = recv ( conn_socket , size , BUFFERSIZE , 0) ;
                printf ("%d bytes received : %s \n " , bytes_received , size ) ;
                fsize = atoi (size);

                //file operation
                fseek (fp, 0, SEEK_SET);
                int c = 0;
                while (c < fsize)
                {
                    bytes_received = recv(conn_socket, recv_buffer,BUFFERSIZE, 0);
                    //b=strlen(recv_buf);
                    printf("yoo size of recv buf = %d \n",bytes_received);
                    c = c + bytes_received;
                    fwrite (recv_buffer,1,bytes_received, fp);
                    bytes_received = 0;
                    recv_buffer[0] = '\0';
                }
                fclose(fp);
                printf ("\n Download Complete \n");
            }
            else if(strcmp(com[0],"put")==0)
            {
                FILE *fp;
                fp = fopen (&name, "r");
                fseek (fp, 0, SEEK_END);
                fsize = ftell (fp);
                sprintf (size, "%d", fsize);
                bytes_sent = send (conn_socket,size,strlen(size)+1, 0) ;
                printf ("%d bytes of data sent : %s \n", bytes_sent, size);
                sleep(2);


                fseek (fp, 0, SEEK_SET);
                int c = 0;
                while (c < fsize)
                {
                    send_buffer[0]='\0';
                    bytes_received = fread (send_buffer, 1, 1000, fp);
                    printf(" bytes_read = %d \n", bytes_received);
                    send_buffer[bytes_received]='\0';
                    c = c + bytes_received;
                    bytes_sent = send (conn_socket, send_buffer, bytes_received, 0);
                    sleep(2);
                    bytes_sent = 0;
                    bytes_received = 0;
                }
                printf ("\n Upload Complete \n");
            }
            else
            {
                printf("\nInvalid Command");
            }
            
        }

    }

}
