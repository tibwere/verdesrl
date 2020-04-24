#pragma once

#include <stdio.h>
#include <mysql.h>
#include <stdbool.h>

#include "sizes.h"

#define DBHOST_LENGTH 	64
#define DBNAME_LENGTH 	64
#define DBUSER_LENGTH 	64
#define DBPASS_LENGTH	64

typedef struct configuration 
{
	char host[DBHOST_LENGTH];
	char username[DBUSER_LENGTH];
	char password[DBPASS_LENGTH];
	unsigned int port;
	char database[DBNAME_LENGTH];
} config_t;

int parse_config(const char *path, config_t *conf, const char *delimiter); 
void print_error(MYSQL *conn, char *message);
size_t get_input(unsigned int length, char *string, bool hide);
void init_screen(bool);
bool setup_prepared_stmt(MYSQL_STMT **stmt, char *statement, MYSQL *conn);
void print_stmt_error (MYSQL_STMT *stmt, char *message);
char multi_choice(const char *question, const char *choices, int no_choices);
