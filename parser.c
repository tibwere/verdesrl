#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

#define LINE_SIZE 1024

int parse_config(const char *path, config_t *conf, const char * delimiter) 
{
    char line[LINE_SIZE];
    char *key;
    char *value;
    int no_lines = 0;

    FILE *file = fopen(path, "r");
    if (file == NULL) 
        return 1;

    while (fgets(line, LINE_SIZE, file) != NULL && no_lines < 5) 
    {   
        ++ no_lines;
        key = strtok(line, delimiter);
        value = strtok(NULL, delimiter);
        value[strcspn(value, "\n")] = '\0'; // necessario per rimuovere il '\n' finale

        if (strcmp(key, "host") == 0) 
            strncpy(conf->host, value, DBHOST_LENGTH);
        else if (strcmp(key, "username") == 0)
            strncpy(conf->username, value, DBUSER_LENGTH);
        else if (strcmp(key, "password") == 0)
            strncpy(conf->password, value, DBPASS_LENGTH);
        else if (strcmp(key, "database") == 0)
            strncpy(conf->database, value, DBNAME_LENGTH); 
        else if (strcmp(key, "port") == 0)
            conf->port = strtol(value, NULL, 10);  
        else 
            return 1;
    }

    return (no_lines != 5);
}