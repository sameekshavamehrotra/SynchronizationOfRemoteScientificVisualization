#if defined WIN32
#include <winsock.h>
typedef int socklen_t;
#define closesocket(s);
#elif defined __linux__
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define close(s);
#endif
#include <string.h>
#include <stdio.h>

/* BufferLength is 100 bytes */
#define BufferLength 1024
/* Server port number */
#define SERVPORT 8221

class ClientSocket
{
public:
		struct sockaddr_in addr;
		SOCKET   s;
		int r;
		hostent* h;

		char data[BufferLength];
		char recieved[BufferLength];
		int Option;
		static int intCounter;
		
		char IPAddress[100];
		int PortNumber;

		ClientSocket()
		{
			#if defined WIN32
				WSADATA wsa_data;
				WSAStartup(MAKEWORD(1,1), &wsa_data);
			#endif
			printf("Enter the IP Address of the Server\n");
			scanf("%s",IPAddress);

			printf("Enter the Port Number to connect to the Server\n");
			scanf("%d",&PortNumber);

			start_client();
		}

		~ClientSocket()
		{
			#if defined WIN32
				WSACleanup();
			#endif
		}
		void CloseSocket()
		{
				#if defined WIN32
					closesocket(s);
				#elif defined __linux__
					close(s);
				#endif
		}
		int start_client()
		{
		try
		{
			memset((void*)&addr, 0, sizeof(addr));
			addr.sin_addr.s_addr = inet_addr(IPAddress);
			if(INADDR_NONE == addr.sin_addr.s_addr) {
				h = gethostbyname(IPAddress);
				if(NULL == h) {
					perror("Could not get host by name");
					return -1;
				}
			} else {
				h = gethostbyaddr((const char*)&addr.sin_addr, sizeof(struct sockaddr_in), AF_INET);
				if(NULL == h) {
					perror("Could not get host by address");
					return -1;
				}
			}

			s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(INVALID_SOCKET == s) {
				perror("Could not create socket");
				return -1;
			}

			addr.sin_family = AF_INET;
			addr.sin_addr   = *((in_addr*)*h->h_addr_list);
			addr.sin_port   = htons(PortNumber);

			printf("Connecting... ");
			r = connect(s, (sockaddr*)&addr, sizeof(struct sockaddr));
			if(SOCKET_ERROR == r) {
				perror("Cannot connect to server");
				CloseSocket();
				return -1;
			}
			printf("Connected.\n");
			return 0;
		}
		catch(char exp[])
		{
			printf(exp);
		}

		}

		virtual void *SendUpdate(void *arg)
		{

		}
};

class ServerSocket
{
public:

	static int intCounter;
	int sd, sd2, rc, length;
	int totalcnt, on;
	char temp;
	char buffer[BufferLength];
	struct sockaddr_in serveraddr;
	struct sockaddr_in their_addr;
		 
	fd_set read_fd;
	struct timeval timeout;

	ServerSocket()
	{
		/* Variable and structure definitions. */
		length=sizeof(int);
		totalcnt=0;
		on=1;
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;

		#if defined WIN32
			WSADATA wsa_data;
			WSAStartup(MAKEWORD(1,1), &wsa_data);
		#endif

		ServerSocket sSocket;
		sSocket.start_Server();
	}

	~ServerSocket()
	{		
		#if defined WIN32
			WSACleanup();
		#endif	
	}
	void CloseSocket()
	{
			#if defined WIN32
				closesocket(sd);
				closesocket(sd2);
			#elif defined __linux__
				close(sd);
				close(sd2);
			#endif
	}
	void Close1Socket()
	{
			#if defined WIN32
				closesocket(sd);
			#elif defined __linux__
				close(sd);
			#endif
	}
	void start_Server()
	{
		/* The socket() function returns a socket descriptor */
		/* representing an endpoint. The statement also */
		/* identifies that the INET (Internet Protocol) */
		/* address family with the TCP transport (SOCK_STREAM) */
		/* will be used for this socket. */
		/************************************************/
		/* Get a socket descriptor */
		if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		perror("Server-socket() error");
		/* Just exit */
		exit (-1);
		}
		else
		printf("Server-socket() is OK\n");
		 
		/* The setsockopt() function is used to allow */
		/* the local address to be reused when the server */
		/* is restarted before the required wait time */
		/* expires. */
		/***********************************************/
		/* Allow socket descriptor to be reusable */
		if((rc = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) < 0)
		{
		perror("Server-setsockopt() error");
		Close1Socket();
		exit (-1);
		}
		else
		printf("Server-setsockopt() is OK\n");
		 
		/* bind to an address */
		memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(SERVPORT);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		 
		printf("Using %s, listening at %d\n", inet_ntoa(serveraddr.sin_addr), SERVPORT);
		 
		/* After the socket descriptor is created, a bind() */
		/* function gets a unique name for the socket. */
		/* In this example, the user sets the */
		/* s_addr to zero, which allows the system to */
		/* connect to any client that used port 3005. */
		if((rc = bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) < 0)
		{
		perror("Server-bind() error");
		/* Close the socket descriptor */
		Close1Socket();
		/* and just exit */
		exit(-1);
		}
		else
			printf("Server-bind() is OK\n");
		 
		/* The listen() function allows the server to accept */
		/* incoming client connections. In this example, */
		/* the backlog is set to 10. This means that the */
		/* system can queue up to 10 connection requests before */
		/* the system starts rejecting incoming requests.*/
		/*************************************************/
		/* Up to 10 clients can be queued */
		if((rc = listen(sd, 10)) < 0)
		{
			perror("Server-listen() error");
			Close1Socket();
			exit (-1);
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
		if((sd2 = accept(sd, (struct sockaddr *)&their_addr, &sin_size)) < 0)
		{
			perror("Server-accept() error");
			Close1Socket();
			exit (-1);
		}
		else
			printf("Server-accept() is OK\n");
		 
		/*client IP*/
		printf("Server-new socket, sd2 is OK...\n");
		printf("Got connection from the f***ing client: %s\n", inet_ntoa(their_addr.sin_addr));		 

	}

	virtual void *ProcessUpdate(void *arg)
	{

	}
};

int ClientSocket::intCounter=0;
int ServerSocket::intCounter=0;
