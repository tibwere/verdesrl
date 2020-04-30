#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include "defines.h"

typedef enum { CLP, CLR, ADM, OPP, MNG, CPP, ERR } role_t;

typedef struct credentials
{
    char username[BUFFSIZE_L];
    char password[BUFFSIZE_L];
} credentials_t;

typedef struct customer_signup 
{
	credentials_t credentials;
	char code[BUFFSIZE_XS];
	char name[BUFFSIZE_S];
	char residential_address[BUFFSIZE_M];
	char billing_address[BUFFSIZE_M];
	char referent_first_name[BUFFSIZE_S];
	char referent_last_name[BUFFSIZE_S];
} customer_signup_t;


static role_t attempt_login(credentials_t *cred, char *identifier); 
static bool attempt_signup(customer_signup_t *cst, bool is_private);
static bool login_manager(void);
static bool signup_manager(void);
static char *strupper(char *str);
static bool attempt_change_password(char *username, char *old_passwd, char *new_passwd);


MYSQL *conn;


int main (void)
{
    char choice;
	config_t cnf;

    memset(&cnf, 0, sizeof(config_t));

    if (parse_config("config/nrg.user", &cnf, "="))
    {
        fprintf(stderr, "Invalid configuration file selected (NRG)\n");
        exit(EXIT_FAILURE);
    } 

    conn = mysql_init(NULL);
    if (conn == NULL)
    {
        fprintf(stderr, "Out of memory, connection was not established.");
        exit(EXIT_FAILURE);
    }

    if (mysql_real_connect (conn, cnf.host, cnf.username, cnf.password, cnf.database, cnf.port, NULL, CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS) == NULL) 
	{
		print_error(conn, "Something went wrong, connection was not established.");
		mysql_close (conn);
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		init_screen(false);
    	choice = multi_choice("Do you wanna [l]ogin or [s]ignup?", "ls", 2);

		if (choice == 'l') 
		{
			if (login_manager())
				break;
		}
		else if (choice == 's')
		{
			if (signup_manager())
				break;
		} 
		else
		{
			fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
			abort();
		} 
	}   
    
    mysql_close(conn);
    exit(EXIT_SUCCESS);
}

static role_t attempt_login(credentials_t *cred, char *identifier) 
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[4];
	int role;

	memset(param, 0, sizeof(param)); 

	if(!setup_prepared_stmt(&stmt, "call login(?, ?, ?, ?)", conn)) 
	{
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return ERR;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_username VARCHAR(128)
	param[0].buffer = cred->username;
	param[0].buffer_length = strlen(cred->username);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_password VARCHAR(128)
	param[1].buffer = cred->password;
	param[1].buffer_length = strlen(cred->password);

	param[2].buffer_type = MYSQL_TYPE_LONG; // OUT var_ruolo INT
	param[2].buffer = &role;
	param[2].buffer_length = sizeof(role);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING; // OUT var_codice_cliente VARCHAR(16)
	param[3].buffer = identifier;
	param[3].buffer_length = BUFFSIZE_XS * sizeof(char);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
		CLOSEANDRET(ERR);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
		CLOSEANDRET(ERR);
	}


	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_ruolo INT
	param[0].buffer = &role;
	param[0].buffer_length = sizeof(role);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // OUT var_codice_cliente VARCHAR(16)
	param[1].buffer = identifier;
	param[1].buffer_length = BUFFSIZE_XS * sizeof(char);
	
	if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Could not retrieve output parameter");
		CLOSEANDRET(ERR);
	}
	
	if(mysql_stmt_fetch(stmt)) 
	{
		print_stmt_error(stmt, "Could not buffer results");
		CLOSEANDRET(ERR);
	}

	mysql_stmt_close(stmt);
	return role;
}

static bool attempt_signup(customer_signup_t *cst, bool is_private) 
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[(is_private ? 6 : 8)];

	memset(param, 0, sizeof(param));

	if (is_private)
	{
		if(!setup_prepared_stmt(&stmt, "call registra_privato(?, ?, ?, ?, ?, ?)", conn)) 
		{
			print_stmt_error(stmt, "Unable to initialize the statement\n");
			return false;
		}
	}
	else
	{
		if(!setup_prepared_stmt(&stmt, "call registra_rivendita(?, ?, ?, ?, ?, ?, ?, ?)", conn)) 
		{
			print_stmt_error(stmt, "Unable to initialize the statement\n");
			return false;
		}
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_codice CHAR(16) / CHAR(11)
	param[0].buffer = cst->code;
	param[0].buffer_length = strlen(cst->code);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome VARCHAR(32)
	param[1].buffer = cst->name;
	param[1].buffer_length = strlen(cst->name);

	param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_res VARCHAR(64)
	param[2].buffer = cst->residential_address;
	param[2].buffer_length = strlen(cst->residential_address);

	if (strlen(cst->billing_address) > 0)
	{
		param[3].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_fat VARCHAR(64)
		param[3].buffer = cst->billing_address;
		param[3].buffer_length = strlen(cst->billing_address);
	}
	else
	{
		param[3].buffer_type = MYSQL_TYPE_NULL; // IN var_fat VARCHAR(64)
		param[3].buffer = NULL;
		param[3].buffer_length = 0;		
	}


	param[4].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_user VARCHAR(128)
	param[4].buffer = (cst->credentials).username;
	param[4].buffer_length = strlen((cst->credentials).username);

	param[5].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_pass VARCHAR(128)
	param[5].buffer = (cst->credentials).password;
	param[5].buffer_length = strlen((cst->credentials).password);

	if (!is_private)
	{
		param[6].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_ref VARCHAR(32)
		param[6].buffer = cst->referent_first_name;
		param[6].buffer_length = strlen(cst->referent_first_name);

		param[7].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cognome_ref VARCHAR(32)
		param[7].buffer = cst->referent_last_name;
		param[7].buffer_length = strlen(cst->referent_last_name);
	}

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
		CLOSEANDRET(false);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
		CLOSEANDRET(false);
	}

	mysql_stmt_close(stmt);
	return true;
}

