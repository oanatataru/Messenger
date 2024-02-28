#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;

int port;

int main (int argc, char *argv[])
{
  int sd;			//descriptorul de socket
  struct sockaddr_in server;	//structura folosita pentru conectare 
  char msg[1000];		//mesajul trimis

  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  //stabilim portul
  port = atoi (argv[2]);

  //cream socketul
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons (port);
  
  //conectare la server
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare - connect().\n");
      return errno;
    }

  printf("[client]Meniu: login | exit\n");
	fflush(stdout);

  while(1)
  {
    //citire mesaj
    bzero (msg, 1000);
    printf ("[client]Introduceti o comanda: ");
    fflush (stdout);
    read (0, msg, 1000);
  
    /* trimiterea mesajului la server */
    if (write (sd, msg, 1000) <= 0)
      {
        perror ("[client]Eroare la write() spre server.\n");
        return errno;
      }

    //citire raspuns de la server (apel blocant pana cand serverul raspunde) 
    if (read (sd, msg, 1000) < 0)
      {
        perror ("[client]Eroare - read() de la server.\n");
        return errno;
      }

    
    if (msg[0] == '+')
    {
      sleep(1);
      strcpy(msg, msg+1);
      printf("\033[A\033[K");
      fflush(stdout);
    
      printf("%s\n", msg); 
      fflush(stdout);
    }
    else
    {
      printf ("\n%s\n", msg);
    }
    if (strstr(msg, "Exited succesfully"))
      {
        close(sd);
        break;
      }
  }
  
}