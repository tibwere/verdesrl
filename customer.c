#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
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
#define MODIFY_CONTACT_LIST_SP_NO_PARAMS 2

/* bitmask: 00000001 (prima colonna dell'output) */
#define LEADING_ZERO_BITMASK_SEARCH 1 

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
	MYSQL_STMT *stmt;
	
	MYSQL_BIND param[CREATE_ORDER_SP_NO_PARAMS];
	unsigned int id;

	if(!setup_prepared_stmt(&stmt, "call crea_ordine(?, ?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 0;
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

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_id INT
	param[0].buffer = &id;
	param[0].buffer_length = sizeof(unsigned int);
	
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
	return id;
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
	MYSQL_STMT *stmt;
	MYSQL_BIND param[ORDER_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call %s(?, ?, ?, ?)", 
        (is_add) ? "aggiungi_specie_ad_ordine_esistente" : "modifica_ordine");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
	    return 1; 
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


	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(1);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute selected the statement");
		CLOSEANDRET(1);
    }

	mysql_stmt_close(stmt);
	return 0; 
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
	MYSQL_STMT *stmt;
	MYSQL_BIND param[ORDER_SP_NO_PARAMS];

    int deleted_order;

	if(!setup_prepared_stmt(&stmt, "call rimuovi_specie_da_ordine(?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return -1;
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

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_ordine_eliminato_si_no INT
	param[0].buffer = &deleted_order;
	param[0].buffer_length = sizeof(int);
	
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
	return deleted_order;  
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
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[FINALIZE_ORDER_SP_NO_PARAMS];


	if(!setup_prepared_stmt(&stmt, "call finalizza_ordine(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 1;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[1].buffer = &order_id;
	param[1].buffer_length = sizeof(unsigned int);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(1); 	
    }

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
    	CLOSEANDRET(1); 
	}

	mysql_stmt_close(stmt);
	return 0;  
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
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[SEARCH_SP_NO_PARAMS];
    char prompt[2 * SPEC_NAME_LENGTH];

	if(!setup_prepared_stmt(&stmt, "call visualizza_dettagli_specie(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return 1;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_nome_comune	VARCHAR(64)
	param[0].buffer = name;
	param[0].buffer_length = strlen(name);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(1);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
        CLOSEANDRET(1);	
    }

    if (strlen(name) > 0)
        snprintf(prompt, SPEC_NAME_LENGTH, "\nSearch results for \'%s\':", name);
    else
        snprintf(prompt, SPEC_NAME_LENGTH, "\nSearch results:");

    if (!dump_result_set(stmt, prompt, LEADING_ZERO_BITMASK_SEARCH)) 
    {
        CLOSEANDRET(1);
    }
    
	mysql_stmt_close(stmt);
	return 0;
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
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[UPDATE_ADDR_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call modifica_%s(?, ?)", 
        (is_res) ? "residenza" : "fatturazione");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 1;
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


	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(1);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute the statement");
        CLOSEANDRET(1);
	}
    
	mysql_stmt_close(stmt);
	return 0;   
}

static void update_addr(bool is_res)
{
    char addr[REALSIZE(ADDRESS_LENGTH)];
    int ret;

    init_screen(false);

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

static unsigned int attempt_to_modify_contact_list(char *contact, bool is_customer, bool to_delete)
{
    char sp_str[PROC_STR_LENGTH];
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[MODIFY_CONTACT_LIST_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    if (to_delete)
    {
        if (is_customer)
            snprintf(sp_str, PROC_STR_LENGTH, "call rimuovi_contatto_cliente(?, ?)");
        else   
            snprintf(sp_str, PROC_STR_LENGTH, "call rimuovi_contatto_referente(?, ?)");
    }
    else
    {
        if (is_customer)
            snprintf(sp_str, PROC_STR_LENGTH, "call modifica_contatto_preferito_cliente(?, ?)");
        else   
            snprintf(sp_str, PROC_STR_LENGTH, "call modifica_contatto_preferito_referente(?, ?)");
    }

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 1;
	}

	memset(param, 0, sizeof(param));
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(256)
    param[1].buffer = contact;
    param[1].buffer_length = strlen(contact);

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
		CLOSEANDRET(1);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
        if (to_delete && (mysql_stmt_errno(stmt) == 1451))
			fprintf(stderr, "Cannot remove contact because it is associated to an order!\n");
        else if (!to_delete && (mysql_stmt_errno(stmt) == 1452))
            fprintf(stderr, "Cannot set this contact as favourite one because you don't own it!\n");
		else
			print_stmt_error(stmt, "Could not execute the statement");
		CLOSEANDRET(1);
	}
    
	mysql_stmt_close(stmt);
	return 0;  
}

static void modify_contact_list(bool is_customer, bool to_delete)
{
    char contact[REALSIZE(CONTACT_LENGTH)];
    int ret;

    init_screen(false);
    
    if (to_delete)
        printf("*** Remove a contact from your %s list ***\n", (is_customer) ? "" : "referent");
    else
        printf("*** Change %s favourite contact %s ***\n", 
            (is_customer) ? "your" : "",
            (is_customer) ? "" : "of your referent");
 
    printf("%s code.: %s\n", (is_customer) ? "Customer" : "Referent", curr_customer.code);
    printf("Insert contact: ");
    get_input(CONTACT_LENGTH, contact, false, true);

    ret = attempt_to_modify_contact_list(contact, is_customer, to_delete);

    if (ret == 0)
        printf("Contact succesfully %s\n", (to_delete) ? "removed" : "changed");
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static unsigned int attempt_add_contact(char *contact, char *type, bool is_customer)
{
    char sp_str[PROC_STR_LENGTH];
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[ADD_CONTACT_SP_NO_PARAMS];

    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, PROC_STR_LENGTH, "call aggiungi_contatto_%s(?, ?, ?)", 
        (is_customer) ? "cliente" : "referente");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 1;
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

	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
		CLOSEANDRET(1);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
        if (mysql_stmt_errno(stmt) == 1062)
			fprintf(stderr, "Contact already inserted!\n");
		else
			print_stmt_error(stmt, "Could not execute the statement");
		CLOSEANDRET(1);
	}
    
	mysql_stmt_close(stmt);
	return 0;  
}

static void add_contact(bool is_customer, bool show_prompt)
{
    char contact[REALSIZE(CONTACT_LENGTH)];
    char type[REALSIZE(TYPE_MAX_LENGTH)];
    int ret;
    char choice;

    init_screen(false);

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
    {
        printf("Contact succesfully added\n");
        choice = multi_choice("Do you wanna set this as your favourite contact?", "yn", 2);
        if (choice == 'y')
        {
            ret = attempt_to_modify_contact_list(contact, is_customer, false);
            if (ret == 0)
                printf("Contact succesfully changed\n");
            else
                printf("Operation failed\n");
        }
    }
    else
        printf("Operation failed\n");
        
    if (show_prompt)
    {
        printf("Press enter key to get back to menu ...\n");
        getchar();
    }
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
        printf("4) Remove contact from your list\n");
        printf("5) Change your favourite contact\n");
        printf("6) Back to main menu\n");

        choice = multi_choice("Pick an option", "123456", 6);

        switch (choice)
        {
            case '1': update_addr(true); break;
            case '2': update_addr(false); break;
            case '3': add_contact(true, true); break;
            case '4': modify_contact_list(true, true); break;
            case '5': modify_contact_list(true, false); break;
            case '6': return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}

static void referent_management_menu(void)
{
    char choice;

    while (true)
    {
        init_screen(false);

        printf("*** [REFERENT MANAGEMENT] What do you wanna do? ***\n\n");
        printf("1) Add a contact to your referent list\n");
        printf("2) Remove contact from your referent list\n");
        printf("3) Change favourite contact of your referent\n");
        printf("4) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234", 4);

        switch (choice)
        {
            case '1': update_addr(true); break;
            case '2': update_addr(false); break;
            case '3': add_contact(true, true); break;
            case '4': modify_contact_list(true, true); break;
            case '5': modify_contact_list(true, false); break;
            case '6': return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}

static void main_menu(void)
{
    char choice;
    if (curr_customer.is_private)
    {
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
    else
    {
        while (true)
        {
            init_screen(true);
            printf("Welcome %s (%s)\n\n", curr_customer.username, curr_customer.code);
            printf("*** What do you wanna do? ***\n\n");
            printf("1) Orders management\n");
            printf("2) Profile management\n");
            printf("3) Referent management\n");
            printf("4) Quit\n");

            choice = multi_choice("Pick an option", "1234", 4);

            switch (choice)
            {
                case '1': order_management_menu(); break;
                case '2': profile_management_menu(); break;
                case '3': referent_management_menu(); break;
                case '4': printf("Bye bye!\n\n\n"); return;
                default:
                    fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                    abort();
            }
        }        
    }

}

void run_as_customer(char *username, char *customer_code, bool is_private, bool first_access)
{
    char path[PATH_LENGTH];
    config_t cnf;
    char choice;

    memset(path, 0, PATH_LENGTH);   
    memset(&curr_customer, 0, sizeof(customer_info_t));
    memset(&cnf, 0, sizeof(config_t));

    memcpy(curr_customer.code, customer_code, CUSTOMER_CODE_MAX_LENGTH);
    memcpy(curr_customer.username, username, CRED_LENGTH);
    curr_customer.is_private = is_private;

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

    if (first_access)
    {
        choice = multi_choice("Do you wanna insert a contact?", "yn", 2);
        if (choice == 'y')
        {
            add_contact(true, false);
            sleep(1.5);
        }
    }

    main_menu();
}