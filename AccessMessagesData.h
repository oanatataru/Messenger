//AccessMessagesData.h

#ifndef ACCESSMESSAGESDATA_H
#define ACCESSMESSAGESDATA_H

#include <mysql/mysql.h>
#include <cstdio>
#include <string.h>

class AccessMessagesData {
    MYSQL *connection; 
    public:
    AccessMessagesData ()
    {
        connection = mysql_init(NULL);
        if (!mysql_real_connect(connection, "localhost", "root", "TataruOV2004", "offline_messenger", 0, NULL, 0)) {
            fprintf(stderr, " ERROR: %s\n", mysql_error(connection));
            exit(1);
        } 
    }

    bool insertMessage(char username1[50], char username2[50], char message[100]) 
    {
        if (connection) 
        {
            char command[500];
            snprintf(command, sizeof(command), "INSERT INTO Messages (id, message, username_sender, username_receiver, send_time, previous_message) VALUES (NULL, '%s', '%s', '%s', NOW(), '-')", message, username1, username2);
            mysql_query(connection, command);
        }
        return NULL;
    }

    bool searchIdMessage(int id)
    {
        if (connection)
        {
            char command[100];
            snprintf(command, sizeof(command), "SELECT * FROM Messages WHERE id='%d'", id);

            if (mysql_query(connection, command)) {
                return NULL;
            }

            MYSQL_RES *result = mysql_store_result(connection);
            MYSQL_ROW row = mysql_fetch_row(result);
            if (!row) {
                mysql_free_result(result);
                return 0;
            }

            mysql_free_result(result);
            return 1;
        }
        return NULL;
    }

    const char* searchMessageById(int id)
    {
        if (connection)
        {
            char command[100];
            snprintf(command, sizeof(command), "SELECT message FROM Messages WHERE id='%d'", id);

            if (mysql_query(connection, command)) {
                return NULL;
            }

            MYSQL_RES *result = mysql_store_result(connection);
            MYSQL_ROW row = mysql_fetch_row(result);
            if (!row) {
                mysql_free_result(result);
                return 0;
            }

            char* prevMessage = strdup(row[0]);
            mysql_free_result(result);
            return prevMessage;
        }
        return NULL;
    }

    bool insertReplyMessage(char username1[50], char username2[50], char message[100], char prevmessage[100]) 
    {
        if (connection) 
        {
            char command[500];
            snprintf(command, sizeof(command), "INSERT INTO Messages (id, message, username_sender, username_receiver, send_time, previous_message) VALUES (NULL, '%s', '%s', '%s', NOW(), '%s')", message, username1, username2, prevmessage);
            mysql_query(connection, command);
        }
        return NULL;
    }

    const char *getMessagesBetweenXY(char username1[], char username2[]) 
    {
        if (connection) 
        {
            char command[200];
            snprintf(command, sizeof(command), "SELECT id, previous_message, message FROM Messages WHERE username_sender IN ('%s', '%s') AND username_receiver IN ('%s', '%s')" , username1, username2, username1, username2);

            if (mysql_query(connection, command) == 0) 
            {
                MYSQL_RES *result = mysql_store_result(connection);
                if (result) 
                {
                    MYSQL_ROW row;
                    char *messages = new char[1000]; 

                    messages[0] = '\0';
                    strcat(messages, "Meniu: send | reply | exitchat\n");
                    while ((row = mysql_fetch_row(result))) 
                    {
                        strcat(messages, row[0]); 
                        strcat(messages, " ");
                        if (strcmp(row[1], "-") != 0)
                        {
                            strcat(messages, "["); 
                            strcat(messages, row[1]);
                            strcat(messages, "] ");
                        }
                        strcat(messages, row[2]);
                        strcat(messages, "\n");
                    }
                    
                    mysql_free_result(result);
                    return messages;
                } 
            } 
        } 
        return NULL;
    }

    const char *getLastMessageBetweenXY(char username1[], char username2[]) 
    {
        if (connection) 
        {
            char command[200];
            snprintf(command, sizeof(command), "SELECT id, previous_message, message FROM Messages WHERE username_sender IN ('%s', '%s') AND username_receiver IN ('%s', '%s')" , username1, username2, username1, username2);

            if (mysql_query(connection, command) == 0) 
            {
                MYSQL_RES *result = mysql_store_result(connection);
                if (result) 
                {
                    MYSQL_ROW row;
                    char *messages = new char[1000]; 

                    messages[0] = '\0';
                    while ((row = mysql_fetch_row(result))) {
                        strcpy(messages, row[0]); 
                        strcat(messages, " ");
                        if (strcmp(row[1], "-") != 0)
                        {
                            strcat(messages, "[");
                            strcat(messages, row[1]);
                            strcat(messages, "] ");
                        }
                        strcat(messages, row[2]);
                    }
                    strcat(messages, "                   ");
                    mysql_free_result(result);
                    return messages;
                } 
            } 
        } 
        return NULL;
    }

    const char *getLastTimestamp (char username1[], char username2[]) 
    {
        if (connection) 
        {
            char command[200];
            snprintf(command, sizeof(command), "SELECT send_time FROM Messages WHERE username_sender IN ('%s', '%s') AND username_receiver IN ('%s', '%s')" , username1, username2, username1, username2);

            if (mysql_query(connection, command) == 0) 
            {
                MYSQL_RES *result = mysql_store_result(connection);
                if (result) 
                {
                    MYSQL_ROW row;
                    char *lastDate = new char[1000]; 

                    lastDate[0] = '\0';
                    while ((row = mysql_fetch_row(result))) {
                        strcpy(lastDate, row[0]);
                    }
                    mysql_free_result(result);
                    return lastDate;
                } 
            } 
        } 
        return NULL;
    }

    const char *getMessagesAfterLastDate(char username1[], char username2[], char lastDate[]) 
    {
        if (connection) 
        {
            char command[200];
            snprintf(command, sizeof(command), "SELECT id, previous_message, message FROM Messages WHERE username_sender IN ('%s', '%s') AND username_receiver IN ('%s', '%s') AND send_date > '%s'" , username1, username2, username1, username2, lastDate);

            if (mysql_query(connection, command) == 0) 
            {
                MYSQL_RES *result = mysql_store_result(connection);
                if (result) 
                {
                    MYSQL_ROW row;
                    char *messages = new char[1000]; 

                    messages[0] = '\0';
                    while ((row = mysql_fetch_row(result))) {
                        strcat(messages, row[0]); 
                        strcat(messages, " ");
                        if (strcmp(row[1], "-") != 0)
                        {
                            strcat(messages, "[");
                            strcat(messages, row[1]);
                            strcat(messages, "] ");
                        }
                        strcat(messages, row[2]);
                        strcat(messages, "\n");
                    }
                    if (messages[0]) {
                        strcpy(messages, "no new messages");
                    }
                    mysql_free_result(result);
                    return messages;
                } 
            } 
        } 
        return NULL;
    }
};

#endif