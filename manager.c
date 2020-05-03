#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include <regex.h>
#include <time.h>
#include "defines.h"

typedef struct insert_species_sp_no_params 
{
    char common_name[BUFFSIZE_M];
    char latin_name[BUFFSIZE_M];
    signed char in_or_out;
    char coloring[BUFFSIZE_S];
    signed char exotic;
    unsigned int stock;
    char price[BUFFSIZE_XS];
 } insert_species_sp_no_params_t;


static char curr_user[BUFFSIZE_L];


static int check_price(char *inserted_price, char *strerror, size_t strerror_length)
{
    regex_t reg;
    char err_mess[BUFFSIZE_L];
    int ret;

    ret = regcomp(&reg, "^([0-9]{0,5}((.)[0-9]{0,2}))$", REG_EXTENDED);
    if (ret) 
    {
        regerror(ret, &reg, err_mess, sizeof(err_mess));
        snprintf(strerror, strerror_length, "Unable to compile regexp to check inserted price\nThe error was: %s\n", err_mess);
        return -1;
    }

    ret = regexec(&reg, inserted_price, 0, NULL, 0);
    if (ret == 0) 
    {
        regfree(&reg);
        return 1;
    }
    else if (ret == REG_NOMATCH) 
    {
        regfree(&reg);
        return 0;
    }
    else 
    {
        regerror(ret, &reg, err_mess, sizeof(err_mess));
        snprintf(strerror, strerror_length, "Unable to exec regexp to check inserted price\nThe error was: %s\n", err_mess);
        return -1;
    }
}

