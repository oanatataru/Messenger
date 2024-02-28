//AcessFromUsers.h

#ifndef ACCESSFROMUSERS_H
#define ACCESSFROMUSERS_H

#include <mysql/mysql.h>
#include <cstdio>
#include <string.h>

class AccessFromUsers {
    MYSQL *connection;
    public:
    AccessFromUsers ()
    {
        connection = mysql_init(NULL);
        if (!mysql_real_connect(connection, "localhost", "root", "TataruOV2004", "offline_messenger", 0, NULL, 0)) {
            fprintf(stderr, " ERROR: %s\n", mysql_error(connection));
            exit(1);
        }          
    }

    char* getPasswdForUser(const char* username) 
    {
        if (connection)
        {
            char command[100];
            snprintf(command, sizeof(command), "SELECT password FROM Users WHERE username='%s'", username);

            if (mysql_query(connection, command)) {
                return NULL;
            }

            MYSQL_RES *result = mysql_store_result(connection);
            MYSQL_ROW row = mysql_fetch_row(result);
            if (!row) {
                mysql_free_result(result);
                const char* x = "!!!";
                char* rez = strdup(x);
                return rez;
            }

            char* userPassword = strdup(row[0]);
            mysql_free_result(result);
            return userPassword;
        }
        return NULL;
    }

    bool insertUser (char username[50], char password[50])
    {
        if (connection)
        {
            char command[100];
            snprintf(command, sizeof(command), "INSERT INTO Users (username, password) VALUES ('%s', '%s')", username, password);
            mysql_query(connection, command);
        }
        return NULL;
    }

    bool searchUsername(char username[50])
    {
        if (connection)
        {
            char command[100];
            snprintf(command, sizeof(command), "SELECT * FROM Users WHERE username='%s'", username);

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

    char *getOtherUsernames(const char *inputUsername) 
    {
        if (connection) 
        {
            char command[100];
            snprintf(command, sizeof(command), "SELECT username FROM Users WHERE username <> '%s'", inputUsername);

            if (mysql_query(connection, command) == 0) 
            {
                MYSQL_RES *result = mysql_store_result(connection);

                if (result) 
                {
                    MYSQL_ROW row;
                    char *otherUsernames = new char[1000]; 

                    otherUsernames[0] = '\0';

                    while ((row = mysql_fetch_row(result))) {
                        strcat(otherUsernames, row[0]); 
                        strcat(otherUsernames, "\n");
                    }
                    strcat(otherUsernames, "Meniu: enterchat | logout\n");

                    mysql_free_result(result);
                    return otherUsernames;
                } 
            } 
        } 
        return nullptr;
    }
};

#endif