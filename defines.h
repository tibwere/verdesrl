#pragma once

#include <stdio.h>
#include <mysql.h>
#include <stdbool.h>

/* dimensione effettiva +1 (per '\0') */
#define BUFFSIZE_XS 17 
#define BUFFSIZE_S	33
#define BUFFSIZE_M	65
#define BUFFSIZE_L	129
#define BUFFSIZE_XL	257

#define CLOSEANDRET(x) \
            mysql_stmt_close(stmt); \
            return (x);

extern MYSQL *conn;

typedef struct configuration 
{
	char host[BUFFSIZE_L];
	char username[BUFFSIZE_L];
	char password[BUFFSIZE_L];
	unsigned int port;
	char database[BUFFSIZE_L];
} config_t;

int parse_config(const char *path, config_t *conf, const char *delimiter); 
void print_error(MYSQL *conn, char *message);
size_t get_input(unsigned int length, char *string, bool hide, bool not_null);
void init_screen(bool);
bool setup_prepared_stmt(MYSQL_STMT **stmt, char *statement, MYSQL *conn);
void print_stmt_error (MYSQL_STMT *stmt, char *message);
char multi_choice(const char *question, const char *choices, int no_choices);
void run_as_customer(char *username, char *customer_code, bool is_private, bool first_access);
bool dump_result_set(MYSQL_STMT *stmt, char *title, int leading_zeros_bitmask);