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

/* bitmask: 00000001 (prima colonna dell'output) */
#define LEADING_ZERO_BITMASK_IDX_0 1 
#define LEADING_ZERO_BITMASK_IDX_1 2 
#define LEADING_ZERO_BITMASK_IDX_2 4 

#define CLOSE_AND_RETURN(retval, obj) \
	mysql_stmt_close(obj); \
	return (retval);


extern MYSQL *conn;


struct configuration {
	char host[BUFFSIZE_L];
	char username[BUFFSIZE_L];
	char password[BUFFSIZE_L];
	unsigned int port;
	char database[BUFFSIZE_L];
};


bool parse_config(const char *path, struct configuration *conf, const char *delimiter); 
void print_error(MYSQL *conn, char *message);
size_t get_input(unsigned int length, char *string, bool hide, bool not_null);
void init_screen(bool);
bool setup_prepared_stmt(MYSQL_STMT **stmt, char *statement, MYSQL *conn);
void print_stmt_error (MYSQL_STMT *stmt, char *message);
char multi_choice(const char *question, const char *choices, int no_choices);
void run_as_customer(char *username, char *customer_code, bool is_private, bool first_access);
void run_as_manager(char *username);
void run_as_chief_of_staff(char *username);
void run_as_warehouse_clerk(char *username);
void run_as_order_processor(char *username);
bool dump_result_set(MYSQL_STMT *stmt, char *title, int leading_zeros_bitmask);
void change_password(char *username);
void species_tips(unsigned int dots);
bool ask_for_tips(const char *message, unsigned int dots);
int format_prompt(char *dest, size_t length, const char *src, unsigned int dots);
bool attempt_search_species(bool only_flowery, char *name);
bool exec_sp(MYSQL_STMT **stmt_ptr, MYSQL_BIND *param, char *sp_name);
bool fetch_res_sp(MYSQL_STMT *stmt, MYSQL_BIND *param);