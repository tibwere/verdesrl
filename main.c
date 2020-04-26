#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include "defines.h"

#define LOGIN_SP_NO_PARAMS 4
#define SIGNUP_SP_NO_PARAMS 8

typedef enum { CLP, CLR, ADM, OPP, MNG, CPP, ERR } role_t;

typedef struct credentials
{
    char username[REALSIZE(CRED_LENGTH)];
    char password[REALSIZE(CRED_LENGTH)];
} credentials_t;

typedef struct customer_signup 
{
	credentials_t credentials;
	char code[REALSIZE(CUSTOMER_CODE_MAX_LENGTH)];
	char name[REALSIZE(NAME_LENGTH)];
	char residential_address[REALSIZE(ADDRESS_LENGTH)];
	char billing_address[REALSIZE(ADDRESS_LENGTH)];
	char referent_first_name[REALSIZE(NAME_LENGTH)];
	char referent_last_name[REALSIZE(NAME_LENGTH)];
} customer_signup_t;


static role_t attempt_login(credentials_t *cred, char *identifier); 
static int attempt_signup(customer_signup_t *cst, char modality);
static void login_manager(void);
static void signup_manager(void);

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
        print_error(conn, "Something went wrong, connection was not established.");

    init_screen(false);

    choice = multi_choice("Do you wanna [l]ogin or [s]ignup?", "ls", 2);

    if (choice == 'l') 
		login_manager();
	else if (choice == 's')
		signup_manager(); 
	else
	{
		fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
		abort();
	}    

    
    mysql_close(conn);
    exit(EXIT_SUCCESS);
}

static role_t attempt_login(credentials_t *cred, char *identifier) 
{
	MYSQL_STMT *login_procedure;
	
	MYSQL_BIND param[LOGIN_SP_NO_PARAMS];
	int role;
    

	if(!setup_prepared_stmt(&login_procedure, "call login(?, ?, ?, ?)", conn)) 
	{
		print_stmt_error(login_procedure, "Unable to initialize login statement\n");
		goto err_log_2;
	}

	memset(param, 0, sizeof(param));
	
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
	param[3].buffer_length = CUSTOMER_CODE_MAX_LENGTH * sizeof(char);

	if (mysql_stmt_bind_param(login_procedure, param) != 0) 
	{ 
		print_stmt_error(login_procedure, "Could not bind parameters for login");
		goto err_log;
	}

	if (mysql_stmt_execute(login_procedure) != 0) 
	{
		print_stmt_error(login_procedure, "Could not execute login procedure");
		goto err_log;
	}


	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_ruolo INT
	param[0].buffer = &role;
	param[0].buffer_length = sizeof(role);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // OUT var_codice_cliente VARCHAR(16)
	param[1].buffer = identifier;
	param[1].buffer_length = CUSTOMER_CODE_MAX_LENGTH * sizeof(char);
	
	if(mysql_stmt_bind_result(login_procedure, param)) 
	{
		print_stmt_error(login_procedure, "Could not retrieve output parameter");
		goto err_log;
	}
	
	if(mysql_stmt_fetch(login_procedure)) 
	{
		print_stmt_error(login_procedure, "Could not buffer results");
		goto err_log;
	}

	mysql_stmt_close(login_procedure);
	return role;

    err_log:
	mysql_stmt_close(login_procedure);
    err_log_2:
	return ERR;
}

