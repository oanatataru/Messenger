#include "AccessMessagesData.h"
#include "AccessFromUsers.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mysql/mysql.h>

int PORT = 2024;

AccessFromUsers accesFromUsers;
AccessMessagesData accessMessagesData;


/*-------------------------VALIDARI--------------------------------*/


bool validRegister(char command[]) 
{
    char *p = strtok(command, " ");

    if (p == nullptr || strcmp(p, "register") != 0) return false;

    char *username = strtok(nullptr, " ");
    char *password = strtok(nullptr, " ");
    char *confirmPassword = strtok(nullptr, " ");
	if (confirmPassword[strlen(confirmPassword)-1] == '\n')
		confirmPassword[strlen(confirmPassword)-1] = '\0';

    if (!username || !password || !confirmPassword || strcmp(password, confirmPassword) != 0) return false;

    if (strtok(nullptr, " ") != NULL) return false;
    
    return true;
}

bool validLogin(char command[]) 
{
    char *p = strtok(command, " ");

    if (p == nullptr || strcmp(p, "login") != 0) return false;

    char *username = strtok(nullptr, " ");
    char *password = strtok(nullptr, " ");

    if (!username || !password) return false;

    if (strtok(nullptr, " ") != NULL) return false;
    
    return true;
}

bool validEnterchat(char command[])
{
    char *p = strtok(command, " ");

    if (p == nullptr || strcmp(p, "enterchat") != 0) return false;

    char *username = strtok(nullptr, " ");

    if (!username) return false;

    if (strtok(nullptr, " ") != NULL) return false;
    
    return true;
}

bool validSend(char command[]) 
{
    if (strncmp(command, "send[", 5) == 0) 
    {
        if (strchr(command + 5, ']') != NULL && strchr(command + 5, ']') > command + 5) // daca exista ] si exista [..]
        {
            if (*(strchr(command + 5, ']') + 1) == '\0') //nu este nimic dupa ]
            {
                return true;
            }
        }
    }
    return false;
}

bool validReply(char command[]) 
{
	if (strncmp(command, "reply ", 6) == 0) 
    {
		strcpy(command, command + 6);
		
		if (command[0] == '\n')
			return false;

        for (int i = 0; command[i] != ' '; i++)
        {
            if (!isdigit(command[i]))
            {
                return false;
            }
        }
        
        if (strchr(command, '[') != NULL && strchr(command, ']') > command) // daca exista ] si exista [..]
        {
			if (*(strchr(command, ']') + 1) == '\0') //nu este nimic dupa ]
            {
                return true;
            }
        }
    }
	return false;
}


/*---------------------------GET INFO FROM COMMNAND-----------------------------*/


char* getUsernameFromCommand(char command[]) {
    char *p = strtok(command, " ");

    char *username = strtok(nullptr, " ");
    
    return username;
}

char* getPasswordFromCommand(char command[]) {
    char *p = strtok(command, " ");
    p = strtok(nullptr, " ");
    char *password = strtok(nullptr, " ");
    
    return password;
}


/*------------------------------MAIN--------------------------------*/


