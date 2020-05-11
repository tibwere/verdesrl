#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "defines.h"

typedef struct customer_info
{
    char username[BUFFSIZE_L];
    char code[BUFFSIZE_XS];
    bool is_private;
} customer_info_t;

typedef struct order_sp_params
{
    char shipping_address[BUFFSIZE_M];
    char contact[BUFFSIZE_XL];
    unsigned int species;
    unsigned int quantity;
} order_sp_params_t;

typedef struct order_info
{
    unsigned int id;
    char date[BUFFSIZE_S];
    char shipping_address[BUFFSIZE_M];
    char chosen_contact[BUFFSIZE_XL];
    char status[BUFFSIZE_XS];
    /* 
     * nel report viene presentato come <nome> <cognome> 
     * -> 32 + 1 + 32 + 1 = 66
     */
    char referent[2 * BUFFSIZE_S]; 
} order_info_t;


static customer_info_t curr_customer;


static bool attempt_report_orders_short(bool only_open)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];

    printf("%d\n", only_open);

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call visualizza_ordini_cliente(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_TINY; // IN var_status TINYINT
	param[1].buffer = &(only_open);
	param[1].buffer_length = sizeof(bool);

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

    if (!dump_result_set(stmt, "Open orders:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;
}

static bool attempt_search_species_belonging_to_order(unsigned int order_id)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call visualizza_specie_appartenenti_ad_ordine(?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[0].buffer = &order_id;
	param[0].buffer_length = sizeof(order_id);

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

    if (!dump_result_set(stmt, "Species belonging to selected order:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;    
}

static void search_species_belonging_to_order(unsigned int order_id, unsigned int dots)
{
    char prompt[BUFFSIZE_XL];
    char choice;
    
    memset(prompt, 0, sizeof(prompt));

    putchar('\n');

    format_prompt(prompt, BUFFSIZE_XL, "Do you wanna see a list of species belonging to selected order", dots);
        
    choice = multi_choice(prompt, "yn", 2);
    if (choice == 'y')
    {
        if (!attempt_search_species_belonging_to_order(order_id))
            printf("Operation failed\n");

        putchar('\n');
    }
}

static void order_tips(bool only_open, unsigned int dots)
{
    char prompt[BUFFSIZE_XL];
    char choice;
    
    memset(prompt, 0, sizeof(prompt));

    putchar('\n');

    if (only_open)
        format_prompt(prompt, BUFFSIZE_XL, "Do you wanna see a report of your open orders", dots);
    else    
        format_prompt(prompt, BUFFSIZE_XL, "Do you wanna see a report of your orders", dots);

    choice = multi_choice(prompt, "yn", 2);
    if (choice == 'y')
    {
        if (!attempt_report_orders_short(only_open))
            printf("Operation failed\n");

        putchar('\n');
    }
}

static unsigned int attempt_open_order(order_sp_params_t *input)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[6];
	unsigned int id;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call crea_ordine(?, ?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return 0;
	}
	
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
	param[3].buffer_length = sizeof(input->species);

	param[4].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
	param[4].buffer = &(input->quantity);
	param[4].buffer_length = sizeof(input->quantity);

    param[5].buffer_type = MYSQL_TYPE_LONG; // OUT var_id INT
	param[5].buffer = &id;
	param[5].buffer_length = sizeof(id);

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
	param[0].buffer_length = sizeof(id);
	
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
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;

    memset(&params, 0, sizeof(params));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** Open a new order ***\n");
    printf("Customer code....................................................: %s\n", curr_customer.code);
    
    printf("Insert shipping address (default residential address)............: ");
    get_input(BUFFSIZE_M, params.shipping_address, false, false);

    printf("Insert contact (default favourite one)...........................: ");
    get_input(BUFFSIZE_XL, params.contact, false, false);

    species_tips(0);
    printf("Insert species code..............................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    params.species = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity.........................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    params.quantity = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    order_id = attempt_open_order(&params);
    if (order_id > 0)
        printf("New order opened (ID: %010u)\n", order_id);
    else
        printf("No order has opened\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_to_add_spec_to_order(unsigned int order_id, unsigned int species_code, unsigned int quantity)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[4];

   	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call aggiungi_specie_ad_ordine_esistente(?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
	    return false; 
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(species_code);

	param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[2].buffer = &order_id;
	param[2].buffer_length = sizeof(order_id);

    param[3].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
    param[3].buffer = &quantity;
    param[3].buffer_length = sizeof(quantity);


	if (mysql_stmt_bind_param(stmt, param) != 0) 
	{ 
		print_stmt_error(stmt, "Could not bind parameters for the statement");
        CLOSEANDRET(false);
	}

	if (mysql_stmt_execute(stmt) != 0) 
	{
		print_stmt_error(stmt, "Could not execute selected the statement");
		CLOSEANDRET(false);
    }

	mysql_stmt_close(stmt);
	return true; 
}

static int attempt_to_modify_order(unsigned int order_id, unsigned int species_code, unsigned int quantity)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[5];
    int affected_rows;

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call modifica_ordine(?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return -1;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(species_code);

	param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[2].buffer = &order_id;
	param[2].buffer_length = sizeof(order_id);

    param[3].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
    param[3].buffer = &quantity;
    param[3].buffer_length = sizeof(quantity);

    param[4].buffer_type = MYSQL_TYPE_LONG; // OUT var_aggiornamento_eff INT
    param[4].buffer = &affected_rows;
    param[4].buffer_length = sizeof(affected_rows);

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

    param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_aggiornamento_eff INT
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

static void exec_op_on_order(bool is_add)
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;
    unsigned int species_code;
    unsigned int quantity;
    int ret;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    if (is_add)
        printf("*** Add a species to already opened order ***\n");
    else
        printf("*** Change the number of plants belonging to a species in an order ***\n");


    printf("Customer code....................................................: %s\n", curr_customer.code);

    order_tips(true, 13);    
    printf("Insert order id..................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    species_tips(0);
    printf("Insert species code..............................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity.........................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    quantity = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');
    
    if (is_add)
    {
        if (attempt_to_add_spec_to_order(order_id, species_code, quantity))
            printf("Species %u succesfully added to your order (ID %010u)\n", species_code, order_id);
        else
            printf("Operation failed\n");
    }
    else
    {
        ret = attempt_to_modify_order(order_id, species_code, quantity);
        if (ret == 0)
            printf("Nothing has changed (species %u not in order [ID %010u])\n", species_code, order_id);
        else if (ret > 0)
            printf("Species %u succesfully updated in your order (ID %010u)\n", species_code, order_id);
        else
            printf("Operation failed\n");
    }
 
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_to_remove_spec_from_order(unsigned int order_id, unsigned int species_code, int *order_status)
{
	MYSQL_STMT *stmt;
	MYSQL_BIND param[5];
    int affected_rows;

    memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call rimuovi_specie_da_ordine(?, ?, ?, ?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return -1;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
	param[1].buffer = &species_code;
	param[1].buffer_length = sizeof(species_code);

	param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[2].buffer = &order_id;
	param[2].buffer_length = sizeof(order_id);

    param[3].buffer_type = MYSQL_TYPE_LONG; // OUT var_ordine_eliminato_si_no INT
    param[3].buffer = order_status;
    param[3].buffer_length = sizeof(*order_status);

    param[4].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_eff INT
    param[4].buffer = &affected_rows;
    param[4].buffer_length = sizeof(affected_rows);

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
    param[0].buffer = order_status;
    param[0].buffer_length = sizeof(*order_status);

    param[1].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_eff INT
    param[1].buffer = &affected_rows;
    param[1].buffer_length = sizeof(affected_rows);
	
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

static void remove_spec_from_order(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;
    unsigned int species_code;
    int order_status;
    int ret;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);
    printf("*** Remove a species from an order not closed yet ***\n");
    printf("Customer code........................................................: %s\n", curr_customer.code);
    
    order_tips(true, 17);
    printf("Insert order id......................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    search_species_belonging_to_order(order_id, 0);
    printf("Insert species code..................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');
    
    ret = attempt_to_remove_spec_from_order(order_id, species_code, &order_status); 

    if (ret > 0)
    {
        printf("Species %u succesfully deleted from your order (ID %010u)\n", species_code, order_id);
        if (order_status == 1)
            printf("Order (ID %010u) has been deleted (there were no more plants belonging to it)\n", order_id);
    }
    else if (ret == 0)
    {
        printf("Nothing has changed (species %u not in order [ID %010u])\n", species_code, order_id);
    }
    else
    {
        printf("Operation failed\n");
    }

    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static int attempt_finalize_order(unsigned int order_id)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];

	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call finalizza_ordine(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[1].buffer = &order_id;
	param[1].buffer_length = sizeof(order_id);

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

static void finalize_order(void) 
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** Finalize an order ***\n");
    printf("Customer code.......................................: %s\n", curr_customer.code);

    order_tips(true, 0);
    printf("Insert order id.....................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (attempt_finalize_order(order_id))
        printf("Order %010u has been finalized\n", order_id);
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_update_addr(char *addr, bool is_res)
{
    char sp_str[BUFFSIZE_L];
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];

    memset(sp_str, 0, sizeof(sp_str));
	memset(param, 0, sizeof(param));

    snprintf(sp_str, BUFFSIZE_L, "call modifica_%s(?, ?)", 
        (is_res) ? "residenza" : "fatturazione");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}
	
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

static void update_addr(bool is_res)
{
    char addr[BUFFSIZE_M];

    memset(addr, 0, sizeof(addr));

    init_screen(false);

    printf("*** Update your %s address ***\n", (is_res) ? "residential" : "billing");
    printf("Customer code......%s: %s\n", (is_res) ? "" : ".............", curr_customer.code);
    printf("Insert new address %s: ", (is_res) ? "" : " (default null)");
    get_input(BUFFSIZE_M, addr, false, is_res);

    putchar('\n');

    if (attempt_update_addr(addr, is_res))
        printf("Address succesfully updated\n");
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_show_contact_list(bool is_customer)
{
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[1];
    char sp_str[BUFFSIZE_L];

	memset(param, 0, sizeof(param));
    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, BUFFSIZE_L, "call visualizza_contatti_%s(?)", (is_customer) ? "cliente" : "referente");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
        return false;
	}
	
	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

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

    if (!dump_result_set(stmt, "\nContact list:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }
    
	mysql_stmt_close(stmt);
	return true;       
}

static void show_contact_list(bool is_customer, unsigned int dots)
{
    char prompt[BUFFSIZE_XL];
    char choice;
    
    memset(prompt, 0, sizeof(prompt));

    putchar('\n');

    if (is_customer)
        format_prompt(prompt, BUFFSIZE_XL, "Do you wanna see a report of your contacts", dots);
    else    
        format_prompt(prompt, BUFFSIZE_XL, "Do you wanna see a report of your referent contacts", dots);

    choice = multi_choice(prompt, "yn", 2);
    if (choice == 'y')
    {
        if (!attempt_show_contact_list(is_customer))
            printf("Operation failed\n");

        putchar('\n');
    }
}

static bool attempt_to_modify_contact_list(char *contact, bool is_customer, bool to_delete)
{
    char sp_str[BUFFSIZE_L];
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];

    memset(sp_str, 0, sizeof(sp_str));
   	memset(param, 0, sizeof(param));

    if (to_delete)
    {
        if (is_customer)
            snprintf(sp_str, BUFFSIZE_L, "call rimuovi_contatto_cliente(?, ?)");
        else   
            snprintf(sp_str, BUFFSIZE_L, "call rimuovi_contatto_referente(?, ?)");
    }
    else
    {
        if (is_customer)
            snprintf(sp_str, BUFFSIZE_L, "call modifica_contatto_preferito_cliente(?, ?)");
        else   
            snprintf(sp_str, BUFFSIZE_L, "call modifica_contatto_preferito_referente(?, ?)");
    }

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}

	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(256)
    param[1].buffer = contact;
    param[1].buffer_length = strlen(contact);

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

static void modify_contact_list(bool is_customer, bool to_delete)
{
    char contact[BUFFSIZE_XL];

    memset(contact, 0, sizeof(contact));

    init_screen(false);
    
    if (to_delete)
        printf("*** Remove a contact from your %s list ***\n", (is_customer) ? "" : "referent");
    else
        printf("*** Change %s favourite contact %s ***\n", 
            (is_customer) ? "your" : "",
            (is_customer) ? "" : "of your referent");
    
    printf("%s code....................................: %s\n", (is_customer) ? "Customer" : "Referent", curr_customer.code);

    show_contact_list(is_customer, 0);
    printf("Insert contact.............................: ");
    get_input(BUFFSIZE_XL, contact, false, true);

    putchar('\n');

    if (attempt_to_modify_contact_list(contact, is_customer, to_delete))
        printf("Contact succesfully %s\n", (to_delete) ? "removed" : "changed");
    else
        printf("Operation failed\n");
        
    printf("Press enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_add_contact(char *contact, char *type, bool is_customer)
{
    char sp_str[BUFFSIZE_L];
	MYSQL_STMT *stmt;	
	MYSQL_BIND param[3];

    memset(sp_str, 0, sizeof(sp_str));
	memset(param, 0, sizeof(param));

    snprintf(sp_str, BUFFSIZE_L, "call aggiungi_contatto_%s(?, ?, ?)", 
        (is_customer) ? "cliente" : "referente");

	if(!setup_prepared_stmt(&stmt, sp_str, conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}

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

static void add_contact(bool is_customer, bool show_prompt)
{
    char contact[BUFFSIZE_XL];
    char type[BUFFSIZE_XS];
    char choice;

    memset(contact, 0, sizeof(contact));
    memset(type, 0, sizeof(type));

    init_screen(false);

    printf("*** Add a contact to your %s list ***\n", (is_customer) ? "" : "referent");
    printf("%s code....................................: %s\n", (is_customer) ? "Customer" : "Referent", curr_customer.code);

    show_contact_list(is_customer, 0);
    printf("Insert new contact...............................: ");
    get_input(BUFFSIZE_XL, contact, false, true);

    choice = multi_choice("Select type [m]obile, [l]andline, [e]mail", "mle", 3);

    switch (choice)
    {
        case 'm': snprintf(type, BUFFSIZE_XS, "cellulare"); break;
        case 'l': snprintf(type, BUFFSIZE_XS, "telefono"); break;
        case 'e': snprintf(type, BUFFSIZE_XS, "email"); break;
        default:
            fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
            abort();
    }

    putchar('\n');

    if (attempt_add_contact(contact, type, is_customer))
    {
        choice = multi_choice("\nDo you wanna set this as favourite contact?", "yn", 2);
        if (choice == 'y')
        {
            if (attempt_to_modify_contact_list(contact, is_customer, false))
                printf("Contact succesfully changed\n");
            else
                printf("Operation failed\n");
        }
        else return;
    }
    else
        printf("Operation failed\n");
        
    if (show_prompt)
    {
        printf("Press enter key to get back to menu ...\n");
        getchar();
    }
}

static bool get_order_info(MYSQL_STMT *stmt, order_info_t *info)
{
    MYSQL_TIME date;
    int status;
    MYSQL_BIND param[6];
    char referent[(2 * BUFFSIZE_S)];
  	my_bool is_null;

    memset(&date, 0, sizeof(date));
    memset(referent, 0, sizeof(referent));
	memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG;
	param[0].buffer = &(info->id);
	param[0].buffer_length = sizeof(info->id);

    param[1].buffer_type = MYSQL_TYPE_DATETIME;
	param[1].buffer = &date;
	param[1].buffer_length = sizeof(date);

    param[2].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[2].buffer = info->shipping_address;
	param[2].buffer_length = BUFFSIZE_M * sizeof(char);

    param[3].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[3].buffer = info->chosen_contact;
	param[3].buffer_length = BUFFSIZE_XL * sizeof(char);

    param[4].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[4].buffer = info->status;
	param[4].buffer_length = BUFFSIZE_XS  * sizeof(char);

    param[5].buffer_type = MYSQL_TYPE_VAR_STRING;
	param[5].buffer = referent;
	param[5].buffer_length = (2 * BUFFSIZE_S) * sizeof(char);
    param[5].is_null = &is_null;


    if(mysql_stmt_bind_result(stmt, param)) 
	{
		print_stmt_error(stmt, "Unable to bind output parameters");
		return false;
	}
    
    status = mysql_stmt_fetch(stmt);
    
    if (status == 1 || status == MYSQL_NO_DATA)
        return false;
    
    snprintf(info->date, BUFFSIZE_S, "%4d-%02d-%02d %02d:%02d:%02d", 
        date.year, date.month, date.day, date.hour, date.minute, date.second);

    if (is_null)
        snprintf(info->referent, (2 * BUFFSIZE_S), "Not expected");
    else   
        strncpy(info->referent, referent, (2 * BUFFSIZE_S));

    return true;
}

static bool attempt_report_order(unsigned int order_id)
{
   	MYSQL_STMT *stmt;	
	MYSQL_BIND param[2];
    order_info_t order;

    memset(&order, 0, sizeof(order));
	memset(param, 0, sizeof(param));

	if(!setup_prepared_stmt(&stmt, "call report_ordine(?, ?)", conn)) 
    {
		print_stmt_error(stmt, "Unable to initialize the statement\n");
		return false;
	}

	param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
	param[0].buffer = curr_customer.code;
	param[0].buffer_length = strlen(curr_customer.code);

	param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
	param[1].buffer = &order_id;
	param[1].buffer_length = sizeof(order_id);

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

    if (get_order_info(stmt, &order))
    {
        printf("\n\nOrder info [ID %010u]\n", order.id);
        printf("Date.............: %s\n", order.date);
        printf("Shipping address.: %s\n", order.shipping_address);
        printf("Chosen contact...: %s\n", order.chosen_contact);
        printf("Status...........: %s\n", order.status);
        printf("Referent.........: %s\n", order.referent);
    }

	if (mysql_stmt_next_result(stmt))
    {
        print_stmt_error(stmt, "Unexpected condition");
        CLOSEANDRET(false); 
    } 

    if (!dump_result_set(stmt, "\n\nInvolved species:", LEADING_ZERO_BITMASK_IDX_0)) 
    {
        CLOSEANDRET(false);
    }

	if (mysql_stmt_next_result(stmt))
    {
        print_stmt_error(stmt, "Unexpected condition");
        CLOSEANDRET(false); 
    }

    if (!dump_result_set(stmt, "\n\nEconiomic details:", 0)) 
    {
        CLOSEANDRET(false);
    }    

	mysql_stmt_close(stmt);
	return true;   
}

static void report_order(void) 
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** View order details ***\n");
    printf("Customer code..................................: %s\n", curr_customer.code);

    order_tips(false, 0);
    printf("Insert order id................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');
    
    if (!attempt_report_order(order_id))
        printf("Operation failed\n");
        
    printf("\nPress enter key to get back to menu ...\n");
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
        printf("7) View order details\n");
        printf("q) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234567q", 8);

        switch (choice)
        {
            case '1': open_order(); break;
            case '2': exec_op_on_order(true); break;
            case '3': remove_spec_from_order(); break;
            case '4': exec_op_on_order(false); break;
            case '5': finalize_order(); break; 
            case '6': search_species(); break;
            case '7': report_order(); break;
            case 'q': return;
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
        printf("q) Back to main menu\n");

        choice = multi_choice("Pick an option", "12345q", 6);

        switch (choice)
        {
            case '1': update_addr(true); break;
            case '2': update_addr(false); break;
            case '3': add_contact(true, true); break;
            case '4': modify_contact_list(true, true); break;
            case '5': modify_contact_list(true, false); break;
            case 'q': return;
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
        printf("q) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234", 4);

        switch (choice)
        {
            case '1': add_contact(false, true); break;
            case '2': modify_contact_list(false, true); break;
            case '3': modify_contact_list(false, false); break;
            case 'q': return;
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
            printf("3) Change password\n");
            printf("q) Quit\n");

            choice = multi_choice("Pick an option", "123q", 4);

            switch (choice)
            {
                case '1': order_management_menu(); break;
                case '2': profile_management_menu(); break;
                case '3': change_password(curr_customer.username); break;
                case 'q': printf("Bye bye!\n\n\n"); return;
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
            printf("4) Change password\n");
            printf("q) Quit\n");

            choice = multi_choice("Pick an option", "1234q", 5);

            switch (choice)
            {
                case '1': order_management_menu(); break;
                case '2': profile_management_menu(); break;
                case '3': referent_management_menu(); break;
                case '4': change_password(curr_customer.username); break;
                case 'q': printf("Bye bye!\n\n\n"); return;
                default:
                    fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                    abort();
            }
        }        
    }

}

void run_as_customer(char *username, char *customer_code, bool is_private, bool first_access)
{
    char path[BUFFSIZE_M];
    config_t cnf;
    char choice;

    memset(path, 0, sizeof(path));   
    memset(&curr_customer, 0, sizeof(curr_customer));
    memset(&cnf, 0, sizeof(cnf));

    memcpy(curr_customer.code, customer_code, BUFFSIZE_XS);
    memcpy(curr_customer.username, username, BUFFSIZE_L);
    curr_customer.is_private = is_private;

    snprintf(path, BUFFSIZE_M, "config/%s.user", (is_private) ? "clp" : "clr");

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
        choice = multi_choice("Do you wanna insert a contact......?", "yn", 2);
        if (choice == 'y')
        {
            add_contact(true, false);
            sleep(1.5);
        }
    }

    main_menu();
}