static int attempt_signup(customer_signup_t *cst, char modality) 
{
	MYSQL_STMT *signup_procedure;
	MYSQL_BIND param[SIGNUP_SP_NO_PARAMS];

	if (modality == 'p')
	{
		if(!setup_prepared_stmt(&signup_procedure, "call registra_privato(?, ?, ?, ?, ?, ?)", conn)) 
		{
			print_stmt_error(signup_procedure, "Unable to initialize signup statement\n");
			goto err_sig_2;
		}
	}
	else
	{
		if(!setup_prepared_stmt(&signup_procedure, "call registra_rivendita(?, ?, ?, ?, ?, ?, ?, ?)", conn)) 
		{
			print_stmt_error(signup_procedure, "Unable to initialize signup statement\n");
			goto err_sig_2;
		}
	}

	memset(param, 0, sizeof(param));
	
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

	if (modality == 'r')
	{
		param[6].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_ref VARCHAR(32)
		param[6].buffer = cst->referent_first_name;
		param[6].buffer_length = strlen(cst->referent_first_name);

		param[7].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cognome_ref VARCHAR(32)
		param[7].buffer = cst->referent_last_name;
		param[7].buffer_length = strlen(cst->referent_last_name);
	}

	if (mysql_stmt_bind_param(signup_procedure, param) != 0) 
	{ 
		print_stmt_error(signup_procedure, "Could not bind parameters for signup");
		goto err_sig;
	}

	if (mysql_stmt_execute(signup_procedure) != 0) 
	{
		if (mysql_stmt_errno(signup_procedure) == 1062)
			fprintf(stderr, "User already exists!\n");
		else
			print_stmt_error(signup_procedure, "Could not execute signup procedure");
		goto err_sig;
	}

	mysql_stmt_close(signup_procedure);
	return 0;

    err_sig:
	mysql_stmt_close(signup_procedure);
    err_sig_2:
	return 1;
}

static void login_manager(void)
{
	char client_identifier[REALSIZE(CUSTOMER_CODE_MAX_LENGTH)];
	credentials_t cred;
	role_t role;

	memset(&client_identifier, 0, CUSTOMER_CODE_MAX_LENGTH * sizeof(char));
    memset(&cred, 0, sizeof(credentials_t));

	printf("Insert username: ");
	get_input(CRED_LENGTH, cred.username, false, true);
	printf("Insert password: ");
	get_input(CRED_LENGTH, cred.password, true, true);

	role = attempt_login(&cred, client_identifier);
	
	switch (role)
	{
		case CLP: run_as_customer(cred.username, client_identifier, true); break;
		case CLR: run_as_customer(cred.username, client_identifier, false); break;
		case ADM:
		case OPP:
		case MNG:
		case CPP: printf("Sorry not implemented yet!\n"); break;
		case ERR: printf("Login failed!\n"); break;
		default: 
			fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
			abort();
	}
}

static void signup_manager(void)
{
	char password_check[REALSIZE(CRED_LENGTH)];
	customer_signup_t cst;
	char modality;
	int ret;
	
	memset(&cst, 0, sizeof(customer_signup_t));

	modality = multi_choice("Are you a [p]rivate or [r]etailer?", "pr", 2);

	if (modality != 'p' && modality != 'r')
	{
		fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
		abort();		
	}

	printf("Insert username: ");
	get_input(CRED_LENGTH, (cst.credentials).username, false, true);

retype_pass:
	printf("Insert password: ");
	get_input(CRED_LENGTH, (cst.credentials).password, true, true);

	printf("Confirm password: ");
	get_input(CRED_LENGTH, password_check, true, true);

	if (strcmp((cst.credentials).password, password_check) != 0)
	{
		printf("Mismatch password, please retry!\n");
		goto retype_pass;
	}

	if (modality == 'p')
	{
		printf("Insert fiscal code: ");
		get_input(16, cst.code, false, true);
	}
	else
	{
		printf("Insert VAT code: ");
		get_input(11, cst.code, false, true);
	}

	printf("Insert your name: ");
	get_input(NAME_LENGTH, cst.name, false, true);

	printf("Insert your residential address: ");
	get_input(ADDRESS_LENGTH, cst.residential_address, false, true);

	printf("Insert your billing address (default null): ");
	get_input(ADDRESS_LENGTH, cst.billing_address, false, false);

	if (modality == 'r')
	{
		printf("Insert referent first name: ");
		get_input(NAME_LENGTH, cst.referent_first_name, false, true);

		printf("Insert referent last name: ");
		get_input(NAME_LENGTH, cst.referent_last_name, false, true);		
	}

	ret = attempt_signup(&cst, modality);
	(ret == 1) ? printf("Signup failed!\n") : printf("Succesfully signed up\n");

}