static unsigned int attempt_insert_species(insert_species_sp_no_params_t *input)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[8];
	unsigned int code;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call inserisci_nuova_specie(?, ?, ?, ?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 0;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_comune VARCHAR(64)
	param[0].buffer = input->common_name;
	param[0].buffer_length = strlen(input->common_name);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_latino VARCHAR(64)
    param[1].buffer = input->latin_name;
    param[1].buffer_length = strlen(input->latin_name);

    param[2].buffer_type = MYSQL_TYPE_TINY; // IN var_int_est TINYINT
    param[2].buffer = &(input->in_or_out);
    param[2].buffer_length = sizeof(input->in_or_out);

    if (strlen(input->coloring) > 0)
    {
        param[3].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_colorazione VARCHAR(32)
        param[3].buffer = input->coloring;
        param[3].buffer_length = strlen(input->coloring);   
    }
    else
    {
        param[3].buffer_type = MYSQL_TYPE_NULL; // IN var_colorazione VARCHAR(32)
        param[3].buffer = NULL;
        param[3].buffer_length = 0; 
    }

    param[4].buffer_type = MYSQL_TYPE_TINY; // IN var_esotica TINYINT
    param[4].buffer = &(input->exotic);
    param[4].buffer_length = sizeof(input->exotic);

    param[5].buffer_type= MYSQL_TYPE_LONG; // IN var_giacenza INT
    param[5].buffer = &(input->stock);
    param[5].buffer_length = sizeof(input->stock);

    param[6].buffer_type = MYSQL_TYPE_NEWDECIMAL; // IN var_prezzo DECIMAL(7, 2)
    param[6].buffer = input->price;
    param[6].buffer_length = strlen(input->price);

    param[7].buffer_type = MYSQL_TYPE_LONG; // OUT var_codice INT
	param[7].buffer = &code;
	param[7].buffer_length = sizeof(code);

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

static void insert_a_species(void)
{
    insert_species_sp_no_params_t params;
    char buffer_for_integer[BUFFSIZE_XS];
    char strerror[BUFFSIZE_XL];
    unsigned int species_code;
    char choice;
    int ret;

    memset(&params, 0, sizeof(params));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(strerror, 0, sizeof(strerror));

    init_screen(false);

    printf("*** Insert a new species ***\n");

    printf("Insert common name................................: ");
    get_input(BUFFSIZE_M, params.common_name, false, true);

    printf("Insert latin name.................................: ");
    get_input(BUFFSIZE_M, params.latin_name, false, true);

    choice = multi_choice("Which kind of species is it? [o]utdoor or [i]ndoor: ", "oi", 2);
    params.in_or_out = (choice == 'i');

    choice = multi_choice("Which kind of species is it? [g]reen or [f]lowery.: ", "gf", 2);
    if (choice == 'f')
    {
        printf("Insert default coloring...........................: ");
        get_input(BUFFSIZE_S, params.coloring, false, true);
    }

    choice = multi_choice("Is it exotic? [y]es or [n]o:......................: ", "yn", 2);
    params.exotic = (choice == 'y') ? 1 : 0;

    printf("Insert initial stock..............................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    params.stock = strtol(buffer_for_integer, NULL, 10);

insert_price:
    printf("Insert price (#####.##)...........................: ");
    get_input(BUFFSIZE_XS, params.price, false, true);
    ret = check_price(params.price, strerror, BUFFSIZE_XL);

    if (ret == 0)
    {
        printf("Not compliant input please retry\n");
        goto insert_price;
    }
    if (ret == -1)
    {
        fprintf(stderr, strerror);
        goto exit;
    }

    putchar('\n');

    species_code = attempt_insert_species(&params);
    if (species_code > 0)
        printf("New species inserted (CODE: %010u)\n", species_code);
    else
        printf("Operation failed\n");
        
exit:
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_remove_species(unsigned int species_code)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];
    int affected_rows;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call rimuovi_specie_di_pianta(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return -1;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_codice INT
	param[0].buffer = &species_code;
	param[0].buffer_length = sizeof(species_code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_effettiva INT
	param[1].buffer = &affected_rows;
	param[1].buffer_length = sizeof(affected_rows);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(-1); 	
    }

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
    	CLOSEANDRET(-1); 
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_effettiva INT
	param[0].buffer = &affected_rows;
	param[0].buffer_length = sizeof(affected_rows);
	
	if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Could not retrieve output parameter");
		CLOSEANDRET(-1);
	}
	
	if(mysql_stmt_fetch(stmt)) 
	{
		print_stmt_error(stmt, "Could not buffer results");
		CLOSEANDRET(-1);
	}

	mysql_stmt_close(stmt);
	return affected_rows;  
}

static void remove_a_species(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int species_code;
    int ret;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** Finalize an order ***\n");
    printf("Insert species code: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    ret = attempt_remove_species(species_code);

    if (ret > 0)
        printf("Species %u succesfully deleted\n", species_code);
    else if (ret == 0)
        printf("Nothing has changed (species %u not found)\n", species_code);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_add_coloring(unsigned int species_code, char *coloring)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call aggiungi_colorazione(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_specie_fiorita INT
	param[0].buffer = &species_code;
	param[0].buffer_length = sizeof(species_code);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_colore VARCHAR(32)
	param[1].buffer = coloring;
	param[1].buffer_length = strlen(coloring);

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

    if (!dump_result_set(stmt, "\nUpdated coloring list:", 0)) 
    {
        CLOSEANDRET(false);
    }

	mysql_stmt_close(stmt);
	return true;  
}

static void add_coloring(void)
{
    unsigned int species_code;
    char coloring[BUFFSIZE_S];
    char buffer_for_integer[BUFFSIZE_XS];

    memset(coloring, 0, sizeof(coloring));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** Add a coloring for a flowering species ***\n");
    printf("Insert species code.............: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert coloring for this species: ");
    get_input(BUFFSIZE_S, coloring, false, true);

    putchar('\n');

    if (attempt_add_coloring(species_code, coloring))
        printf("Coloring \"%s\" for %010u succesfully added\n", coloring, species_code);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();   
}

static int attempt_remove_coloring(unsigned int species_code, char *coloring)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[3];
    int affected_rows;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call rimuovi_colorazione(?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return -1;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_specie_fiorita INT
	param[0].buffer = &species_code;
	param[0].buffer_length = sizeof(species_code);

	param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_colore VARCHAR(32)
	param[1].buffer = coloring;
	param[1].buffer_length = strlen(coloring);

	param[2].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_effettiva INT
	param[2].buffer = &affected_rows;
	param[2].buffer_length = sizeof(affected_rows);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(-1); 	
    }

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
    	CLOSEANDRET(-1); 
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_effettiva INT
	param[0].buffer = &affected_rows;
	param[0].buffer_length = sizeof(affected_rows);
	
	if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Could not retrieve output parameter");
		CLOSEANDRET(-1);
	}
	
	if(mysql_stmt_fetch(stmt)) 
	{
		print_stmt_error(stmt, "Could not buffer results");
		CLOSEANDRET(-1);
	}

	mysql_stmt_close(stmt);
	return affected_rows;   
}

static void remove_coloring(void)
{
    unsigned int species_code;
    char coloring[BUFFSIZE_S];
    char buffer_for_integer[BUFFSIZE_XS];
    int ret;

    memset(coloring, 0, sizeof(coloring));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** Remove a coloring from a flowering species ***\n");
    printf("Insert species code..........: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert coloring to be removed: ");
    get_input(BUFFSIZE_S, coloring, false, true);

    putchar('\n');

    ret = attempt_remove_coloring(species_code, coloring);

    if (ret > 0)
        printf("Coloring \"%s\" succesfully removed for %010u\n", coloring, species_code);
    else if (ret == 0)
        printf("Nothing has changed (species %010u does not have this coloring)\n", species_code);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar(); 
}

static int attempt_change_price(unsigned int species_code, char *price)
{
    MYSQL_STMT *stmt;	
	MYSQL_BIND param[3];
    int affected_rows;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call modifica_prezzo(?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return -1;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[0].buffer = &species_code;
	param[0].buffer_length = sizeof(species_code);

    param[1].buffer_type = MYSQL_TYPE_NEWDECIMAL; // IN var_prezzo DECIMAL(7, 2)
    param[1].buffer = price;
    param[1].buffer_length = strlen(price);

	param[2].buffer_type = MYSQL_TYPE_LONG; // OUT var_aggiornamento_effettivo INT
	param[2].buffer = &affected_rows;
	param[2].buffer_length = sizeof(affected_rows);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(-1); 	
    }

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
    	CLOSEANDRET(-1); 
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_aggiornamento_effettivo INT
	param[0].buffer = &affected_rows;
	param[0].buffer_length = sizeof(affected_rows);
	
	if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Could not retrieve output parameter");
		CLOSEANDRET(-1);
	}
	
	if(mysql_stmt_fetch(stmt)) 
	{
		print_stmt_error(stmt, "Could not buffer results");
		CLOSEANDRET(-1);
	}

	mysql_stmt_close(stmt);
	return affected_rows;   
}

