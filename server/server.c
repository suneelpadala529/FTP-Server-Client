//Multithread server
# include <stdio.h>
# include <pthread.h>
# include <time.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <string.h>
# include <unistd.h>
# include <stdlib.h>

# define SERVERPORT 8003
# define BUFFERSIZE 1024

int g ;

void client(void *args)
{
    while (1)
    {
        int connection_sock = *(int*) args ;
        int i , opt, fsize, bytes_sent, bytes_read, c;
        char recv_buf[BUFFERSIZE] , send_buf[BUFFERSIZE];
        char command[3][20], name[50], size[50];

        bytes_read = recv(connection_sock , command , BUFFERSIZE , 0);
        printf("%d server bytes received \ncommand is : %s \n " , bytes_read , command[0]);

        if(strcmp(command[0],"cd")==0)
        {
            if (strcmp(command[1], "")==0)
            {
                printf("\ninside cd");
                chdir(getenv("HOME"));
                printf("\ncurrent directory ::%s\n\n",getcwd(NULL, 0));
            }
            else
            {
                int temp=0;
                temp = chdir(command[1]);
                if(temp==-1)
                    printf("Entered path does not exist in the directory\n");
                printf("\ncurrent directory ::%s\n\n",getcwd(NULL, 0));
            }
        }
        else if(strcmp(command[0],"close")==0)
        {
            char ack[10];
            bytes_read = recv (connection_sock , ack , 10 , 0) ;
            printf("\n%s " , ack);
            sleep(2);
            if(strcmp(ack,"fin")==0)
            {
                strcpy(ack,"ack");
                bytes_sent = send(connection_sock,ack,10, 0);
                sleep(2);
                strcpy(ack,"fin");
                bytes_sent = send(connection_sock,ack,10, 0);
                sleep(2);
                bytes_read = recv ( connection_sock , ack , 10 , 0) ;
                printf("\n%s " , ack);
                sleep(2);
                if(strcmp(ack,"ack")==0)
                {
                    close(connection_sock);
                    printf("\nServer closed");
                    break;
                }
            }

        }
        else if(strcmp(command[0],"ls")==0)
        {
            printf("\ncurrent directory ::%s\n\n",getcwd(NULL, 0));
            char path[50];
            char buffer[1024], out_path[1024];

            //Add output file
            strcpy(path,"/bin/");
            strcat(path,command[0]);
            strcat(path, "> output.txt");
            sleep(1);

            FILE *fp;
            int status=0;
            pid_t pid=vfork();
            if(pid==0)
                execlp("sh","sh","-c",path,(char *)0); // executed the ls command and redirected to  file
            sleep(1);
            fp=fopen("output.txt","r"); //open the file read the content and stored to buffer
            int i=0;
            //Store in charatcter array
            while((c=getc(fp))!=EOF)
                buffer[i++]=c;
            buffer[i]='\0';
            fclose(fp);
            //Send buffer to Client
            bytes_sent = send(connection_sock,buffer,1024, 0);
            sleep(2);
        }
        else if(strcmp(command[0],"get")==0||strcmp(command[0],"put")==0)
        {
            printf("\n Server just at GET and PUT");
            bytes_read = recv (connection_sock , name , BUFFERSIZE , 0) ;
            printf ( " %d server bytes received : %s \n " , bytes_read , name ) ;
            printf("\n check if of get at server");
            if(strcmp(command[0],"get")==0)
            {
                FILE* fp;
                fp = fopen (&name, "r");
                fseek (fp, 0, SEEK_END);
                fsize = ftell (fp);
                sprintf (size, "%d", fsize);
                bytes_sent = send (connection_sock,size,strlen(size)+1, 0) ;
                printf ("%d bytes of data sent : %s \n", bytes_sent, size);
                sleep(2);

                fseek (fp, 0, SEEK_SET);
                c = 0;

                while (c < fsize)
                {
                    send_buf[0]='\0';
                    bytes_read = fread (send_buf, 1, 1000, fp);
                    printf(" bytes_read = %d \n", bytes_read);
                    send_buf[bytes_read]='\0';
                    c = c + bytes_read;
                    bytes_sent = send (connection_sock, send_buf, bytes_read, 0);
                    sleep(2);
                    bytes_sent = 0;
                    bytes_read = 0;
                }
                fclose(fp);
                printf ("\n GET complete at server\n");
            }
            if(strcmp(command[0],"put")==0)
            {
                FILE* fp;
                fp = fopen (&name, "w");
                bytes_read = recv ( connection_sock , size , BUFFERSIZE , 0) ;
                printf ( " %d bytes received : %s \n " , bytes_read , size ) ;
                fsize = atoi (size);

                c=0;
                while (c < fsize)
                {
                    bytes_read = recv (connection_sock, recv_buf,BUFFERSIZE, 0);
                    printf("size of recv buf = %d \n",bytes_read);
                    c = c + bytes_read;
                    fwrite (recv_buf,1,bytes_read, fp);
                    bytes_read = 0;
                    recv_buf[0] = '\0';
                }
                fclose(fp);
                printf("\n PUT Complete at server\n");
            }
        }
        else if(strcmp(command[0],"chmod")==0)
        {
            char path[50];
            strcpy(path,"/bin/");
            strcat(path,command[0]);
            pid_t process_id = fork();
            if(process_id < 0)
            {
                perror("\nInternal error: cannot fork.\n");
            }
            else if(process_id == 0)
            {   // Child Process.

                int exec_ret=execlp(path, command[0], command[1], command[2],NULL);
                printf("%d: if fail",exec_ret);
                printf("\nchmod complete at server");

            }
        }
        else
        {
            printf("\nInvalid Command");
        }
    }
}

