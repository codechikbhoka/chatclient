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


struct cell
{
string ip;
string name;
};

struct cell info[1000];

void myfunc(string s)
{
	int number = s[0] - '0';
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
	for(int i=0;i<number-1;i++){
		cout<<"id = " << i <<"    ip = "<<info[i].ip<<"    name = "<<info[i].name<<endl;
	}
}

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

	int port,port1;
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


	/*Server==================================================*/
	if(argc==1 || argc == 3)
	{
	   	if(argc==3)
	   	{
	   		if(!strcmp("-p",argv[1]))
	   		{
	   			sscanf(argv[2],"%i",&port);
	   		}
	   		else
	   		{
	   			printf("Invalid parameter.\nUsage: chat [-p PORT] HOSTNAME\n");
	   			exit(0);
	   		}
	   	}
	   	else 
	   	{
	   		port=CENTRAL_SERVER_PORT;
	   	}

	   	printf("\n***Central Server program starting (enter \"quit\" to stop): \n");
	   	fflush(stdout);

	     /* Create and name a socket for the server */
	   	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	   	server_address.sin_family = AF_INET;
	   	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	   	server_address.sin_port = htons(port);
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
                   			fd_IParray[num_clients] = s;
		                    printf("Client %d joined\n",num_clients++);
		                    fflush(stdout);
		                    
		                    stringstream strs;
          					strs << num_clients-1;
          					
          					string temp_strs = strs.str();
          					//temp_strs = "Z" + temp_strs;
          					char* char_type = (char*) temp_strs.c_str();
		                    sprintf(msg,char_type);
		                    send(client_sockfd,msg,strlen(msg),0);

		                    for (int i = 1; i < num_clients; ++i)
		                    {
		                    	memset(&msg[0], 0, sizeof(msg));
		                    	int length = (fd_IParray[i]).size();
		                    	int a;
          						for (a=0;a<length;a++)
          				        {
          				            msg[a]= fd_IParray[i][a];
          				        }
          				        msg[a] = '\0';
                              	send(client_sockfd,msg,strlen(msg),0);

                              	memset(&msg[0], 0, sizeof(msg));
                              	length = (fd_nickarray[i]).size();
          						for (a=0;a<length;a++)
          				        {
          				            msg[a]= fd_nickarray[i][a];
          				        }
          				        msg[a] = '\0';
                              	send(client_sockfd,msg,strlen(msg),0);
		                    	
		                    }

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
		   			}
		   			else if(fd) 
		   			{  /*Process Client specific activity*/
		                 //read data from open socket
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

		                    if (strstr(msg, "NICK:") == msg+1)   //if message is nick
		                    {

		                    	cout << "Client " << fd-3 << "'s nick is : " << msg+6;
								fd_nickarray[fd-3]=msg+6;  // +5 to remove NICK:
								//cout << "fd_nickarray["<<num_clients<<"] is "<<fd_nickarray[num_clients];
							}
							else if (strcmp(msg, "GETLIST") == 0)
							{
								stringstream strs;
	          					strs << num_clients-1;
	          					
	          					string temp_strs = strs.str();
	          					//temp_strs = "Z" + temp_strs;
	          					char* char_type = (char*) temp_strs.c_str();
			                    sprintf(msg,char_type);
			                    send(client_sockfd,msg,strlen(msg),0);

			                    for (int i = 1; i < num_clients; ++i)
			                    {
			                    	memset(&msg[0], 0, sizeof(msg));
			                    	int length = (fd_IParray[i]).size();
			                    	int a;
	          						for (a=0;a<length;a++)
	          				        {
	          				            msg[a]= fd_IParray[i][a];
	          				        }
	          				        msg[a] = '\0';
	                              	send(client_sockfd,msg,strlen(msg),0);

	                              	memset(&msg[0], 0, sizeof(msg));
	                              	length = (fd_nickarray[i]).size();
	          						for (a=0;a<length;a++)
	          				        {
	          				            msg[a]= fd_nickarray[i][a];
	          				        }
	          				        msg[a] = '\0';
	                              	send(client_sockfd,msg,strlen(msg),0);
			                    	
			                    }
							}
		                    else   //if message is not nick
		                    {
		                         /*Exit Client*/
		                    	if(msg[0] == 'X')
		                    	{
		                    		exitClient(fd,&readfds, fd_array,&num_clients);
		                    	} 
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


	}//end Server code

}//main


