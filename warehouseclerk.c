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

typedef struct spec_info
{
    unsigned int species_code;
    char species_name[132]; // |NOME COMUNE| + |NOME LATINO| + 3 + 1
    unsigned int stock;
} spec_info_t;

static char curr_user[BUFFSIZE_L];


static bool attempt_search_suppliers(char *name)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];
    char prompt[BUFFSIZE_L];

	memset(param, 0, sizeof(param));
   	memset(prompt, 0, sizeof(prompt));

	if(!setup_prepared_stmt(&stmt, "call visualizza_fornitori(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome VARCHAR(32)
	param[0].buffer = name;
	param[0].buffer_length = strlen(name);

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

    if (strlen(name) > 0)
        snprintf(prompt, BUFFSIZE_L, "\nSearch results for \'%s\':", name);
    else
        snprintf(prompt, BUFFSIZE_L, "\nSearch results:");

    if (!dump_result_set(stmt, prompt, LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;
}

static bool attempt_select_available_species(unsigned int sup_code)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call visualizza_specie_disponibili(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_fornitore INT
	param[0].buffer = &sup_code;
	param[0].buffer_length = sizeof(sup_code);

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

    if (!dump_result_set(stmt, "\nSearch results:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;
}

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

static bool attempt_add_supply_availability(unsigned int supplier_code, unsigned int species_code)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[2];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call aggiungi_disponibilita_fornitura(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_fornitore INT
	param[0].buffer = &supplier_code;
	param[0].buffer_length = sizeof(supplier_code);

    param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(species_code);

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

    if (!dump_result_set(stmt, "\nUpdated list:", 0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;  
}

static void add_supply_availability(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char spec_name[BUFFSIZE_M];
    char sup_name[BUFFSIZE_S];
    unsigned int sup_code;
    unsigned int spec_code;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(spec_name, 0, sizeof(spec_name));
    memset(sup_name, 0, sizeof(sup_name));

    init_screen(false);

    printf("*** Add supply availability ***\n");
  
    if (ask_for_tips("Do you wanna see a list of available suppliers", 12))
    {
        printf("\nInsert the name to filter on (default all).......................: "); 
        get_input(BUFFSIZE_S, sup_name, false, false);
        if (!attempt_search_suppliers(sup_name))
            printf("Operation failed\n");

        putchar('\n');
    }

    printf("Insert supplier code.............................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    sup_code = strtol(buffer_for_integer, NULL, 10);

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
    spec_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (attempt_add_supply_availability(sup_code, spec_code))
        printf("Availability succesfully added\n");
    else
        printf("Operation failed\n");

    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_add_supply_request(unsigned int supplier_code, unsigned int species_code, unsigned int quantity)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[3];
    int flags = LEADING_ZERO_BITMASK_IDX_0 | LEADING_ZERO_BITMASK_IDX_1;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call inserisci_richiesta_fornitura(?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_fornitore INT
	param[0].buffer = &supplier_code;
	param[0].buffer_length = sizeof(supplier_code);

    param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(species_code);

    param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
	param[2].buffer = &quantity;
	param[2].buffer_length = sizeof(quantity);

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

    if (!dump_result_set(stmt, "\nPendant request for selected species:", flags)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;  
}

static void add_supply_request(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char sup_name[BUFFSIZE_S];
    unsigned int sup_code;
    unsigned int spec_code;
    unsigned int quantity;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(sup_name, 0, sizeof(sup_name));

    init_screen(false);

    printf("*** Add supply request ***\n");
  
    if (ask_for_tips("Do you wanna see a list of available suppliers", 10))
    {
        printf("\nInsert the name to filter on (default all).....................: "); 
        get_input(BUFFSIZE_S, sup_name, false, false);
        if (!attempt_search_suppliers(sup_name))
            printf("Operation failed\n");

        putchar('\n');
    }

    printf("Insert supplier code...........................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    sup_code = strtol(buffer_for_integer, NULL, 10);
                      
    if (ask_for_tips("Do you wanna see avaliable species for selected supplier", 0))
    {
        if (!attempt_select_available_species(sup_code))
            printf("Operation failed\n");

        putchar('\n');        
    }

    printf("Insert species code............................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    spec_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity.......................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    quantity = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (!attempt_add_supply_request(sup_code, spec_code, quantity))
        printf("Operation failed\n");
    
    putchar('\n');

    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_show_stock_status(unsigned int species_code)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call visualizza_dettagli_giacenza(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[0].buffer = &species_code;
	param[0].buffer_length = sizeof(species_code);

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

    if (!dump_result_set(stmt, "\nResults for selected species:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;   
}

static void show_stock_status(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char spec_name[BUFFSIZE_M];
    unsigned int spec_code;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(spec_name, 0, sizeof(spec_name));

    init_screen(false);

    printf("*** Show stock status for a species ***\n");  

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
    spec_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (!attempt_show_stock_status(spec_code))
        printf("Operation failed\n");
    
    putchar('\n');

    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_report_stock(unsigned int range)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];
    int counter = 0;
    char prompt[BUFFSIZE_XL];
    spec_info_t spec_info;
    int status;

	memset(param, 0, sizeof(param));
    memset(&spec_info, 0, sizeof(spec_info));
    memset(prompt, 0, sizeof(prompt));

	if(!setup_prepared_stmt(&stmt, "call report_giacenza(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_range INT
	param[0].buffer = &range;
	param[0].buffer_length = sizeof(range);

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

	do {
		if(conn->server_status & SERVER_PS_OUT_PARAMS) 
			goto next;

        if (counter == 0)
            snprintf(prompt, BUFFSIZE_XL, "\n*** Species details ***");
        else if (counter % 2 == 0 && counter != 0)
            snprintf(prompt, BUFFSIZE_XL, "\n\n\n*** Species details ***");
        else
            snprintf(prompt, BUFFSIZE_XL, "\nList of the 5 most frequently chosen suppliers:");

        if (!dump_result_set(stmt, prompt, LEADING_ZERO_BITMASK_IDX_0))
        {
            CLOSEANDRET(false);
        }

        ++counter;

	    next:
		status = mysql_stmt_next_result(stmt);
		if (status > 0)
        {
            printf("here\n");
            print_stmt_error(stmt, "Unexpected condition");
            CLOSEANDRET(false);        
        }
		
	} while (status == 0);

	mysql_stmt_close(stmt);
	return true;   
}

static void report_stock(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int range;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** View details of the species to be supplied ***\n");  

    printf("How many species do you wanna see into the report: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    range = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (!attempt_report_stock(range))
        printf("Operation failed\n");
    

    printf("\n\nPress enter key to get back to menu ...\n");
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
        printf("2) Add supply availability\n");
        printf("3) Add supply request\n");
        printf("4) View stock status for a species\n");
        printf("5) View details of the species to be supplied\n");
        printf("p) Change password\n");
        printf("q) Quit\n");

        choice = multi_choice("Pick an option", "12345pq", 7);

        switch (choice)
        {
            case '1': insert_new_supplier(); break;
            case '2': add_supply_availability(); break;
            case '3': add_supply_request(); break;
            case '4': show_stock_status(); break;
            case '5': report_stock(); break;
            case 'p': change_password(curr_user); break;
            case 'q': printf("Bye bye!\n\n\n"); return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}