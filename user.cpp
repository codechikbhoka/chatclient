#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>
#include <sstream>

using namespace std;

#define MSG_SIZE 80
#define MAX_CLIENTS 95
#define CENTRAL_SERVER_PORT 7405
#define SERVER_PORT 7400
#define CONNECT_PORT 7400

void exitClient(int fd, fd_set *readfds, char fd_array[], int *num_clients)
{
	int i;

	close(fd);
    FD_CLR(fd, readfds); //clear the leaving client from the set
    for (i = 0; i < (*num_clients) - 1; i++)
    	if (fd_array[i] == fd)
    		break;          
    	for (; i < (*num_clients) - 1; i++)
    		(fd_array[i]) = (fd_array[i + 1]);
    	(*num_clients)--;
}

struct cell
{
string ip;
string name;
};

struct cell info[1000];

void myfunc(string s)
{
	int number = s[0] - '0';
	for(int i=0;i<number;i++){
		info[i].ip = "";
		info[i].name = "";
	}
	cout<<"Total Online Users : "<<number<<endl;
	int count = 0;
	int i=1;
	int flag = 0;
	while(1){
    if(s[i]!='\n'){
		if(isalpha(s[i])){
			flag=1;
			info[count].name = info[count].name + s[i]; 
		}
		else{
			if(flag==1){count++;}
			flag=0;
			info[count].ip = info[count].ip+ s[i];
		}	}
		i++;
		if(i==s.length()){break;}
	}
	for(int i=0;i<number;i++){
		cout<<"id = " << i <<"    ip = "<<info[i].ip<<"    name = "<<info[i].name<<endl;
	}
}



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char *argv[]) 
{
	int i=0;
	string MYNICK;
	string CENTRAL_SERVER;

	int port,port1,port2;
	int num_clients = 1;
	int server_sockfd, client_sockfd;
	struct sockaddr_in server_address;
	int addresslen = sizeof(struct sockaddr_in);
	int fd;
	char fd_array[MAX_CLIENTS]; 
	string fd_nickarray[MAX_CLIENTS];
	string fd_IParray[MAX_CLIENTS];
	fd_set readfds, testfds, clientfds;
	char msg[MSG_SIZE + 1];     
	char kb_msg[MSG_SIZE + 10]; 

	struct sockaddr_storage their_addr;  //new
	socklen_t sin_size; // new
	char s[INET6_ADDRSTRLEN]; // new

	/*Client variables=======================*/
	int sockfd, Friend_sockfd; //new
	int result;
	char hostname[MSG_SIZE];
	struct hostent *hostinfo, *Friend_hostinfo;
	struct sockaddr_in address, Friend_address;
	char alias[MSG_SIZE];
	int clientid;

   

   /*Client==================================================*/
	if(argc==2 || argc==4)
	{
			   	if(!strcmp("-p",argv[1]))
			   	{
			   		if(argc==2)
			   		{
			   			printf("Invalid parameters.\nUsage: chat [-p PORT] HOSTNAME\n");
			   			exit(0);
			   		}
			   		else
			   		{
			   			sscanf(argv[2],"%i",&port);
			   			strcpy(hostname,argv[3]);
			   		}
			   	}
			   	else
			   	{
			   		port=CENTRAL_SERVER_PORT;
			   		strcpy(hostname,argv[1]);
			   		CENTRAL_SERVER = string(hostname);
			   	}
			   	printf("\n*** Client program starting (enter \"quit\" to stop): \n");
			   	fflush(stdout);

			     /* Create a socket for the client */
			   	sockfd = socket(AF_INET, SOCK_STREAM, 0);

			     /* Name the socket, as agreed with the server */
			    hostinfo = gethostbyname(hostname);  /* look for host's name */
			   	address.sin_addr = *(struct in_addr *)*hostinfo -> h_addr_list;
			   	address.sin_family = AF_INET;
			   	address.sin_port = htons(port);

			     /* Connect the client socket "sockfd" to the server's socket */
			   	if(connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
			   	{
			   		perror("connecting");
			   		exit(1);
			   	}
			   	else
			   	{

			        printf("What is your nick ?\n");
			        fgets(kb_msg, MSG_SIZE+1, stdin);
			        //printf("%s\n",kb_msg);
			        if (strcmp(kb_msg, "quit\n")==0) 
			        {
			            sprintf(msg, "XClient is shutting down.\n");
			            write(sockfd, msg, strlen(msg));
			            close(sockfd); //close the current client
			            exit(0); //end program
			        }
			        else 
			        {
			            sprintf(msg, " NICK:%s", kb_msg);
			            write(sockfd, msg, strlen(msg));

			            result = read(sockfd, msg, MSG_SIZE); 
			            msg[result] = '\0';  /* Terminate string with null */

			            string y(msg);
			            if( (y[0]-'0') != 1)
			            {
			              myfunc(y);
			            }

			        } 
				}

				fflush(stdout);
			    FD_ZERO(&clientfds);  //clear all entries from the set clientfds (client file descriptor set)
			    FD_SET(sockfd,&clientfds);  // add sockfd to the clientfds to get input from server
			    FD_SET(0,&clientfds);//add stdin to the clientfds
			     
	} // end client code



	/*Server==================================================*/

		port1=SERVER_PORT;

	   	printf("\n*** Server program starting (enter \"quit\" to stop): \n");
	   	fflush(stdout);

	     /* Create and name a socket for the server */
	   	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	   	server_address.sin_family = AF_INET;
	   	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	   	server_address.sin_port = htons(port1);
	   	bind(server_sockfd, (struct sockaddr *)&server_address, addresslen);

	     /* Create a connection queue and initialize a file descriptor set */
	   	listen(server_sockfd, 1);
	   	FD_ZERO(&readfds);
	   	FD_SET(server_sockfd, &readfds);
	    FD_SET(0, &readfds);  /* Add keyboard to file descriptor set */


	     /*  Now wait for clients and requests */
	   	while (1) 
	   	{
	   		testfds = readfds;
	   		select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

	        /* If there is activity, find which descriptor it's on using FD_ISSET */
	   		for (fd = 0; fd < FD_SETSIZE; fd++) 
	   		{
	   			if (FD_ISSET(fd, &testfds)) 
	   			{

					if (fd == server_sockfd) 
					{ 	/* Accept a new connection request */
						sin_size = sizeof their_addr;
						client_sockfd = accept(server_sockfd , (struct sockaddr *)&their_addr, &sin_size);
						inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
						printf("server: got connection from %s\n", s);

						if (num_clients < MAX_CLIENTS) 
						{
							FD_SET(client_sockfd, &readfds);
                    		fd_array[num_clients]=client_sockfd;
                    		//fd_nickarray[num_clients]=info[y2].name;
                   			fd_IParray[num_clients] = s;

		                    /*Client ID*/
		                    printf("Client %d joined\n",num_clients++);
		                    fflush(stdout);

						}
						else 
						{
							sprintf(msg, "XSorry, too many clients.  Try again later.\n");
							write(client_sockfd, msg, strlen(msg));
							close(client_sockfd);
						}
	   				}
		   			else if(fd == 0)  
					{  
						/* Process keyboard activity */                
						fgets(kb_msg, MSG_SIZE + 1, stdin);
			   			if (strcmp(kb_msg, "quit\n")==0) 
			   			{
			   				sprintf(msg, "XServer is shutting down.\n");
			   				for (i = 0; i < num_clients ; i++) 
			   				{
			   					write(fd_array[i], msg, strlen(msg));
			   					close(fd_array[i]);

			   				}
			   				close(server_sockfd);
			   				exit(0);
			   			}
			   			else if (strcmp(kb_msg, "connect\n")==0)
			   			{

			   				sprintf(msg, "GETLIST");
						    write(sockfd, msg, strlen(msg));

						    result = read(sockfd, msg, MSG_SIZE); 
						    msg[result] = '\0';  /* Terminate string with null */

						    string y(msg);
						    if( (y[0]-'0') != 1)
						    {
						      myfunc(y);
						    }

						    int Online = y[0]-'0';
						    memset(&msg[0], 0, sizeof(msg));

						    if (Online>1)
						    {
								cout << "Enter the id you want to connect ?" << endl;
								fgets(kb_msg, MSG_SIZE+1, stdin);
								sprintf(msg, "%s", kb_msg);
								string y1(msg);
								int y2 = y1[0]-'0';

								char *xxx = &info[y2].ip[0];
								Friend_sockfd = socket(AF_INET, SOCK_STREAM, 0);
								Friend_hostinfo = gethostbyname(xxx);  /* look for host's name */
								Friend_address.sin_addr = *(struct in_addr *)*Friend_hostinfo -> h_addr_list;
								Friend_address.sin_family = AF_INET;
								Friend_address.sin_port = htons(port1);
								close(sockfd);
								connect(Friend_sockfd, (struct sockaddr *)&Friend_address, sizeof(Friend_address));			     
								cout << "Connected to " << y2 << endl;  


								FD_SET(Friend_sockfd, &readfds);
								//cout<<"num_clients while connecting is " << num_clients << endl;
	                    		fd_array[num_clients]=Friend_sockfd;
	                    		fd_nickarray[num_clients]=info[y2].name;
	                   			fd_IParray[num_clients++] = info[y2].ip;

						    }

			   			}
			   			else 
			   			{
			   				sprintf(msg, "%s", kb_msg);
			   				for (i = 0; i < num_clients ; i++) 
			   				{
			   					write(fd_array[i], msg, strlen(msg));

			   				}
			   				
			   				// write(Friend_sockfd, msg, strlen(msg));
			   			}
		   			}
		   			else if(fd) 
		   			{  /*Process Client specific activity*/
		                 //read data from open socket
		   				//cout << "fd is " << fd << endl;
				   		memset(&msg[0], 0, sizeof(msg));
				   		memset(&kb_msg[0], 0, sizeof(kb_msg));
				   		result = read(fd, msg, MSG_SIZE);

				   		if(result==-1) 
				   		{
				   			perror("read()");
				   		}
				   		else if(result>0)
				   		{
				   			msg[result]='\0';
	                    	cout << "message : "<< fd_nickarray[fd-4] << " : " << msg << endl;
	                        
	                         /*Exit Client*/
	                    	if(msg[0] == 'X')
	                    	{
	                    		exitClient(fd,&readfds, fd_array,&num_clients);
	                    	} 

				        }                                                   
			            
		        	}
		        	else 
		            {  /* A client is leaving */
		            	exitClient(fd,&readfds, fd_array,&num_clients);
		            }//if
		        }

	        }//for


	    }//while

}//main


