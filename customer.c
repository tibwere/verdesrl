#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "defines.h"

struct customer_info {
    char username[BUFFSIZE_L];
    char code[BUFFSIZE_XS];
    bool is_private;
};

struct create_order_sp_params {
    char shipping_address[BUFFSIZE_M];
    char contact[BUFFSIZE_XL];
    unsigned int species;
    unsigned int quantity;
};

struct order_info {
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
};


static struct customer_info curr_customer;


static bool attempt_search_species_belonging_to_order(unsigned int order_id)
{
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[1];

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[0].buffer = &order_id;
    param[0].buffer_length = sizeof(order_id);

    if (!exec_sp(&stmt, param, "call visualizza_specie_appartenenti_ad_ordine(?)"))
        return false;

    if (!dump_result_set(stmt, "Species belonging to selected order:", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }

    mysql_stmt_close(stmt);
    return true;    
}

static void search_species(void) 
{
    char name[BUFFSIZE_M];

    memset(name, 0, sizeof(name));

    init_screen(false);

    printf("*** Search species by name ***\n");
    printf("Insert the name to filter on (default all): ");
    get_input(BUFFSIZE_M, name, false, false);

    putchar('\n');

    if (!attempt_search_species(false, name))
        printf("Operation failed\n");

    printf("\nPress enter key to get back to menu ...\n");
    getchar();
}

static bool attempt_report_orders_short(bool only_open)
{
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[2];
    
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_TINY; // IN var_status TINYINT
    param[1].buffer = &(only_open);
    param[1].buffer_length = sizeof(only_open);

    if (!exec_sp(&stmt, param, "call visualizza_ordini_cliente(?, ?)"))
        return false;

    if (!dump_result_set(stmt, "Open orders:", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }

    mysql_stmt_close(stmt);
    return true;
}

static unsigned int attempt_open_order(struct create_order_sp_params *input)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND param[6];
    unsigned int id;

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    if (strlen(input->shipping_address) > 0) {
        param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_ind_sped VARCHAR(64)
        param[1].buffer = input->shipping_address;
        param[1].buffer_length = strlen(input->shipping_address);
    } else {
        param[1].buffer_type = MYSQL_TYPE_NULL; // IN var_ind_sped VARCHAR(64)
        param[1].buffer = NULL;
        param[1].buffer_length = 0;	
    }

    if (strlen(input->contact) > 0) {
        param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(64)
        param[2].buffer = input->contact;
        param[2].buffer_length = strlen(input->contact);
    } else {
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

    if (!exec_sp(&stmt, param, "call crea_ordine(?, ?, ?, ?, ?, ?)"))
        return 0;

    param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_id INT
    param[0].buffer = &id;
    param[0].buffer_length = sizeof(id);

    if (!fetch_res_sp(stmt, param))
        return 0;

    mysql_stmt_close(stmt);
    return id;
}

static void open_order(void)
{
    struct create_order_sp_params params;
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;
    char spec_name[BUFFSIZE_M];

    memset(&params, 0, sizeof(params));
    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(spec_name, 0, sizeof(spec_name));

    init_screen(false);

    printf("*** Open a new order ***\n");
    printf("Customer code....................................................: %s\n", curr_customer.code);
    
    printf("Insert shipping address (default residential address)............: ");
    get_input(BUFFSIZE_M, params.shipping_address, false, false);

    printf("Insert contact (default favourite one)...........................: ");
    get_input(BUFFSIZE_XL, params.contact, false, false);

    if (ask_for_tips("Do you wanna search species by name to find the right code", 0)) {
        printf("\nInsert the name to filter on (default all).......................: ");   
        get_input(BUFFSIZE_M, spec_name, false, false);
        
        if (!attempt_search_species(false, spec_name))
            printf("Operation failed\n");

        putchar('\n');        
    }

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

    if (!exec_sp(&stmt, param, "call aggiungi_specie_ad_ordine_esistente(?, ?, ?, ?)"))
        return false;

    mysql_stmt_close(stmt);
    return true; 
}

static int attempt_to_modify_order(unsigned int order_id, unsigned int species_code, unsigned int quantity)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND param[5];
    int affected_rows;

    memset(param, 0, sizeof(param));
    
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

    if (!exec_sp(&stmt, param, "call modifica_ordine(?, ?, ?, ?, ?)"))
        return -1;

    param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_aggiornamento_eff INT
    param[0].buffer = &affected_rows;
    param[0].buffer_length = sizeof(affected_rows);

    if (!fetch_res_sp(stmt, param))
        return -1;

    mysql_stmt_close(stmt);
    return affected_rows;  
}

static void exec_op_on_order(bool is_add)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char spec_name[BUFFSIZE_M];
    unsigned int order_id;
    unsigned int species_code;
    unsigned int quantity;
    int ret;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(spec_name, 0, sizeof(spec_name));

    init_screen(false);

    if (is_add)
        printf("*** Add a species to already opened order ***\n");
    else
        printf("*** Change the number of plants belonging to a species in an order ***\n");


    printf("Customer code....................................................%s: %s\n", 
           (is_add) ? "" : "....", curr_customer.code);

    if (ask_for_tips("Do you wanna see a report of your open orders", (is_add) ? 13 : 17)) {
        if (!attempt_report_orders_short(true))
            printf("Operation failed\n");

        putchar('\n');    
    }  

    printf("Insert order id..................................................%s: ", (is_add) ? "" : "....");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    if (is_add) {
        if (ask_for_tips("Do you wanna search species by name to find the right code", 0)) {
            printf("\nInsert the name to filter on (default all).......................: ");   
            get_input(BUFFSIZE_M, spec_name, false, false);

            if (!attempt_search_species(false, spec_name))
                printf("Operation failed\n");

            putchar('\n');        
        }
    } else {
        if (ask_for_tips("Do you wanna see a list of species belonging to selected order", 0)) {
            if (!attempt_search_species_belonging_to_order(order_id))
                printf("Operation failed\n");

            putchar('\n');   
        }
    }

    printf("Insert species code..............................................%s: ", (is_add) ? "" : "....");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    printf("Insert relative quantity.........................................%s: ", (is_add) ? "" : "....");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    quantity = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (is_add) {
        if (attempt_to_add_spec_to_order(order_id, species_code, quantity))
            printf("Species %u succesfully added to your order (ID %010u)\n", species_code, order_id);
        else
            printf("Operation failed\n");
    } else {
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

    if (!exec_sp(&stmt, param, "call rimuovi_specie_da_ordine(?, ?, ?, ?, ?)"))
        return -1;

    param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_ordine_eliminato_si_no INT
    param[0].buffer = order_status;
    param[0].buffer_length = sizeof(*order_status);

    param[1].buffer_type = MYSQL_TYPE_LONG; // OUT var_eliminazione_eff INT
    param[1].buffer = &affected_rows;
    param[1].buffer_length = sizeof(affected_rows);

    if (!fetch_res_sp(stmt, param))
        return -1;
    
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
    printf("Customer code........................................................: %s\n", 
           curr_customer.code);

    if (ask_for_tips("Do you wanna see a report of your open orders", 17)) {
        if (!attempt_report_orders_short(true))
            printf("Operation failed\n");

        putchar('\n');    
    }  

    printf("Insert order id......................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    if (ask_for_tips("Do you wanna see a list of species belonging to selected order", 0)) {
        if (!attempt_search_species_belonging_to_order(order_id))
            printf("Operation failed\n");

        putchar('\n');   
    }  

    printf("Insert species code..................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    species_code = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    ret = attempt_to_remove_spec_from_order(order_id, species_code, &order_status); 

    if (ret > 0) {
        printf("Species %u succesfully deleted from your order (ID %010u)\n", species_code, order_id);

        if (order_status == 1)
            printf("Order (ID %010u) has been deleted (there were no more plants belonging to it)\n", order_id);
    } else if (ret == 0) {
        printf("Nothing has changed (species %u not in order [ID %010u])\n", species_code, order_id);
    } else {
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

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[1].buffer = &order_id;
    param[1].buffer_length = sizeof(order_id);

    if (!exec_sp(&stmt, param, "call finalizza_ordine(?, ?)"))
        return false;

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

    if (ask_for_tips("Do you wanna see a report of your open orders", 0)) {
        if (!attempt_report_orders_short(true))
            printf("Operation failed\n");

        putchar('\n');    
    } 

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

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    if (strlen(addr) != 0) {
        param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_indirizzo VARCHAR(64)
        param[1].buffer = addr;
        param[1].buffer_length = strlen(addr);
    } else {
        param[1].buffer_type = MYSQL_TYPE_NULL; // IN var_indirizzo VARCHAR(64)
        param[1].buffer = NULL;
        param[1].buffer_length = 0;
    }

    snprintf(sp_str, BUFFSIZE_L, "call modifica_%s(?, ?)", (is_res) ? "residenza" : "fatturazione");
    if (!exec_sp(&stmt, param, sp_str))
        return false;

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

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    snprintf(sp_str, BUFFSIZE_L, "call visualizza_contatti_%s(?)", (is_customer) ? "cliente" : "referente");
    if (!exec_sp(&stmt, param, sp_str))
        return false;

    if (!dump_result_set(stmt, "\nContact list:", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }

    mysql_stmt_close(stmt);
    return true;       
}

static bool attempt_to_modify_contact_list(char *contact, bool is_customer, bool to_delete)
{
    char sp_str[BUFFSIZE_L];
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[2];

    memset(sp_str, 0, sizeof(sp_str));
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(256)
    param[1].buffer = contact;
    param[1].buffer_length = strlen(contact);

    if (to_delete)  {
        if (is_customer)
            snprintf(sp_str, BUFFSIZE_L, "call rimuovi_contatto_cliente(?, ?)");
        else   
            snprintf(sp_str, BUFFSIZE_L, "call rimuovi_contatto_referente(?, ?)");
    } else {
        if (is_customer)
            snprintf(sp_str, BUFFSIZE_L, "call modifica_contatto_preferito_cliente(?, ?)");
        else   
            snprintf(sp_str, BUFFSIZE_L, "call modifica_contatto_preferito_referente(?, ?)");
    }

    if (!exec_sp(&stmt, param, sp_str))
        return false;

    mysql_stmt_close(stmt);
    return true;  
}

static void modify_contact_list(bool is_customer, bool to_delete)
{
    char contact[BUFFSIZE_XL];
    char message[BUFFSIZE_L];

    memset(contact, 0, sizeof(contact));
    memset(message, 0, sizeof(message));

    init_screen(false);

    if (to_delete)
        printf("*** Remove a contact from your %slist ***\n", (is_customer) ? "" : "referent");
    else
        printf("*** Change %s favourite contact %s***\n", (is_customer) ? "your" : "", (is_customer) ? "" : "of your referent");

    printf("%s code....................................%s: %s\n", 
           (is_customer) ? "Customer" : "Referent", 
           (is_customer) ? "": ".........",
           curr_customer.code);    

    snprintf(message, BUFFSIZE_L, "Do you wanna see a report of your %scontacts", 
             (is_customer) ? "" : "referent");

    if (ask_for_tips(message, 0)) {
        if (!attempt_show_contact_list(is_customer))
            printf("Operation failed\n");

        putchar('\n');
    } 

    printf("Insert contact...................................%s: ", (is_customer) ? "": ".........");
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

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_contatto VARCHAR(256)
    param[1].buffer = contact;
    param[1].buffer_length = strlen(contact);

    param[2].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_tipo CHAR(16)
    param[2].buffer = type;
    param[2].buffer_length = strlen(type);

    snprintf(sp_str, BUFFSIZE_L, "call aggiungi_contatto_%s(?, ?, ?)", 
             (is_customer) ? "cliente" : "referente");

    if (!exec_sp(&stmt, param, sp_str))
        return false;

    if (!dump_result_set(stmt, "\nUpdated list:", 0)) {
        CLOSE_AND_RETURN(false, stmt);
    }

    mysql_stmt_close(stmt);
    return true;  
}

static void add_contact(bool is_customer, bool show_prompt)
{
    char contact[BUFFSIZE_XL];
    char type[BUFFSIZE_XS];
    char message[BUFFSIZE_L];
    char choice;

    memset(contact, 0, sizeof(contact));
    memset(type, 0, sizeof(type));
    memset(message, 0, sizeof(message));

    init_screen(false);

    printf("*** Add a contact to your %slist ***\n", (is_customer) ? "" : "referent ");
    printf("%s code....................................%s: %s\n", 
           (is_customer) ? "Customer" : "Referent", 
           (is_customer) ? "": ".........",
           curr_customer.code);

    snprintf(message, BUFFSIZE_L, "Do you wanna see a report of your %scontacts", (is_customer) ? "" : "referent ");

    if (ask_for_tips(message, 0)) {
        if (!attempt_show_contact_list(is_customer))
            printf("Operation failed\n");

        putchar('\n');
    } 
    printf("Insert new contact...............................%s: ", (is_customer) ? "": ".........");
    get_input(BUFFSIZE_XL, contact, false, true);

    snprintf(message, BUFFSIZE_L, "Select type [m]obile, [l]andline, [e]mail%s", 
             (is_customer) ? "": ".........");

    choice = multi_choice(message, "mle", 3);

    switch (choice) {
    case 'm': snprintf(type, BUFFSIZE_XS, "cellulare"); break;
    case 'l': snprintf(type, BUFFSIZE_XS, "telefono"); break;
    case 'e': snprintf(type, BUFFSIZE_XS, "email"); break;
    default:
        fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
        abort();
    }

    putchar('\n');

    if (attempt_add_contact(contact, type, is_customer)) {
        choice = multi_choice("\nDo you wanna set this as favourite contact?", "yn", 2);

        if (choice == 'y') {
            if (attempt_to_modify_contact_list(contact, is_customer, false))
                printf("Contact succesfully changed\n");
            else
                printf("Operation failed\n");
        } else {
            return;
        }
    } else {
        printf("Operation failed\n");
    } 

    if (show_prompt) {
        printf("Press enter key to get back to menu ...\n");
        getchar();
    }   
}

static bool attempt_report_order(unsigned int order_id)
{
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[2];
    struct order_info order;
    int i = 0;
    int status;

    int flags[] = {
        LEADING_ZERO_BITMASK_IDX_0,
        LEADING_ZERO_BITMASK_IDX_0,
        0
    };

    char *messages[] = {
        "\nOrder info:",
        "\n\nInvolved species:",
        "\n\nEconiomic details:"
    };

    memset(&order, 0, sizeof(order));
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_cliente	VARCHAR(16)
    param[0].buffer = curr_customer.code;
    param[0].buffer_length = strlen(curr_customer.code);

    param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[1].buffer = &order_id;
    param[1].buffer_length = sizeof(order_id);

    if(!exec_sp(&stmt, param, "call report_ordine(?, ?)")) 
        return false;

    do {
        if(conn->server_status & SERVER_PS_OUT_PARAMS) 
            goto next;

        if (!dump_result_set(stmt, messages[i], flags[i])) {
            CLOSE_AND_RETURN(false, stmt);
        }

        ++i;
next:
        status = mysql_stmt_next_result(stmt);
        if (status > 0) {
            print_stmt_error(stmt, "Unexpected condition");
            CLOSE_AND_RETURN(false, stmt);        
        }

    } while (status == 0);

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

    if (ask_for_tips("Do you wanna see a report of your orders", 0)) {
        if (!attempt_report_orders_short(false))
            printf("Operation failed\n");

        putchar('\n');    
    } 
    
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

    while (true) {
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

        switch (choice) {
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

    while (true) {
        init_screen(false);

        printf("*** [PROFILE MANAGEMENT] What do you wanna do? ***\n\n");
        printf("1) Update your residential address\n");
        printf("2) Update your billing address\n");
        printf("3) Add a contact to your list\n");
        printf("4) Remove contact from your list\n");
        printf("5) Change your favourite contact\n");
        printf("q) Back to main menu\n");

        choice = multi_choice("Pick an option", "12345q", 6);

        switch (choice) {
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

    while (true) {
        init_screen(false);

        printf("*** [REFERENT MANAGEMENT] What do you wanna do? ***\n\n");
        printf("1) Add a contact to your referent list\n");
        printf("2) Remove contact from your referent list\n");
        printf("3) Change favourite contact of your referent\n");
        printf("q) Back to main menu\n");

        choice = multi_choice("Pick an option", "1234", 4);

        switch (choice){
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
    if (curr_customer.is_private) {
        while (true) {
            init_screen(true);
            printf("Welcome %s (%s)\n\n", curr_customer.username, curr_customer.code);
            printf("*** What do you wanna do? ***\n\n");
            printf("1) Orders management\n");
            printf("2) Profile management\n");
            printf("p) Change password\n");
            printf("q) Quit\n");

            choice = multi_choice("Pick an option", "12pq", 4);

            switch (choice) {
            case '1': order_management_menu(); break;
            case '2': profile_management_menu(); break;
            case 'p': change_password(curr_customer.username); break;
            case 'q': printf("Bye bye!\n\n\n"); return;
            default:
                    fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                    abort();
            }
        }
    } else {
        while (true) {
            init_screen(true);
            printf("Welcome %s (%s)\n\n", curr_customer.username, curr_customer.code);
            printf("*** What do you wanna do? ***\n\n");
            printf("1) Orders management\n");
            printf("2) Profile management\n");
            printf("3) Referent management\n");
            printf("4) Change password\n");
            printf("q) Quit\n");

            choice = multi_choice("Pick an option", "1234q", 5);

            switch (choice) {
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
    struct configuration cnf;
    char choice;

    memset(path, 0, sizeof(path));   
    memset(&curr_customer, 0, sizeof(curr_customer));
    memset(&cnf, 0, sizeof(cnf));

    memcpy(curr_customer.code, customer_code, BUFFSIZE_XS);
    memcpy(curr_customer.username, username, BUFFSIZE_L);
    curr_customer.is_private = is_private;

    snprintf(path, BUFFSIZE_M, "config/%s.user", (is_private) ? "pcs" : "rcs");

    if (parse_config(path, &cnf, "=")) {
        fprintf(stderr, "Invalid configuration file selected (%s)\n", (is_private) ? "PCS" : "RCS");
        exit(EXIT_FAILURE);
    }

    if(mysql_change_user(conn, cnf.username, cnf.password, cnf.database)) {
        fprintf(stderr, "Unable to switch privileges\n");
        exit(EXIT_FAILURE);
    }

    if (first_access) {
        choice = multi_choice("Do you wanna insert a contact......?", "yn", 2);
        
        if (choice == 'y') {
            add_contact(true, false);
            sleep(1.5);
        }   
    }

    main_menu();
}
