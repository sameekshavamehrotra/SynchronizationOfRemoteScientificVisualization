#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define close(s);

/* BufferLength is 100 bytes */
#define BufferLength 1024
/* Server port number */
#define SERVPORT 8221
pthread_t threads[2];

class ClientSocket
{
public:
		
		SOCKET   s;
		int r;
		struct hostent *h;

		int Option;
		
		char* IPAddress;
		int PortNumber;
	
		ClientSocket()
		{

		}

		ClientSocket(char* IPAddress,int PortNumber)
		{
			IPAddress=IPAddress;

			printf("IP Address of the Server %s \n", IPAddress);

			PortNumber=PortNumber;
			printf("Port Number to connect to the Server %d\n",PortNumber);

			pthread_create(&threads[2],NULL,start_client,(void *)this);
		}

		void AddHost(char* IPAddressArg,int PortNumber)
		{
			IPAddress=IPAddressArg;
			this->PortNumber=PortNumber;
			pthread_create(&threads[2],NULL,start_client,(void *)this);
		}

		~ClientSocket()
		{
			#if defined WIN32
				WSACleanup();
			#endif
		}
		void CloseSocket()
		{
			close(s);
		}
		static void* start_client(void *arg)
		{
			struct sockaddr_in addr;

			int diffTimeSecToReConnect=300;
			int tryToConnectCounter=0;
			time_t firstTimeToConnect;

			ClientSocket *cSocket=(ClientSocket *)arg;
			try
			{
				memset((void*)&(addr), 0, sizeof(addr));
				cSocket->h = gethostbyname(cSocket->IPAddress);
					if(NULL == cSocket->h) {
						perror("Could not get host by name");
						pthread_exit(NULL);
					}
					else
					{
					      bcopy((char *)((cSocket->h)->h_addr),
					      (char *)&(addr.sin_addr.s_addr),
					      (cSocket->h)->h_length);
						printf("Hostname ok\n");
					}			

				cSocket->s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if(INVALID_SOCKET == cSocket->s) {
					perror("Could not create socket");
					pthread_exit(NULL);
				}

				addr.sin_family = AF_INET;
				addr.sin_addr   = *((in_addr*)*((cSocket->h)->h_addr_list));
	 			addr.sin_port   = htons(cSocket->PortNumber);
			
				while(diffTimeSecToReConnect <= 300)
				{
					if((cSocket->r = connect(cSocket->s, (struct sockaddr *)&addr,sizeof(addr))) < 0)
					{
						tryToConnectCounter=tryToConnectCounter + 1;
						if(tryToConnectCounter == 1)
						{
							firstTimeToConnect=time(NULL);
						}
						else if(tryToConnectCounter > 300)
						{
							fprintf(stderr, "Failed to connect\n");				
							perror("Cannot connect to server\n");
							cSocket->CloseSocket();
							pthread_exit(NULL);
						}
						else
						{
							time_t currentTime=time(NULL);
							diffTimeSecToReConnect=difftime(currentTime,firstTimeToConnect);
						}
					}
					else
					{
						printf("Connected.\n");
						break;
					}
				}
			}
			catch(char *exp)
			{
				printf("Exception is %s\n",exp);
				printf("here?\n");
			}
		}

		virtual void SendUpdate(int PortNumber)
		{

		}
		
