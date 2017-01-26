#include "minet_socket.h"
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <string>

#define BUFSIZE 1024
#define FILENAMESIZE 100

using namespace std;

int handle_connection(int);
int writenbytes(int,char *,int);
int readnbytes(int,char *,int);

int main(int argc,char *argv[])
{
  int server_port;
  int sock,sock2;
  struct sockaddr_in sa,sa2;
  int rc;
  int addrlen;
  fd_set read_fds;
  fd_set master;
  int fdmax;
  FD_ZERO(&read_fds);	//clear set
  FD_ZERO(&master);
  
  /* parse command line args */
  if (argc != 3)
  {
    fprintf(stderr, "usage: http_server1 k|u port\n");
    exit(-1);
  }
  server_port = atoi(argv[2]);
  if (server_port < 1500)
  {
    fprintf(stderr,"INVALID PORT NUMBER: %d; can't be < 1500\n",server_port);
    exit(-1);
  }

  /* initialize and make socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
	  perror("socket");
	  exit(1);
  }
  

  /* set server address*/
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons(server_port);
  memset(&(sa.sin_zero), '\0', 8);

  /* bind listening socket */
  if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("bind");
        exit(1);
    }

  /* start listening */
  if (listen(sock, 5) == -1) {
        perror("listen");
        exit(1);
    }
	
	FD_SET(sock, &master);	//add listening socket to set
	fdmax = sock;
	
  /* connection handling loop */
  while(1)
  {
	  /* addrlen = sizeof(sa2);
		if((sock2 = accept(sock, (struct sockaddr *) &sa2, (socklen_t *) &addrlen)) == -1)
		{
			perror("accept");
			continue;
		}
		cout << "connection made" << endl;
	  
	  if (!fork())
		{ // this is the child process
            close(sock); // child doesn't need the listener
			
            rc = handle_connection(sock2);
			
            close(sock2);
            exit(0);
        }
        close(sock2);  // parent doesn't need this
	 */ 
	  cout << "enter while loop" << endl;
	 
	  //wait until connection request has arrived
	  read_fds = master;
	if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(1);
	}	
	cout << "past select" << endl; 
	for (int i = 0; i <= fdmax; i++)
 	{
		if(FD_ISSET(i, &read_fds))
		{
			// sock ready to accept
			if(i == sock)
			{
				addrlen = sizeof(sa2);
				if((sock2 = accept(sock, (struct sockaddr *) &sa2, (socklen_t *) &addrlen)) == -1)
					perror("accept");
				else
				{
					FD_SET(sock2, &master); // add to master set
                        if (sock2 > fdmax)
						{    // keep track of the maximum
                            fdmax = sock2;
                        }
						cout << "connection made" << endl;
				}
			}
			else
			{
				rc = handle_connection(i);
				fdmax = sock;
			}
		}
		
		
	}
	/*
		// connection request arrived
	if(FD_ISSET(sock, &read_fds))
	{
		addrlen = sizeof(sa2);
		if((sock2 = accept(sock, (struct sockaddr *) &sa2, (socklen_t *) &addrlen)) == -1)
			perror("accept");
		else
		{
			FD_ZERO(&read_fds);
			FD_SET(sock2, &read_fds);
			if(sock2 > fdmax)
				fdmax = sock2;
			cout << "connection made" << endl;
			
		}
	}
	if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(1);
	}	
		
    // handle connections 
	if(FD_ISSET(sock2, &read_fds))
	{
		rc = handle_connection(sock2);
		
	}
	
	*/
	
  }
		
}

int handle_connection(int sock2)
{
  char filename[FILENAMESIZE+1];
  int rc;
  int fd;
  struct stat filestat;
  char buf[BUFSIZE+1];
  char *headers;
  char *endheaders;
  char *bptr;
  int datalen=0;
  char *ok_response_f = "HTTP/1.0 200 OK\r\n"\
                      "Content-type: text/plain\r\n"\
                      "Content-length: %d \r\n\r\n";
  char ok_response[100];
  char *notok_response = "HTTP/1.0 404 FILE NOT FOUND\r\n"\
                         "Content-type: text/html\r\n\r\n"\
                         "<html><body bgColor=black text=white>\n"\
                         "<h2>404 FILE NOT FOUND</h2>\n"
                         "</body></html>\n";
  bool ok=true;
	FILE * file;
	
	cout << "hello" << endl;
	
  /* first read loop -- get request and headers*/
	if(read(sock2, buf, BUFSIZE) < 0){
		cout << "1" << endl;
		perror("read");
		ok = false;
	}
	else
	{
		cout << "2" <<endl;
		cout << buf << endl;
		/* parse request to get file name */
		/* Assumption: this is a GET request and filename contains no spaces*/
		for(int i =4; buf[i] != ' '; i++)
		{
			filename[i-4] = buf[i];
		}
		cout << filename << endl;
	
	
		/* try opening the file */
		if((file = fopen(filename, "r")) == NULL){
		perror("opening file");
		ok = false;
		}
		cout << "file = " << file <<  endl;
	}

  /* send response */
  if (ok)
  {
	  cout << "3" << endl;
	  
    /* send headers */
	int n = write(sock2, ok_response_f, strlen(ok_response_f));
	cout << "bytes written 1st time = " << n << endl;
    /* send file */	
	fread(ok_response, sizeof(char), 100, file);		//extract file into ok_response
	cout << "ok response = " << ok_response << endl;	
	
	
	n = write(sock2, ok_response, 100); 					//write file contents
		cout << "bytes written 1st time = " << n << endl;
  }
  else // send error response
  {
	  cout << "4" << endl;
	  write(sock2,notok_response,strlen(notok_response));
  }

  /* close socket and free space */
	close(sock2);
  
  if (ok)
    return 0;
  else
    return -1;
}

//changed minet_read to read
int readnbytes(int fd,char *buf,int size)
{
  int rc = 0;
  int totalread = 0;
  while ((rc = minet_read(fd,buf+totalread,size-totalread)) > 0)
    totalread += rc;

  if (rc < 0)
  {
    return -1;
  }
  else
    return totalread;
}

int writenbytes(int fd,char *str,int size)
{
  int rc = 0;
  int totalwritten =0;
  while ((rc = minet_write(fd,str+totalwritten,size-totalwritten)) > 0)
    totalwritten += rc;

  if (rc < 0)
    return -1;
  else
    return totalwritten;
}