static bool attempt_change_password(char *username, char *old_passwd, char *new_passwd) 
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[3];

	memset(param, 0, sizeof(param)); 

	if(!setup_prepared_stmt(&stmt, "call modifica_password(?, ?, ?)", conn)) 
	{
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_username VARCHAR(128)
	param[0].buffer = username;
	param[0].buffer_length = strlen(username);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_vecchia_password VARCHAR(128)
	param[1].buffer = old_passwd;
	param[1].buffer_length = strlen(old_passwd);

    param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nuova_password VARCHAR(128)
	param[2].buffer = new_passwd;
	param[2].buffer_length = strlen(new_passwd);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
		CLOSEANDRET(false);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
		CLOSEANDRET(false);
	}

	mysql_stmt_close(stmt);
	return true;
}

static bool login_manager(void)
{
	char client_identifier[BUFFSIZE_XS];
	credentials_t cred;
	role_t role;
	char choice;

	memset(&client_identifier, 0, sizeof(client_identifier));
    memset(&cred, 0, sizeof(cred));

	init_screen(false);

	printf("Insert username: ");
	get_input(BUFFSIZE_L, cred.username, false, true);
	printf("Insert password: ");
	get_input(BUFFSIZE_L, cred.password, true, true);

	role = attempt_login(&cred, client_identifier);
	
	switch (role)
	{
		case CLP: run_as_customer(cred.username, client_identifier, true, false); break;
		case CLR: run_as_customer(cred.username, client_identifier, false, false); break;
		case MNG: run_as_manager(cred.username); break;
		case ADM:
		case OPP:
		case CPP: printf("Sorry not implemented yet!\n"); break;
		case ERR: printf("Login failed!\n"); break;
		default: 
			fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
			abort();
	}

	if (role == ERR)
	{
		choice = multi_choice("Do you wanna quit? ", "yn", 2);
		return (choice == 'y');
	}

	return true;
}

static char *strupper(char *str)
{
	for (unsigned int i = 0; i < strlen(str); ++i)
		str[i] = toupper(str[i]);
		
	return str;
}

static bool signup_manager(void)
{
	char password_check[BUFFSIZE_L];
	customer_signup_t cst;
	char modality;
	char choice;
	
	memset(&cst, 0, sizeof(cst));
	memset(password_check, 0, sizeof(password_check));

	modality = multi_choice("Are you a [p]rivate or [r]etailer?", "pr", 2);

	if (modality != 'p' && modality != 'r')
	{
		fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
		abort();		
	}

	init_screen(false);

	printf("Insert username...........................: ");
	get_input(BUFFSIZE_L, (cst.credentials).username, false, true);

retype_pass:
	printf("Insert password...........................: ");
	get_input(BUFFSIZE_L, (cst.credentials).password, true, true);

	printf("Confirm password..........................: ");
	get_input(BUFFSIZE_L, password_check, true, true);

	if (strcmp((cst.credentials).password, password_check) != 0)
	{
		printf("Mismatch password, please retry!\n");
		goto retype_pass;
	}

	if (modality == 'p')
	{
		printf("Insert fiscal code........................: ");
		get_input(16, cst.code, false, true);
	}
	else
	{
		printf("Insert VAT code...........................: ");
		get_input(11, cst.code, false, true);
	}

	printf("Insert your name..........................: ");
	get_input(BUFFSIZE_S, cst.name, false, true);

	printf("Insert your residential address...........: ");
	get_input(BUFFSIZE_M, cst.residential_address, false, true);

	printf("Insert your billing address (default null): ");
	get_input(BUFFSIZE_M, cst.billing_address, false, false);

	if (modality == 'r')
	{
		printf("Insert referent first name...........: ");
		get_input(BUFFSIZE_S, cst.referent_first_name, false, true);

		printf("Insert referent last name:............:");
		get_input(BUFFSIZE_S, cst.referent_last_name, false, true);		
	}

	if (!attempt_signup(&cst, (modality == 'p')))
	{
		printf("Signup failed!\n");
		choice = multi_choice("Do you wanna quit? ", "yn", 2);
		return (choice == 'y');
	}

	run_as_customer((cst.credentials).username, strupper(cst.code), (modality == 'p'), true); 
	return true;
}

void change_password(char *username) 
{
	char old_passwd[BUFFSIZE_L];
	char new_passwd[BUFFSIZE_L];
	char passwd_check[BUFFSIZE_L];

	memset(old_passwd, 0, sizeof(old_passwd));
	memset(new_passwd, 0, sizeof(new_passwd));
	memset(passwd_check, 0, sizeof(passwd_check));

    init_screen(false);

    printf("*** Change password ***\n");
    printf("Customer username......: %s\n", username);
    printf("Insert old password....: ");
    get_input(BUFFSIZE_L, old_passwd, true, true);

retype_pass:
	printf("Insert new password....: ");
	get_input(BUFFSIZE_L, new_passwd, true, true);

	printf("Retype new password....: ");
	get_input(BUFFSIZE_L, passwd_check, true, true);

	if (strcmp(new_passwd, passwd_check) != 0)
	{
		printf("Mismatch password, please retry!\n");
		goto retype_pass;
	}
    
    if (attempt_change_password(username, old_passwd, new_passwd))
		printf("Password has been changed!\n");
	else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}