		void SendData(int CaseNumber,char *VariableName,char *VariableContent)
		{
			char *dataContent=(char *)malloc(sizeof(char)*(strlen(VariableName) + strlen(VariableContent) + 5));
			char serverResponse[6];			
			switch(CaseNumber)
			{
				case 1:
				{
					int PacketSize=strlen(VariableName) + strlen(VariableContent) + 2;
					sprintf(dataContent,"%d_PSize",PacketSize);
					printf("%s\n",dataContent);
					break;
				}				
				case 2:
				{
					int number =strlen(VariableContent);
					int counter =0;
					while(number != 0)
					{
						counter=counter + 1;
						int quotient=number/10;
						int remainder=number-(quotient*10);
						number=(number-remainder)/10;	
					}
					
					int PacketSize=strlen(VariableName) + counter + 2;			
					sprintf(dataContent,"%d_PSize!",PacketSize);
					printf("%s\n",dataContent);
					break;
				}
				case 3:
				{
					int PacketSize=strlen(VariableName) + strlen(VariableContent) + 2;
					sprintf(dataContent,"%d_PSize",PacketSize);
					printf("%s\n",dataContent);
					break;
				}
			}
				
			send(s,dataContent,strlen(dataContent),0);
			
			int rc=recv(s,serverResponse,6,0);
			if(rc > 0)
			{
			memset((void*)dataContent, 0, strlen(dataContent));
			switch(CaseNumber)
				{
					case 1:
					{
						sprintf(dataContent,"%s@%s!",VariableName,VariableContent);
						printf("%s\n",dataContent);
						break;
					}				
					case 2:
					{			
						sprintf(dataContent,"%s@%d!",VariableName,strlen(VariableContent));
						printf("%s\n",dataContent);
						break;
					}
					case 3:
					{
						sprintf(dataContent,"%s@%s!",VariableName,VariableContent);
						printf("%s\n",dataContent);
						break;
					}
				}
				send(s,dataContent,strlen(dataContent),0);
}
				dataContent=NULL;		
		}
};

class ServerSocket
{
public:

	int sd, sd2, rc, length;
	int totalcnt, on;
	char temp;
	char buffer[BufferLength];
	struct sockaddr_in serveraddr;
	struct sockaddr_in their_addr;
	char *serverDirectoryPath;
	int lendataReceived;
	fd_set read_fd;
	struct timeval timeout;

	ServerSocket()
	{
		/* Variable and structure definitions. */
		length=sizeof(int);
		on=1;
		lendataReceived=20;
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		#if defined WIN32
			WSADATA wsa_data;
			WSAStartup(MAKEWORD(1,1), &wsa_data);
		#endif

		pthread_create(&threads[1],NULL,start_Server,(void *) this);
	}