//main function
int main ()
{
	pthread_t t1 ;
	int t_status ;
	int req_socket , bytes_sent , bytes_received , sock_len , connect_status , bind_status ;
	int connection_sock;
	struct sockaddr_in sock_server , sock_client ;
	char recv_buffer[BUFFERSIZE] , send_buffer[BUFFERSIZE];
	sock_len = sizeof (sock_client) ;

	// Create server socket for requests
	req_socket = socket(AF_INET,SOCK_STREAM,0) ;
	if ( req_socket == -1)
	{
		printf("Socket creation failed .\n " ) ;
		close(req_socket) ;
		return 1;
	}
	else
	{
		printf ("Socket creation successful with descriptor %d .\n " , req_socket ) ;
	}
	sock_server.sin_family = AF_INET ;
	sock_server.sin_addr . s_addr = htonl ( INADDR_ANY ) ;
	sock_server.sin_port = htons ( SERVERPORT ) ;
	bind_status = bind(req_socket , ( struct sockaddr*)&sock_server , sizeof (sock_server) ) ;

	if ( bind_status == -1)
	{
		printf("Socket binding failed .\n " ) ;
		close (req_socket) ;
		return 1;
	}
	else
    {
		printf ("Socket binding successful .\n " ) ;
	}
	int* p;
	printf("Listening for connection requests .\n " ) ;
	listen(req_socket , 5) ;
	while(1)
	{
		connection_sock = accept( req_socket , ( struct sockaddr*) &sock_client , &sock_len ) ;
		p=(int*)malloc(sizeof(int));
		*p=connection_sock;
		if ( connection_sock == -1)
		{
			printf (" Socket creation for client failed .\n " );
			close (connection_sock);
			close (req_socket);
			return 1;
		}
		else
		{
            printf("Socket created for client with descriptor %d .\n ", connection_sock);

            t_status = pthread_create(&t1 , NULL , &client , (void*)p);
            if(t_status)
            {
                printf("Thread  creation failed .\n ") ;
            }
            else
            {
                printf("Thread  creation successful .\n ") ;
            }
		}
    }
	// Waiting for threads to complete
	pthread_join (t1,NULL);
    close (connection_sock);
	close(req_socket);
    return 0;
 }
