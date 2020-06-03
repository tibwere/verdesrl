#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include "defines.h"

struct create_pack_sp_param {
    unsigned int order_id;
    unsigned int pack_number;
    unsigned int species_code;
    unsigned int quantity;
};


static char curr_user[BUFFSIZE_L];


static bool attempt_search_species_belonging_to_order(unsigned int order_id)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND param[1];

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[0].buffer = &order_id;
    param[0].buffer_length = sizeof(order_id);

    if(!exec_sp(&stmt, param, "call visualizza_piante_rimanenti_da_impacchettare(?)")) 
        return false;
    
    if (!dump_result_set(stmt, "Species belonging to selected order:", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }
    
    mysql_stmt_close(stmt);
    return true;    
}

static bool attempt_show_status(unsigned int order_id)
{
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[1];

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[0].buffer = &(order_id);
    param[0].buffer_length = sizeof(order_id);

    if(!exec_sp(&stmt, param, "call visualizza_stato_ordine(?)")) 
        return false;

    if (!dump_result_set(stmt, "Order info:", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }
    
	mysql_stmt_close(stmt);
	return true;
}

static bool attempt_exec_op_on_pack(struct create_pack_sp_param *args, bool is_create)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND param[4];
    char sp_str[BUFFSIZE_L];

    memset(param, 0, sizeof(param));
    memset(sp_str, 0, sizeof(sp_str));

    snprintf(sp_str, BUFFSIZE_L, "call %s_pacco(?, ?, ?, ?)", (is_create) ? "crea" : "aggiungi_specie_a");

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[0].buffer = &(args->order_id);
    param[0].buffer_length = sizeof(args->order_id);

    param[1].buffer_type = MYSQL_TYPE_LONG; // IN var_specie INT
    param[1].buffer = &(args->species_code);
    param[1].buffer_length = sizeof(args->species_code);  

    param[2].buffer_type = MYSQL_TYPE_LONG; // IN var_quantita INT
    param[2].buffer = &(args->quantity);
    param[2].buffer_length = sizeof(args->quantity);  

    param[3].buffer_type = MYSQL_TYPE_LONG; // OUT var_numero INT
    param[3].buffer = &(args->pack_number);
    param[3].buffer_length = sizeof(args->pack_number);

    if(!exec_sp(&stmt, param, sp_str)) 
        return false;

    if (is_create) {
        param[0].buffer_type = MYSQL_TYPE_LONG; // OUT var_numero INT
        param[0].buffer = &(args->pack_number);
        param[0].buffer_length = sizeof(args->pack_number);
        
        if (!fetch_res_sp(stmt, param))
            return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

static bool attempt_report_packs(unsigned int order_id)
{
    MYSQL_STMT *stmt;
    MYSQL_BIND param[1];

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG; // IN var_ordine INT
    param[0].buffer = &order_id;
    param[0].buffer_length = sizeof(order_id);

    if(!exec_sp(&stmt, param, "call report_pacchi(?)")) 
        return false;

    if (!dump_result_set(stmt, "Processing details", LEADING_ZERO_BITMASK_IDX_0)) {
        CLOSE_AND_RETURN(false, stmt);
    }

    mysql_stmt_close(stmt);
    return true; 
}

static void report_packs(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    unsigned int order_id;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));

    init_screen(false);

    printf("*** View details about order processing ***\n");

    printf("Insert order ID......................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    order_id = strtol(buffer_for_integer, NULL, 10);

    putchar('\n');

    if (!attempt_show_status(order_id))
        printf("Operation failed\n");

    putchar('\n');

    if (!attempt_search_species_belonging_to_order(order_id))
        printf("Operation failed\n");
        
    putchar('\n');

    if (!attempt_report_packs(order_id))
        printf("Operation failed\n");

    putchar('\n');

    printf("Press enter key to get back to menu ...\n");
    getchar();    
}

static void create_pack(void)
{
    char buffer_for_integer[BUFFSIZE_XS];
    char spec_name[BUFFSIZE_M];
    struct create_pack_sp_param args;

    memset(buffer_for_integer, 0, sizeof(buffer_for_integer));
    memset(&args, 0, sizeof(args));
    memset(spec_name, 0, sizeof(spec_name));

    init_screen(false);

    printf("*** Create new pack ***\n");

    printf("Insert order ID......................................................: ");
    get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
    args.order_id = strtol(buffer_for_integer, NULL, 10);

    if (!attempt_show_status(args.order_id))
        printf("Operation failed\n");

    if (ask_for_tips("Do you wanna see a list of species belonging to selected order", 0)) {
        if (!attempt_search_species_belonging_to_order(args.order_id))
            printf("Operation failed\n");

        putchar('\n');   
    }
    printf("For each species insert code and relative quantity.\nTo exit the loop enter 0 as a species code\n\n");

    while (true) {
        printf("\nInsert species code..................................................: ");
        get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
        args.species_code = strtol(buffer_for_integer, NULL, 10);

        if (args.species_code == 0) {
            if (args.pack_number == 0) {
                printf("A pack cannot be empty!\nOperation aborted\n");
                goto exit_with_a_failure;
            } else {
                break;
            }
        }

        printf("Insert quantity......................................................: ");
        get_input(BUFFSIZE_XS, buffer_for_integer, false, true);
        args.quantity = strtol(buffer_for_integer, NULL, 10);
        
        if (args.pack_number == 0) {
            if (!attempt_exec_op_on_pack(&args, true)) {
                printf("Operation failed\n");
                goto exit_with_a_failure;
            }
        } else {
            if (!attempt_exec_op_on_pack(&args, false)) {
                printf("Operation failed\n");
                goto exit_with_a_failure;
            }
        }
    }

    printf("New pack (no. %u) has been created for order %010u\n", args.pack_number, args.order_id);

exit_with_a_failure:
    putchar('\n');

    printf("Press enter key to get back to menu ...\n");
    getchar();    
}

void run_as_order_processor(char *username)
{
    struct configuration cnf;
    char choice;

    memset(&cnf, 0, sizeof(cnf));
    memset(curr_user, 0, sizeof(curr_user));

    strncpy(curr_user, username, BUFFSIZE_L);

    if (parse_config("config/opc.user", &cnf, "=")) {
        fprintf(stderr, "Invalid configuration file selected (OPP)\n");
        exit(EXIT_FAILURE);
    }

    if (mysql_change_user(conn, cnf.username, cnf.password, cnf.database)) {
        fprintf(stderr, "Unable to switch privileges\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        init_screen(true);
        printf("Welcome %s\n\n", curr_user);
        printf("*** What do you wanna do? ***\n\n");
        printf("1) Create new pack\n");
        printf("2) View details about order processing\n");
        printf("p) Change password\n");
        printf("q) Quit\n");

        choice = multi_choice("Pick an option", "12pq", 4);

        switch (choice) {
        case '1': create_pack(); break;
        case '2': report_packs(); break;
        case 'p': change_password(curr_user); break;
        case 'q': printf("Bye bye!\n\n\n"); return;
        default:
            fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
            abort();
        }
    }
}