	~ServerSocket()
	{		
		#if defined WIN32
			WSACleanup();
		#endif	
	}
	void CloseSocket()
	{
		close(sd);
		close(sd2);
	}
	void Close1Socket()
	{
		close(sd);
	}
	static void* start_Server(void *arg)
	{
				
		/* The socket() function returns a socket descriptor */
		/* representing an endpoint. The statement also */
		/* identifies that the INET (Internet Protocol) */
		/* address family with the TCP transport (SOCK_STREAM) */
		/* will be used for this socket. */
		/************************************************/
		/* Get a socket descriptor */
		ServerSocket *sSocket = (ServerSocket *) arg;
		char *dataReceived=(char *)malloc(sizeof(char)* (sSocket->lendataReceived));
		
		if((sSocket->sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		perror("Server-Socket() error");
		/* Just exit */

		}
		else
		printf("Server-Socket() is OK\n");
		 
		/* The setsockopt() function is used to allow */
		/* the local address to be reused when the server */
		/* is restarted before the required wait time */
		/* expires. */
		/***********************************************/
		/* Allow socket descriptor to be reusable */
		if((sSocket->rc = setsockopt(sSocket->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&(sSocket->on), sizeof(sSocket->on))) < 0)
		{
		perror("Server-SetSockopt() error");
		sSocket->Close1Socket();
		pthread_exit(NULL);
		}
		else
		printf("Server-SetSockopt() is OK\n");
		 
		/* bind to an address */
		memset(&(sSocket->serveraddr), 0x00, sizeof(struct sockaddr_in));
		sSocket->serveraddr.sin_family = AF_INET;
		sSocket->serveraddr.sin_port = htons(SERVPORT);
		sSocket->serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

//		printf("Using %s, listening at %d\n",(char *)(sSocket->serveraddr.sin_addr.s_addr), SERVPORT);
		 
		/* After the socket descriptor is created, a bind() */
		/* function gets a unique name for the socket. */
		/* In this example, the user sets the */
		/* s_addr to zero, which allows the system to */
		/* connect to any client that used port 3005. */
		if((sSocket->rc = bind(sSocket->sd, (struct sockaddr *)&(sSocket->serveraddr), sizeof(sSocket->serveraddr))) < 0)
		{
		perror("Server-Bind() error");
		/* Close the socket descriptor */
		sSocket->Close1Socket();
		/* and just exit */
		pthread_exit(NULL);
		}
		else
			printf("Server-Bind() is OK\n");
		 
		/* The listen() function allows the server to accept */
		/* incoming client connections. In this example, */
		/* the backlog is set to 10. This means that the */
		/* system can queue up to 10 connection requests before */
		/* the system starts rejecting incoming requests.*/
		/*************************************************/
		/* Up to 10 clients can be queued */
		if((sSocket->rc = listen(sSocket->sd, 10)) < 0)
		{
			perror("Server-listen() error");
			sSocket->Close1Socket();
			pthread_exit(NULL);
		}
		else
			printf("Server-Ready for client connection...\n");
		 
		/* The server will accept a connection request */
		/* with this accept() function, provided the */
		/* connection request does the following: */
		/* - Is part of the same address family */
		/* - Uses streams sockets (TCP) */
		/* - Attempts to connect to the specified port */
		/***********************************************/
		/* accept() the incoming connection request. */
		int sin_size = sizeof(struct sockaddr_in);
		if((sSocket->sd2 = accept(sSocket->sd, (struct sockaddr *)&(sSocket->their_addr), (socklen_t *)&sin_size)) < 0)
		{
			perror("Server-Accept() error");
			sSocket->Close1Socket();
			pthread_exit(NULL);
		}
		else
		printf("Server-Accept() is OK\n");
		printf("Server-new socket, sd2 is OK...\n");
		
		//Start Reading data from the socket for file Updates.
			
		FD_ZERO(&(sSocket->read_fd));
		FD_SET(sSocket->sd2, &(sSocket->read_fd));
		sSocket->rc = select((sSocket->sd2)+1, &(sSocket->read_fd), NULL, NULL, &(sSocket->timeout));
		if((sSocket->rc == 1) && (FD_ISSET(sSocket->sd2, &(sSocket->read_fd))))
		{ 
			while(SERVPORT != 0)
			{	
				int prevlenDataReceived=sSocket->lendataReceived;
				sSocket->rc=recv((sSocket->sd2), (dataReceived), sSocket->lendataReceived, 0);
			
				printf("%s\n",dataReceived);
				char *PacketSize=strstr(dataReceived,"PSize");				
				if(PacketSize != NULL)
				{		
					char *splitData=strtok(dataReceived,"!");
					char *splitSize=strtok(splitData,"_");
					int lenTobeReceivedContent=atoi(splitSize);
					sSocket->lendataReceived=lenTobeReceivedContent;
					sSocket->rc=send((sSocket->sd2), "Resend", 6, 0);
				}
				else
				{					
					if((sSocket->rc) < 0)
					{
						printf("Client Connection Hanged Up.\n");
						sSocket->CloseSocket();
						break;
					}
					else if ((sSocket->rc) == 0)
					{
						printf("Client Program has issued a Close.\n");
						sSocket->CloseSocket();
						break;
					}
					else
					{	
						sSocket->ProcessUpdate(dataReceived);						
					}
				}
				dataReceived=NULL;
				dataReceived=(char *)realloc(dataReceived,sizeof(char)* (sSocket->lendataReceived));
			}
		}
		else if (sSocket->rc < 0)
		{
			printf("Server-Select() Error.\n");
			sSocket->CloseSocket();
		}
		else
		{
			printf("Server-Select() timed Out.\n");
			sSocket->CloseSocket();
		}	
		
	}

	virtual void ProcessUpdate(char *dataRcv)
	{				
		
	}
};
