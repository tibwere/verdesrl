#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "defines.h"

#define PATH_LENGTH 16
#define CREATE_ORDER_SP_NO_PARAMS 6
#define FINALIZE_ORDER_SP_NO_PARAMS 2
#define ORDER_SP_NO_PARAMS 4
#define SEARCH_SP_NO_PARAMS 1
#define FINAL_MSG_LENGTH 32
#define PROC_STR_LENGTH 64
#define UPDATE_ADDR_SP_NO_PARAMS 2
#define ADD_CONTACT_SP_NO_PARAMS 3

typedef struct customer_info
{
    char username[REALSIZE(CRED_LENGTH)];
    char code[REALSIZE(CUSTOMER_CODE_MAX_LENGTH)];
    bool is_private;
} customer_info_t;

typedef struct order_sp_params
{
    char shipping_address[REALSIZE(ADDRESS_LENGTH)];
    char contact[REALSIZE(CONTACT_LENGTH)];
    unsigned int species;
    unsigned int quantity;
} order_sp_params_t;


static customer_info_t curr_customer;


static unsigned int attempt_open_order(order_sp_params_t *input)
{
	MYSQL_STMT *create_order_procedure;
	
	MYSQL_BIND param[CREATE_ORDER_SP_NO_PARAMS];
	unsigned int id;

	if(!setup_prepared_stmt(&create_order_procedure, "call crea_ordine(?, ?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(create_order_procedure, "Unable to initialize create order statement\n");
		goto err_open_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	if (strlen(input->shipping_address) > 0)
	{
        param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_ind_sped VARCHAR(64)
        param[1].buffer = input->shipping_address;
        param[1].buffer_length = strlen(input->shipping_address);
	}
	else
	{
        param[1].buffer_type = MYSQL_TYPE_NULL; // IN var_ind_sped VARCHAR(64)
        param[1].buffer = NULL;
        param[1].buffer_length = 0;	
	}

	if (strlen(input->contact) > 0)
	{
        param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(64)
        param[2].buffer = input->contact;
        param[2].buffer_length = strlen(input->contact);
	}
	else
	{
        param[2].buffer_type = MYSQL_TYPE_NULL; // IN var_contatto VARCHAR(64)
        param[2].buffer = NULL;
        param[2].buffer_length = 0;	
	}

	param[3].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[3].buffer = &(input->species);
	param[3].buffer_length = sizeof(unsigned int);

	param[4].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
	param[4].buffer = &(input->quantity);
	param[4].buffer_length = sizeof(unsigned int);

    param[5].buffer_type = MYSQL_TYPE_VAR_STRING; // OUT var_id INT
	param[5].buffer = &id;
	param[5].buffer_length = sizeof(unsigned int);

	if (mysql_stmt_bind_param(create_order_procedure, param) != 0) 
	{ 
		print_stmt_error(create_order_procedure, "Could not bind parameters for create order");
		goto err_open;
	}

	if (mysql_stmt_execute(create_order_procedure) != 0) 
	{
		print_stmt_error(create_order_procedure, "Could not execute create order procedure");
		goto err_open;
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_id INT
	param[0].buffer = &id;
	param[0].buffer_length = sizeof(unsigned int);
	
	if(mysql_stmt_bind_result(create_order_procedure, param)) 
	{
		print_stmt_error(create_order_procedure, "Could not retrieve output parameter");
		goto err_open;
	}
	
	if(mysql_stmt_fetch(create_order_procedure)) 
	{
		print_stmt_error(create_order_procedure, "Could not buffer results");
		goto err_open;
	}

	mysql_stmt_close(create_order_procedure);
	return id;

    err_open:
	mysql_stmt_close(create_order_procedure);
    err_open_2:
	return 0;    
}

static void open_order(void)
{
    order_sp_params_t params;
    char buffer_for_integer[INT_STR_LENGTH];
    unsigned int order_id;

    memset(&params, 0, sizeof(order_sp_params_t));
    memset(&buffer_for_integer, 0, INT_STR_LENGTH);

    init_screen(false);

    printf("*** Open a new order ***\n");
    printf("Customer code........................................: %s\n", curr_customer.code);
    
    printf("Insert shipping address (default residential address): ");
    get_input(ADDRESS_LENGTH, params.shipping_address, false, false);

    printf("Insert contact (default favourite one)...............: ");
    get_input(CONTACT_LENGTH, params.contact, false, false);

    printf("Insert species code..................................: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    params.species = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity.............................: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    params.quantity = strtol(buffer_for_integer, NULL, 10);

    order_id = attempt_open_order(&params);
    if (order_id > 0)
        printf("New order opened (ID: %010u)\n", order_id);
    else
        printf("No order has opened\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_to_exec_op_on_order(bool is_add, unsigned int order_id, unsigned int species_code, unsigned int quantity)
{
    char sp_str[PROC_STR_LENGTH];
	MYSQL_STMT *order_procedure;
	MYSQL_BIND param[ORDER_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call %s(?, ?, ?, ?)", 
        (is_add) ? "aggiungi_specie_ad_ordine_esistente" : "modifica_ordine");

	if(!setup_prepared_stmt(&order_procedure, sp_str, conn)) 
    {
		print_stmt_error(order_procedure, "Unable to initialize selected operation on order statement\n");
		goto err_op_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(unsigned int);

	param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[2].buffer = &order_id;
	param[2].buffer_length = sizeof(unsigned int);

    param[3].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
    param[3].buffer = &quantity;
    param[3].buffer_length = sizeof(unsigned int);


	if (mysql_stmt_bind_param(order_procedure, param) != 0) 
	{ 
		print_stmt_error(order_procedure, "Could not bind parameters for selected operation on order");
		goto err_op;
	}

	if (mysql_stmt_execute(order_procedure) != 0) 
	{
		print_stmt_error(order_procedure, "Could not execute selected operation on order procedure");
		goto err_op;
    }

	mysql_stmt_close(order_procedure);
	return 0;

    err_op:
	mysql_stmt_close(order_procedure);
    err_op_2:
	return 1;    
}

static void exec_op_on_order(bool is_add)
{
    char buffer_for_integer[INT_STR_LENGTH];
    char final_message[FINAL_MSG_LENGTH];
    unsigned int order_id;
    unsigned int species_code;
    unsigned int quantity;
    int ret;


    memset(buffer_for_integer, 0, INT_STR_LENGTH);
    memset(final_message, 0, FINAL_MSG_LENGTH);

    init_screen(false);

    if (is_add)
        printf("*** Add a species to already opened order ***\n");
    else
        printf("*** Change the number of plants belonging to a species in an order ***\n");


    printf("Customer code......: %s\n", curr_customer.code);
    
    printf("Insert order id.........: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    printf("Insert species code.....: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    quantity = strtol(buffer_for_integer, NULL, 10);
    
    ret = attempt_to_exec_op_on_order(is_add, order_id, species_code, quantity); 

    if (is_add)
        snprintf(final_message, FINAL_MSG_LENGTH, "added to");
    else
        snprintf(final_message, FINAL_MSG_LENGTH, "update in");

    if (ret == 0)
        printf("Species %u succesfully %s your order (ID %010u)\n", species_code, final_message, order_id);
    else
        printf("Operation failed\n");
    
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_to_remove_spec_from_order(unsigned int order_id, unsigned int species_code)
{
	MYSQL_STMT *order_procedure;
	MYSQL_BIND param[ORDER_SP_NO_PARAMS];

    int deleted_order;

	if(!setup_prepared_stmt(&order_procedure, "call rimuovi_specie_da_ordine(?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(order_procedure, "Unable to initialize remove species from order statement\n");
		goto err_del_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(unsigned int);

	param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[2].buffer = &order_id;
	param[2].buffer_length = sizeof(unsigned int);

    param[3].buffer_type = MYSQL_TYPE_LONG; // OUT var_ordine_eliminato_si_no INT
    param[3].buffer = &deleted_order;
    param[3].buffer_length = sizeof(int);


	if (mysql_stmt_bind_param(order_procedure, param) != 0) 
	{ 
		print_stmt_error(order_procedure, "Could not bind parameters for remove species from order");
		goto err_del;
	}

	if (mysql_stmt_execute(order_procedure) != 0) 
	{
		print_stmt_error(order_procedure, "Could not execute emove species from order procedure");
		goto err_del;
    }

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_ordine_eliminato_si_no INT
	param[0].buffer = &deleted_order;
	param[0].buffer_length = sizeof(int);
	
	if(mysql_stmt_bind_result(order_procedure, param)) 
	{
		print_stmt_error(order_procedure, "Could not retrieve output parameter");
		goto err_del;
	}
	
	if(mysql_stmt_fetch(order_procedure)) 
	{
		print_stmt_error(order_procedure, "Could not buffer results");
		goto err_del;
	}

	mysql_stmt_close(order_procedure);
	return deleted_order;

    err_del:
	mysql_stmt_close(order_procedure);
    err_del_2:
	return -1;    
}

static void remove_spec_from_order(void)
{
    char buffer_for_integer[INT_STR_LENGTH];
    unsigned int order_id;
    unsigned int species_code;
    int deleted_order;

    memset(buffer_for_integer, 0, INT_STR_LENGTH);

    init_screen(false);
    printf("*** Remove a species from an order not closed yet ***\n");
    printf("Customer code......: %s\n", curr_customer.code);
    
    printf("Insert order id.........: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    printf("Insert species code.....: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);
    
    deleted_order = attempt_to_remove_spec_from_order(order_id, species_code); 

    if (deleted_order == -1)
        printf("Operation failed\n");
    else
    {
        printf("Species %u succesfully deleted from your order (ID %010u)\n", species_code, order_id);
        if (deleted_order == 1)
            printf("Order (ID %010u) has been deleted (there were no more plants belonging to it)\n", order_id);
    }

    
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static unsigned int attempt_finalize_order(unsigned int order_id)
{
	MYSQL_STMT *finalize_order_procedure;	
	MYSQL_BIND param[FINALIZE_ORDER_SP_NO_PARAMS];


	if(!setup_prepared_stmt(&finalize_order_procedure, "call finalizza_ordine(?, ?)", conn)) 
    {
		print_stmt_error(finalize_order_procedure, "Unable to initialize finalize order statement\n");
		goto err_fin_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[1].buffer = &order_id;
	param[1].buffer_length = sizeof(unsigned int);

	if (mysql_stmt_bind_param(finalize_order_procedure, param) != 0) 
	{ 
		print_stmt_error(finalize_order_procedure, "Could not bind parameters for finalize order");
		goto err_fin;
	}

	if (mysql_stmt_execute(finalize_order_procedure) != 0) 
	{
		print_stmt_error(finalize_order_procedure, "Could not execute finalize order procedure");
		goto err_fin;
	}

	mysql_stmt_close(finalize_order_procedure);
	return 0;

    err_fin:
	mysql_stmt_close(finalize_order_procedure);
    err_fin_2:
	return 1;    
}

static void finalize_order(void) 
{
    char buffer_for_integer[INT_STR_LENGTH];
    unsigned int order_id;
    int ret;

    init_screen(false);

    printf("*** Finalize an order ***\n");
    printf("Customer code......: %s\n", curr_customer.code);
    printf("Insert order id....: ");
    get_input(INT_STR_LENGTH, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    ret = attempt_finalize_order(order_id);
    if (ret == 0)
        printf("Order %010u has been finalized\n", order_id);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static unsigned int attempt_search_species(char *name)
{
	MYSQL_STMT *search_procedure;	
	MYSQL_BIND param[SEARCH_SP_NO_PARAMS];

	if(!setup_prepared_stmt(&search_procedure, "call visualizza_dettagli_specie(?)", conn)) 
    {
		print_stmt_error(search_procedure, "Unable to initialize search statement\n");
		goto err_search_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_comune	VARCHAR(64)
	param[0].buffer = name;
	param[0].buffer_length = strlen(name);

	if (mysql_stmt_bind_param(search_procedure, param) != 0) 
	{ 
		print_stmt_error(search_procedure, "Could not bind parameters for search species");
		goto err_search;
	}

	if (mysql_stmt_execute(search_procedure) != 0) 
	{
		print_stmt_error(search_procedure, "Could not execute search species procedure");
		goto err_search;
	}

    if (!dump_result_set(search_procedure, "\nSearch results:")) { goto err_search; }
    
	mysql_stmt_close(search_procedure);
	return 0;

    err_search:
	mysql_stmt_close(search_procedure);
    err_search_2:
	return 1;    
}

static void search_species(void) 
{
    char name[REALSIZE(SPEC_NAME_LENGTH)];
    init_screen(false);

    printf("*** Search species by name ***\n");
    printf("Insert the name to filter on (default all): ");
    get_input(SPEC_NAME_LENGTH, name, false, false);

    if (attempt_search_species(name) != 0)
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static unsigned int attempt_update_addr(char *addr, bool is_res)
{
    char sp_str[PROC_STR_LENGTH];
	MYSQL_STMT *addr_procedure;	
	MYSQL_BIND param[UPDATE_ADDR_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call modifica_%s(?, ?)", 
        (is_res) ? "residenza" : "fatturazione");

	if(!setup_prepared_stmt(&addr_procedure, sp_str, conn)) 
    {
		print_stmt_error(addr_procedure, "Unable to initialize update address statement\n");
		goto err_addr_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

    if (strlen(addr) != 0)
    {
        param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_indirizzo VARCHAR(64)
        param[1].buffer = addr;
        param[1].buffer_length = strlen(addr);
    }
    else
    {
        param[1].buffer_type = MYSQL_TYPE_NULL; // IN var_indirizzo VARCHAR(64)
        param[1].buffer = NULL;
        param[1].buffer_length = 0;
    }


	if (mysql_stmt_bind_param(addr_procedure, param) != 0) 
	{ 
		print_stmt_error(addr_procedure, "Could not bind parameters for update address");
		goto err_addr;
	}

	if (mysql_stmt_execute(addr_procedure) != 0) 
	{
		print_stmt_error(addr_procedure, "Could not execute update address procedure");
		goto err_addr;
	}
    
	mysql_stmt_close(addr_procedure);
	return 0;

    err_addr:
	mysql_stmt_close(addr_procedure);
    err_addr_2:
	return 1;    
}

static void update_addr(bool is_res)
{
    char addr[REALSIZE(ADDRESS_LENGTH)];
    init_screen(false);
    int ret;

    printf("*** Update your %s address ***\n", (is_res) ? "residential" : "billing");
    printf("Customer code.......%s: %s\n", (is_res) ? "" : ".............", curr_customer.code);
    printf("Insert new address%s: ", (is_res) ? "" : " (default null)");
    get_input(SPEC_NAME_LENGTH, addr, false, is_res);

    ret = attempt_update_addr(addr, is_res);

    if (ret == 0)
        printf("Address succesfully updated\n");
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static unsigned int attempt_add_contact(char *contact, char *type, bool is_customer)
{
    char sp_str[PROC_STR_LENGTH];
	MYSQL_STMT *contact_procedure;	
	MYSQL_BIND param[ADD_CONTACT_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call aggiungi_contatto_%s(?, ?, ?)", 
        (is_customer) ? "cliente" : "referente");

	if(!setup_prepared_stmt(&contact_procedure, sp_str, conn)) 
    {
		print_stmt_error(contact_procedure, "Unable to initialize add contact statement\n");
		goto err_add_cont_2;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(256)
    param[1].buffer = contact;
    param[1].buffer_length = strlen(contact);

    param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_tipo CHAR(16)
    param[2].buffer = type;
    param[2].buffer_length = strlen(type);

	if (mysql_stmt_bind_param(contact_procedure, param) != 0) 
	{ 
		print_stmt_error(contact_procedure, "Could not bind parameters for add contact");
		goto err_add_cont;
	}

	if (mysql_stmt_execute(contact_procedure) != 0) 
	{
		print_stmt_error(contact_procedure, "Could not execute add contact procedure");
		goto err_add_cont;
	}
    
	mysql_stmt_close(contact_procedure);
	return 0;

    err_add_cont:
	mysql_stmt_close(contact_procedure);
    err_add_cont_2:
	return 1;    
}

static void add_contact(bool is_customer)
{
    char contact[REALSIZE(CONTACT_LENGTH)];
    char type[REALSIZE(TYPE_MAX_LENGTH)];
    init_screen(false);
    int ret;
    char choice;

    printf("*** Add a contact to your %s list ***\n", (is_customer) ? "" : "referent");
    printf("%s code.....: %s\n", (is_customer) ? "Customer" : "Referent", curr_customer.code);

    printf("Insert new contact: ");
    get_input(CONTACT_LENGTH, contact, false, true);

    choice = multi_choice("Select type [m]obile, [l]andline, [e]mail", "mle", 3);

    switch (choice)
    {
        case 'm': snprintf(type, TYPE_MAX_LENGTH, "cellulare"); break;
        case 'l': snprintf(type, TYPE_MAX_LENGTH, "telefono"); break;
        case 'e': snprintf(type, TYPE_MAX_LENGTH, "email"); break;
        default:
            fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
            abort();
    }

    ret = attempt_add_contact(contact, type, is_customer);

    if (ret == 0)
        printf("Contact succesfully added\n");
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static void order_management_menu(void)
{
    char choice;

    while (true)
    {
        init_screen(false);

        printf("*** [ORDER MANAGEMENT] What do you wanna do? ***\n\n");
        printf("1) Open a new order\n");
        printf("2) Add a species to already opened order\n");
        printf("3) Remove a species from an order not closed yet\n");
        printf("4) Change the number of plants belonging to a species in an order\n");
        printf("5) Finalize an order\n");
        printf("6) Search a species by common name\n");
        printf("7) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234567", 7);

        switch (choice)
        {
            case '1': open_order(); break;
            case '2': exec_op_on_order(true); break;
            case '3': remove_spec_from_order(); break;
            case '4': exec_op_on_order(false); break;
            case '5': finalize_order(); break; 
            case '6': search_species(); break;
            case '7': return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}

static void profile_management_menu(void)
{
    char choice;

    while (true)
    {
        init_screen(false);

        printf("*** [PROFILE MANAGEMENT] What do you wanna do? ***\n\n");
        printf("1) Update your residential address\n");
        printf("2) Update your billing address\n");
        printf("3) Add a contact to your list\n");
        printf("4) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234", 4);

        switch (choice)
        {
            case '1': update_addr(true); break;
            case '2': update_addr(false); break;
            case '3': add_contact(true); break;
            case '4': return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}

static void main_menu(void)
{
    char choice;

    while (true)
    {
        init_screen(true);
        printf("Welcome %s (%s)\n\n", curr_customer.username, curr_customer.code);
        printf("*** What do you wanna do? ***\n\n");
        printf("1) Orders management\n");
        printf("2) Profile management\n");
		printf("3) Quit\n");

        choice = multi_choice("Pick an option", "123", 3);

        switch (choice)
        {
            case '1': order_management_menu(); break;
            case '2': profile_management_menu(); break;
            case '3': printf("Bye bye!\n\n\n"); return;
            default:
            	fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
		        abort();
        }
    }
}

void run_as_customer(char *username, char *customer_code, bool is_private)
{
    char path[PATH_LENGTH];
    config_t cnf;

    memset(path, 0, PATH_LENGTH);   
    memset(&curr_customer, 0, sizeof(customer_info_t));
    memset(&cnf, 0, sizeof(config_t));

    memcpy(curr_customer.code, customer_code, CUSTOMER_CODE_MAX_LENGTH);
    memcpy(curr_customer.username, username, CRED_LENGTH);
    curr_customer.is_private = is_private;

    printf("%s\n", curr_customer.code);

    snprintf(path, PATH_LENGTH, "config/%s.user", (is_private) ? "clp" : "clr");

    if (parse_config(path, &cnf, "="))
    {
        fprintf(stderr, "Invalid configuration file selected (%s)\n", (is_private) ? "CLP" : "CLR");
        exit(EXIT_FAILURE);
    }

	if(mysql_change_user(conn, cnf.username, cnf.password, cnf.database)) {
		fprintf(stderr, "Unable to switch privileges\n");
		exit(EXIT_FAILURE);
	}

    main_menu();

    return;
}