int main()
{
	struct sockaddr_in server;  //structura folosita de server
	struct sockaddr_in from;
	char msgFromClient[1000];			 
	char msgToClient[1000] = " ";         
	int sd;					 // descriptorul de socket
	int optval = 1;

	//Creare socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("[server]Eroare - socket().\n");
		return errno;
	}

    // reutilizarea aceluiasi port
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); 

	bzero(&server, sizeof(server));
	bzero(&from, sizeof(from));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY); //accepta orice adresa
	server.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
		perror("[server]Eroare - bind().\n");
		return errno;
	}

	if (listen(sd, 1) == -1)
	{
		perror("[server]Eroare - listen().\n");
		return errno;
	}

	while (1)
	{
		int client;
		socklen_t length = sizeof(from);
		int logged = 0;
		int chat = 0;

		printf("[server]Asteptam la portul %d...\n", PORT);
		fflush(stdout);

		//acceptam un client (stare blocanta pana la realizarea conexiunii)
		client = accept(sd, (struct sockaddr *)&from, &length);

		if (client < 0)
		{
			perror("[server]Eroare la accept().\n");
			continue;
		}

		int pid;
		if ((pid = fork()) == -1)
		{
			close(client);
			continue;
		}
		else if (pid > 0) //parinte
		{
			close(client);
			continue;
		}
		else if (pid == 0) //copil
		{
		    char userlogged[50];
			char userchat[50];
			char lastDate[50];
			lastDate[0]='\0';
			
			
			while(1)
			{
                close(sd);
				
				//clientul s-a conectat, se asteapta mesajul
				bzero(msgFromClient, 100);
				printf("[server]Asteptam mesajul...\n");
				fflush(stdout);

				//citire mesaj
				if (read(client, msgFromClient, 100) <= 0)
				{
					perror("[server]Eroare - read() de la client.\n");
					close(client); /* inchidem conexiunea cu clientul */
					continue;	   /* continuam sa ascultam */
				}
				bzero(msgToClient, 100);

				bool ok = 0;


				/*----------------------------REGISTER--------------------------*/


				if (strstr(msgFromClient, "register"))
				{
					// verifica sintaxa lui register: register username parola parola
					// + verif parolele coincid

					char command1[200], command2[200];
					strcpy(command1, msgFromClient);
					strcpy(command2, msgFromClient);

					if (validRegister(command1))
					{
						if (logged ==  1)
						{
							if (chat == 0)
							{
								ok = 1;
								if (write(client, "Logout first to register another account or continue on the current account\nMeniu: enterchat | users | logout\n", 111) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else
							{
								ok = 1;
								if (write(client, "Exit the current chat first\nMeniu: exitchat | send | reply\n", 60) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
						}
						else
						{
							char userPasswd[50];
							char username[50], parola[50];
							username[0]='\0'; parola[0]='\0';
							int cnt=0;
							
							char *p = strtok(command2, " ");
							while (p!=NULL)
							{
								if (cnt == 1) {strcpy(username, p);}
								else if (cnt == 2) {strcpy(parola, p);}
								cnt++;
								p = strtok(NULL, " ");
							}
							if (parola[strlen(parola)-1]=='\n')
								parola[strlen(parola)-1]='\0';

							strcpy(userPasswd, accesFromUsers.getPasswdForUser(username));
							
							if (strcmp(userPasswd, "!!!") == 0)
							{
								//introdu new user in baza de date
								accesFromUsers.insertUser(username, parola);

								logged = 1;
								strcpy(userlogged, username);
								ok = 1;
								if (write(client, "New account created and logged in succesfully\nMeniu: logout | enterchat | users\n", 81) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else if (strcmp(parola, userPasswd) != 0)
							{
								ok = 1;
								if (write(client, "The user exists already, but the password is wrong. Enter the right password or create another account, with a different username\nMeniu: register | login | exit\n", 162) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else if (strcmp(parola, userPasswd) == 0)
							{
								logged = 1;
								strcpy(userlogged, username);
								ok = 1;
								if (write(client, "Account already existent, but you logged in succesfully\nMeniu: logout | enterchat | users\n", 91) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; //continuam sa ascultam
								}
							}
						}
					}
					else
					{
						ok = 1;
						if (write(client, "Invalid command. Write it like this: register username password password\n", 74) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------LOGIN--------------------------*/


				else if (strstr(msgFromClient, "login"))
				{
					// verifica sintaxa lui login: login username parola
					
					char command1[200], command2[200];
					strcpy(command1, msgFromClient);
					strcpy(command2, msgFromClient);

					if (validLogin(command1))
					{
						if (logged ==  1)
						{
							if (chat == 0)
							{
								ok = 1;
								if (write(client, "Logout first or continue on the current account\nMeniu: logout | enterchat\n", 75) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam 
								}
							}
							else
							{
								ok = 1;
								if (write(client, "Exit chat first\nMeniu: exitchat | send | reply\n", 48) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam 
								}
							}
						}
						else
						{
							//verifica daca exista username si parola
							char username[50], parola[50];
							username[0]='\0'; parola[0]='\0';
							int cnt=0;
							
							char *p = strtok(command2, " ");
							while (p!=NULL)
							{
								if (cnt == 1) {strcpy(username, p);}
								else if (cnt == 2) {strcpy(parola, p);}
								cnt++;
								p = strtok(NULL, " ");
							}
							if (parola[strlen(parola)-1]=='\n')
								parola[strlen(parola)-1]='\0';

							char userPasswd[50];
							strcpy(userPasswd, accesFromUsers.getPasswdForUser(username));
							
							if (strcmp(parola, userPasswd) == 0)
							{
								logged = 1;
								strcpy(userlogged, username);
								ok = 1;
								if (write(client, "Logged in succesfully\nMeniu: enterchat | logout | users\n", 57) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam 
								}
							}
							else if (strcmp(userPasswd, "!!!") == 0)
							{
								ok = 1;
								if (write(client, "This account doesn't exist. Log in another account or create a new account\nMeniu: login | register | exit\n", 107) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else if (strcmp(parola, userPasswd) != 0)
							{
								ok = 1;
								if (write(client, "Incorrect password\nMeniu: login | register | exit\n", 51) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam 
								}
							}
						}
					}
					else 
					{
						ok = 1;
						if(write(client, "Invalid command. Write it like this: login username password\n", 62) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------LOGOUT--------------------------*/


				else if (strstr(msgFromClient, "logout"))
				{
					if (logged ==  0)
					{
						ok = 1;
						if (write(client, "You weren't logged in, all good\nMeniu: register | login | exit\n", 64) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
					else
					{
						if (chat == 0)
						{
							logged = 0;
							strcpy(userlogged, "\0");
							ok = 1;
							if (write(client, "Logged out succesfully\nMeniu: register | login | exit\n", 55) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
						else
						{
							ok = 1;
							if (write(client, "Exit chat first\nMeniu: exitchat | send | reply\n", 48) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
						
					}
				}


				/*----------------------------SHOWUSERS--------------------------*/


				else if (strstr(msgFromClient, "showusers"))
				{
					if (logged == 1)
					{
						if (chat == 0)
						{
							auto otherUsernames = accesFromUsers.getOtherUsernames(userlogged);
							ok = 1;
							if (write(client, otherUsernames, strlen(otherUsernames)) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
						else
						{
							ok = 1;
							if (write(client, "Exit chat first\nMeniu: exitchat | send | reply\n", 48) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else
					{
						ok = 1;
						if (write(client, "Log in first\nMeniu: register | login | exit", 44) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}

				}


				/*----------------------------ENTERCHAT--------------------------*/


				else if (strstr(msgFromClient, "enterchat")) 
				{
					//verifica sintaxa: enterchat to_username
					
					char command1[200], command2[200];
					strcpy(command1, msgFromClient);
					strcpy(command2, msgFromClient);

					if (validEnterchat(command1))
					{
						if (logged == 1)
						{
							if (chat == 0)
							{
								//verifica daca exista username + diferit de userul logat (nu faci convo cu el insusi)
								char username[50];
								username[0]='\0'; 
								int cnt=0;
								
								char *p = strtok(command2, " ");
								while (p!=NULL)
								{
									if (cnt == 1) {strcpy(username, p);}
									cnt++;
									p = strtok(NULL, " ");
								}
								if (username[strlen(username)-1]=='\n')
									username[strlen(username)-1]='\0';

								if (strcmp(userlogged, username) == 0)
								{
									ok = 1;
									if (write(client, "You can't enter in a chat with yourself\n", 41) <= 0)
									{
										perror("[server]Eroare - write() catre client.\n");
										continue; // continuam sa ascultam
									}
								}
								else if (accesFromUsers.searchUsername(username) == 0)
								{
									ok = 1;
									if (write(client, "The user you want to chat with doesn't exist\n", 46) <= 0)
									{
										perror("[server]Eroare - write() catre client.\n");
										continue; // continuam sa ascultam
									}
								}
								else
								{
									chat = 1;
									strcpy(userchat, username);
									ok = 1;
									auto messages = accessMessagesData.getMessagesBetweenXY(userlogged, userchat);
									strcpy(lastDate, accessMessagesData.getLastTimestamp(userchat, userlogged));
									//if (write(client, "Entered in chat successfully\nMeniu: send | reply | exitchat\n", 43) <= 0)
									if (write(client, messages, strlen(messages)) <= 0)
									{
										perror("[server]Eroare - write() catre client.\n");
										exit; // continuam sa ascultam
									}
								}
							}
							else
							{
								ok = 1;
								if (write(client, "You are in another chat. Exit the current one first\nMeniu: exitchat | send | reply\n", 84) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
						}
						else
						{
							ok = 1;
							if (write(client, "Log in first\nMeniu: enterchat | users | logout\n", 48) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else 
					{
						ok = 1;
						if(write(client, "Invalid command. Write it like this: enterchat username\n", 57) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------EXITCHAT--------------------------*/


				else if (strstr(msgFromClient, "exitchat"))
				{
					// nu verificam sintaxa (strcmp deja)

					if (logged == 1)
					{
						if (chat == 0)
						{
							ok = 1;
							if (write(client, "You're not in a chat\nMeniu: enterchat | users | logout\n", 56) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultaM
							}
						}
						else
						{
							chat = 0;
							strcpy(userchat, "\0");
							ok = 1;
							if (write(client, "Chat exited succesfully\nMeniu: enterchat | users | logout\n", 59) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else
					{
						ok = 1;
						if (write(client, "You're not even logged in\nMeniu: register | login | exit\n", 43) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------HISTORY--------------------------*/


				else if (strstr(msgFromClient, "history"))
				{
					// nu verificam sintaxa (strcmp deja)

					if (logged == 1)
					{
						if (chat == 0)
						{
							ok = 1;
							if (write(client, "You're not in a chat\nMeniu: enterchat | users | logout\n", 56) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultaM
							}
						}
						else
						{
							auto history = accessMessagesData.getMessagesBetweenXY(userchat, userlogged);
							if (write(client, history, strlen(history)) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else
					{
						ok = 1;
						if (write(client, "You're not even logged in\nMeniu: register | login | exit\n", 43) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------SEND--------------------------*/


				else if (strstr(msgFromClient, "send"))
				{
					//verifica sentaxa: send[...]

					char command1[200], command2[200];
					strcpy(command1, msgFromClient);
					strcpy(command2, msgFromClient);

					if (command1[strlen(command1)-1] == '\n')
						command1[strlen(command1)-1] = '\0';
					if (validSend(command1))
					{
						if (logged ==  1)
						{
							if (chat == 0)
							{
								if (write(client, "Enter a chat first\nMeniu: enterchat | logout\n", 43) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else
							{
								// insereaza mesaj in baza de date

								char message[100], message1[100];
								strncpy(message, strchr(command2, '[') + 1, strchr(command2, ']') - strchr(command2, '[') - 1);
    							message[strchr(command2, ']') - strchr(command2, '[') - 1] = '\0';
								accessMessagesData.insertMessage(userlogged, userchat, message);

								message1[0]='+';
								message1[1]='\0';
								strcat(message1, accessMessagesData.getLastMessageBetweenXY(userchat, userlogged));
								
								//if (write(client, "Message sent\nMeniu: send | reply | exitchat\n", 43) <= 0)
								if (write(client, message1, strlen(message1)) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
						}
						else
						{
							if (write(client, "Log in first\nMeniu: register | login | exit\n", 43) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else 
					{
						ok = 1;
						if(write(client, "Invalid command. Write it like this: send[message]\n", 52) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------REPLY--------------------------*/


				else if (strstr(msgFromClient, "reply"))
				{
					// verifiva sintaxa reply: reply id [...]

					char command1[200], command2[200];
					strcpy(command1, msgFromClient);
					strcpy(command2, msgFromClient);

					if (command1[strlen(command1)-1] == '\n')
						command1[strlen(command1)-1] = '\0';
					
					if(validReply(command1))
					{
						if (logged ==  1)
						{
							if (chat == 0)
							{
								if (write(client, "Enter a chat first\nMeniu: enterchat | logout\n", 43) <= 0)
								{
									perror("[server]Eroare - write() catre client.\n");
									continue; // continuam sa ascultam
								}
							}
							else
							{
								// search for id in baza de date
								int id = 0;
								strcpy(command2, command2 + 6);

								int i=0;
								for (i = 0; command2[i] != ' '; i++)
								{
									id = id * 10 + (command2[i] - '0');
								}
								
								strcpy(command2, command2+i+2);
								strcpy(command2+strlen(command2)-2, command2+strlen(command2));

								if (accessMessagesData.searchMessageById(id))
								{
									// inserez mesajul nou, si pun in colaoana cu 'ancestor_message' mesajul cu id-ul ala
									
									char prevmsg[100];
									strcpy(prevmsg, accessMessagesData.searchMessageById(id));
									accessMessagesData.insertReplyMessage(userlogged, userchat, command2, prevmsg);

									char message1[100];
									message1[0]='+';
									message1[1]='\0';
									strcat(message1, accessMessagesData.getLastMessageBetweenXY(userchat, userlogged));
								
									if (write(client, message1, strlen(message1)))
									{
										perror("[server]Eroare - write() catre client.\n");
										continue; // continuam sa ascultam
									}
								}
								else
								{
									if (write(client, "The message you want to reply to doesn't exist\nMeniu: send | reply | history | exitchat\n", 89) <= 0)
									{
										perror("[server]Eroare - write() catre client.\n");
										continue; // continuam sa ascultam
									}
								}
								
							}
						}
						else
						{
							if (write(client, "Log in first\nMeniu: register | login | exit\n", 43) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else 
					{
						ok = 1;
						if(write(client, "Invalid command. Write it like this: reply id []\n", 50) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
					}
				}


				/*----------------------------EXIT--------------------------*/


				else if (strstr(msgFromClient, "exit"))
				{
					if (logged ==  1)
					{
						if (chat == 0)
						{
							ok = 1;
							if (write(client, "Exit chat first and logout\nMeniu: exitchat | send | reply\n", 50) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
						else 
						{
							ok = 1;
							if (write(client, "Log out first\nMeniu: logout | enterchat | users\n", 49) <= 0)
							{
								perror("[server]Eroare - write() catre client.\n");
								continue; // continuam sa ascultam
							}
						}
					}
					else
					{
						ok = 1;
						if (write(client, "Exited succesfully\n", 20) <= 0)
						{
							perror("[server]Eroare - write() catre client.\n");
							continue; // continuam sa ascultam
						}
						close(client);
					    exit(0);
					}
				}

			}
		}

        /* am terminat cu acest client, inchidem conexiunea */
		close(client);
		exit(0);
	} 
}