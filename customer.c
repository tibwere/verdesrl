#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "defines.h"

#define PATH_LENGTH 16
#define CREATE_ORDER_SP_NO_PARAMS 6

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
		goto err2;
	}

	memset(param, 0, sizeof(MYSQL_BIND));
	
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
		goto err;
	}

	if (mysql_stmt_execute(create_order_procedure) != 0) 
	{
		print_stmt_error(create_order_procedure, "Could not execute create order procedure");
		goto err;
	}

	param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_id INT
	param[0].buffer = &id;
	param[0].buffer_length = sizeof(unsigned int);
	
	if(mysql_stmt_bind_result(create_order_procedure, param)) 
	{
		print_stmt_error(create_order_procedure, "Could not retrieve output parameter");
		goto err;
	}
	
	if(mysql_stmt_fetch(create_order_procedure)) 
	{
		print_stmt_error(create_order_procedure, "Could not buffer results");
		goto err;
	}

	mysql_stmt_close(create_order_procedure);
	return id;

    err:
	mysql_stmt_close(create_order_procedure);
    err2:
	return 0;    
}

static void open_order(void)
{
    order_sp_params_t params;
    char code_str[INT_STR_LENGTH];
    char quantity_str[INT_STR_LENGTH];
    unsigned int order_id;

    memset(&params, 0, sizeof(order_sp_params_t));
    memset(&code_str, 0, INT_STR_LENGTH);

    init_screen(false);

    printf("*** Open a new order ***\n");
    printf("Customer code........................................: %s\n", curr_customer.code);
    
    printf("Insert shipping address (default residential address): ");
    get_input(ADDRESS_LENGTH, params.shipping_address, false);

    printf("Insert contact (default favourite one)...............: ");
    get_input(CONTACT_LENGTH, params.contact, false);

    printf("Insert species code..................................: ");
    get_input(INT_STR_LENGTH, code_str, false);
    params.species = strtol(code_str, NULL, 10);

    printf("Insert relative quantity.............................: ");
    get_input(INT_STR_LENGTH, quantity_str, false);
    params.quantity = strtol(quantity_str, NULL, 10);

    order_id = attempt_open_order(&params);
    if (order_id > 0)
        printf("New order opened (ID: %010u)\n", order_id);
    else
        printf("No order has opened\n");
        
    printf("Press any key to get back to menu ...\n");
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
        printf("2) Back to main menu\n");

        choice = multi_choice("Pick an option", "12", 2);

        switch (choice)
        {
            case '1': open_order(); break;
            case '2': return;
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
            case '2': printf("Profile management\n"); break;
            case '3': return;
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