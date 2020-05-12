#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include "defines.h"

typedef struct insert_supplier_sp_params 
{
    char fiscal_code[BUFFSIZE_XS];
    char name[BUFFSIZE_S];
    unsigned int species_code;
    char address[BUFFSIZE_M];
} insert_supplier_sp_params_t;


static char curr_user[BUFFSIZE_L];


static unsigned int attempt_insert_new_supplier(insert_supplier_sp_params_t *input)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[5];
	unsigned int code;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call inserisci_fornitore(?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 0;
	}
	
	param[0].buffer_type = MYSQL_TYPE_STRING; // IN var_codice_fiscale CHAR(16)
	param[0].buffer = input->fiscal_code;
	param[0].buffer_length = strlen(input->fiscal_code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome VARCHAR(32)
    param[1].buffer = input->name;
    param[1].buffer_length = strlen(input->name);

    param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[2].buffer = &(input->species_code);
	param[2].buffer_length = sizeof(input->species_code);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_indirizzo VARCHAR(64)
    param[3].buffer = input->address;
    param[3].buffer_length = strlen(input->address);

	param[4].buffer_type = MYSQL_TYPE_LONG; // OUT var_codice_fornitore INT
	param[4].buffer = &code;
	param[4].buffer_length = sizeof(code);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(0);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
        CLOSEANDRET(0);
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_codice INT
	param[0].buffer = &code;
	param[0].buffer_length = sizeof(code);
	
	if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Could not retrieve output parameter");
		CLOSEANDRET(0);
	}
	
	if(mysql_stmt_fetch(stmt)) 
	{
		print_stmt_error(stmt, "Could not buffer results");
		CLOSEANDRET(0);
	}

	mysql_stmt_close(stmt);
	return code;
}

static void insert_new_supplier(void)
{
    insert_supplier_sp_params_t params;
    unsigned int supplier_code;
    char spec_name[BUFFSIZE_M];
    char buffer_for_integer[BUFFSIZE_XS];

    memset(&params, 0, sizeof(params));
    memset(spec_name, 0, sizeof(spec_name));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false); 

    printf("*** Insert a new supplier ***\n");

    printf("Insert fiscal code...............................................: ");
    get_input(BUFFSIZE_S, params.fiscal_code, false, true);

    printf("Insert first name................................................: ");
    get_input(BUFFSIZE_S, params.name, false, true);

    if (ask_for_tips("Do you wanna search species by name to find the right code", 0))
    {
        printf("\nInsert the name to filter on (default all).......................: ");   
        get_input(BUFFSIZE_M, spec_name, false, false);
        if (!attempt_search_species(false, spec_name))
            printf("Operation failed\n");

        putchar('\n');        
    }

    printf("Insert species code..............................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    params.species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert address...................................................: ");
    get_input(BUFFSIZE_M, params.address, false, true);

    putchar('\n');

    supplier_code = attempt_insert_new_supplier(&params);
    if (supplier_code > 0)
        printf("New supplier inserted (SCODE: %010u)\n", supplier_code);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

void run_as_warehouse_clerk(char *username)
{
    config_t cnf;
    char choice;

    memset(&cnf, 0, sizeof(cnf));
    memset(curr_user, 0, sizeof(curr_user));

    strncpy(curr_user, username, BUFFSIZE_L);

    if (parse_config("config/adm.user", &cnf, "="))
    {
        fprintf(stderr, "Invalid configuration file selected (ADM)\n");
        exit(EXIT_FAILURE);
    }

	if(mysql_change_user(conn, cnf.username, cnf.password, cnf.database)) {
		fprintf(stderr, "Unable to switch privileges\n");
		exit(EXIT_FAILURE);
	}

    while (true)
    {
        init_screen(true);
        printf("Welcome %s\n\n", curr_user);
        printf("*** What do you wanna do? ***\n\n");
        printf("1) Insert a new supplier\n");
        printf("2) Change password\n");
        printf("q) Quit\n");

        choice = multi_choice("Pick an option", "12q", 2);

        switch (choice)
        {
            case '1': insert_new_supplier(); break;
            case '2': change_password(curr_user); break;
            case 'q': printf("Bye bye!\n\n\n"); return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}