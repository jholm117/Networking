#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <iostream>

#define BUFSIZE 1024

using namespace std;

int write_n_bytes(int fd, char * buf, int count);
int readnbytes(int fd,char *buf,int size);

void error(string msg) {
    perror(msg.c_str());
    exit(0);
}

int main(int argc, char * argv[]) {
    char * server_name = NULL;
    int server_port = 0;
    char * server_path = NULL;
			
    int sock = 0;
    
    bool ok = true;
    struct sockaddr_in sa;
    
    struct hostent * site = NULL;
    
	char * prereq = "GET %s HTTP/1.0\r\n\r\n";
	char getRequest[BUFSIZE];
    char buf[BUFSIZE + 1];
	int length;		//length of getRequest
    char* buf2;
	fd_set set;
	
	//unused variables -maybe helpful later
	
    //char * bptr = NULL;
    //char * bptr2 = NULL;
    //char * endheaders = NULL;
    //struct timeval timeout;
	//int rc = -1;
    //int datalen = 0;
    //FILE * wheretoprint = stdout;
	
	
	bzero(buf, BUFSIZE+1);
	

    /*parse args */
    if (argc != 5) {
	fprintf(stderr, "usage: http_client k|u server port path\n");
	exit(-1);
    }

    server_name = argv[2];
    server_port = atoi(argv[3]);
    server_path = argv[4];


    /* initialize minet */
    if (toupper(*(argv[1])) == 'K') { 
	minet_init(MINET_KERNEL);
    } else if (toupper(*(argv[1])) == 'U') { 
	minet_init(MINET_USER);
    } else {
	fprintf(stderr, "First argument must be k or u\n");
	exit(-1);
    }


    /* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("ERROR opening socket");
	
    // Do DNS lookup
    /* Hint: use gethostbyname() */
	site = gethostbyname(server_name);
	if(site == NULL){
		fprintf(stderr,"ERROR, no such host as %s\n", (char *)site);
		exit(0);
	}
		
    /* set address */
	memset(&sa,0,sizeof(sa));
	sa.sin_port = htons(server_port);
	sa.sin_addr.s_addr = *(unsigned long *)site->h_addr_list[0];
	sa.sin_family = AF_INET;
	

    /* connect socket */
    if(connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0){
		error("ERROR connecting");
	}	
	
	
    /* send request */
	sprintf(getRequest, prereq, server_path);
	length = strlen(getRequest);
	
	
	write_n_bytes(sock, getRequest, length);

						
    /* wait till socket can be read */
    /* Hint: use select(), and ignore timeout for now. */
    
	
	int n;	
	
	FD_ZERO(&set);		//zero out set
	FD_SET(sock,&set);	//add sockfd to set

	if(select(sock+1, &set, NULL, NULL, NULL) == -1)
	{
		perror("select");
		exit(1);
	}

		
    /* first read loop -- read headers */
    n = readnbytes(sock, buf, BUFSIZE);
	//cout << buf << endl;
	if(n <= 0)
	{
		perror("Error reading from socket");
		FD_CLR(sock, &set);
		close(sock);
	}
	
	
	
	//cout << "buf : \n" << buf << endl;
	
	// parse body
	
	buf[n] = '\0';				//set null character to terminate string
	string body = "", header = "";	//initialize strings
	int test = 0;				//index of blank line
	
	body = string(buf);
	
	test = body.find("\r\n\r\n");			
	header = body.substr(0, test);
	
	body = body.substr(test+4);
	body = body.substr(0, n);

    /* examine return code */   
    //Skip "HTTP/1.0"
    //remove the '\0'
    // Normal reply has return code 200
    
	// Parse headers
	int pos1 = header.find(" ");
	int pos2 = header.find(" ", pos1+1);
    int errorCode = stoi(header.substr(pos1, pos2-pos1));
  
  //content length parsing
/*
	pos1 = header.find("Content-Length: ");
    pos1 += strlen("Content-Length: ");
    pos2 = header.find("\r", pos1);
    int contentLength = stoi(header.substr(pos1, pos2-pos1));
    int bodyLength = body.length();
	
	int diff = contentLength - bodyLength;
*/


	if(errorCode != 200)
	{
		ok = false;
		cout << header << endl << endl;
	}
	if(n == BUFSIZE)
	{	
		buf2 = new char[BUFSIZE+1];		//allocate space for buf2
		
	while(n == BUFSIZE){
		memset(buf2, 0, BUFSIZE+1);				//clear buf
		n = readnbytes(sock, buf2, BUFSIZE);		// read into buf2
		
		buf2[n] = '\0';						//set null character at end of string
		
		body += string(buf2);				//concat buf onto body
		
		
	}
		
		delete buf2;
	}
	cout << body;	

    /*close socket and deinitialize */
	close(sock);


    if (ok) {
	return 0;
    } else {
	return -1;
    }
}

int write_n_bytes(int fd, char * buf, int count) {
    int rc = 0;
    int totalwritten = 0;

    while ((rc = minet_write(fd, buf + totalwritten, count - totalwritten)) > 0) {
	totalwritten += rc;
    }
    
    if (rc < 0) {
	return -1;
    } else {
	return totalwritten;
    }
}
int readnbytes(int fd,char *buf,int size)
{
  int rc = 0;
  int totalread = 0;
  while ((rc = read(fd,buf+totalread,size-totalread)) > 0)
    totalread += rc;

  if (rc < 0)
  {
    return -1;
  }
  else
    return totalread;
}