static void change_price(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char price[BUFFSIZE_XS];
    char strerror[BUFFSIZE_XL];
    unsigned int species_code;
    int ret;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(strerror, 0, sizeof(strerror));
    memset(price, 0, sizeof(price));


    init_screen(false);

    printf("*** Change the price of a species ***\n");

    printf("Insert species code....: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

insert_price:
    printf("Insert price (#####.##): ");
    get_input(BUFFSIZE_XS, price, false, true);
    ret = check_price(price, strerror, BUFFSIZE_XL);

    if (ret == 0)
    {
        printf("Not compliant input please retry\n");
        goto insert_price;
    }
    if (ret == -1)
    {
        fprintf(stderr, strerror);
        goto exit;
    }    

    putchar('\n');

    ret = attempt_change_price(species_code, price);

    if (ret > 0)
        printf("Price updated for %010u\n", species_code);    
    else if (ret == 0)
        printf("Nothing has changed (the stored and entered prices coincide)\n");
    else
        printf("Operation failed\n");
        
exit:
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool get_species_name(MYSQL_STMT *stmt, unsigned int species_code, char *species_info)
{
    int status;
    MYSQL_BIND param[1];
    char species_name[132];

	memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[0].buffer = species_name;
	param[0].buffer_length = sizeof(species_name);

    if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Unable to bind output parameters");
		return false;
	}
    
    status = mysql_stmt_fetch(stmt);
    
    if (status == 1 || status == MYSQL_NO_DATA)
        return false;
    
    snprintf(species_info, BUFFSIZE_XL, "\"%s\" [CODE: %010u]", species_name, species_code);

    return true;
}

static bool attempt_report_species(unsigned int species_code)
{
   	MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];
    char species_info[BUFFSIZE_XL];
    char prompt[2 * BUFFSIZE_XL];
    
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    memset(species_info, 0, sizeof(species_info));
    memset(prompt, 0, sizeof(prompt));
	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call report_specie(?)", conn)) 
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

    if (get_species_name(stmt, species_code, species_info))
    {
	    if (mysql_stmt_next_result(stmt))
        {
            print_stmt_error(stmt, "Unexpected condition");
            CLOSEANDRET(false); 
        }

        snprintf(prompt, 2 * BUFFSIZE_XL, "\n\nSales details for %s (year: %d):", species_info, (tm->tm_year + 1900));

        if (!dump_result_set(stmt, prompt, 0)) 
        {
            CLOSEANDRET(false);
        } 
    }   

	mysql_stmt_close(stmt);
	return true;   
}

static void report_species(void)
{
    unsigned int species_code;
    char buffer_for_integer[BUFFSIZE_XS];

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** View sales trend for a chosen species ***\n");
    printf("Insert species code..........: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (!attempt_report_species(species_code))
        printf("Operation failed\n");
        
    printf("\nPress enter key to get back to menu ...\n");
    getchar();
}

void run_as_manager(char *username)
{
    config_t cnf;
    char choice;

    memset(&cnf, 0, sizeof(cnf));
    memset(curr_user, 0, sizeof(curr_user));

    strncpy(curr_user, username, BUFFSIZE_L);

    if (parse_config("config/mng.user", &cnf, "="))
    {
        fprintf(stderr, "Invalid configuration file selected (MNG)\n");
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
        printf("1) Insert a new species\n");
        printf("2) Remove a species\n");
        printf("3) Add a coloring for a flowering species\n");
        printf("4) Remove a coloring from a flowering species list\n");
        printf("5) Change the price of a species\n");
        printf("6) View sales trend for a chosen species\n");
        printf("7) Change password\n");
        printf("q) Quit\n");

        choice = multi_choice("Pick an option", "1234567q", 8);

        switch (choice)
        {
            case '1': insert_a_species(); break;
            case '2': remove_a_species(); break;
            case '3': add_coloring(); break;
            case '4': remove_coloring(); break;
            case '5': change_price(); break;
            case '6': report_species(); break;
            case '7': change_password(curr_user); break;
            case 'q': printf("Bye bye!\n\n\n"); return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}