#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <mysql.h>
#include <stdlib.h>
#include "defines.h"


static char curr_user[BUFFSIZE_L];


static bool attempt_add_employee_account(char *username, char *password, char *role)
{
    MYSQL_STMT *stmt;	
    MYSQL_BIND param[3];

    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_username VARCHAR(128)
    param[0].buffer = username;
    param[0].buffer_length = strlen(username);

    param[1].buffer_type = MYSQL_TYPE_VAR_STRING; // IN var_username VARCHAR(128)
    param[1].buffer = password;
    param[1].buffer_length = strlen(password);

    param[2].buffer_type = MYSQL_TYPE_STRING; // IN var_username VARCHAR(128)
    param[2].buffer = role;
    param[2].buffer_length = strlen(role);

    if (!exec_sp(&stmt, param, "call crea_utenza_dipendente(?, ?, ?)"))
        return false;

    mysql_stmt_close(stmt);
    return true;    
}

static void add_employee_account(void)
{
    char username[BUFFSIZE_L];
    char password[BUFFSIZE_L];
    char password_check[BUFFSIZE_L];
    char role[4];
    char choice;

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(password_check, 0, sizeof(password_check));

    init_screen(false);

    printf("Insert username: ");
    get_input(BUFFSIZE_L, username, false, true);

retype_pass:
    printf("Insert password: ");
    get_input(BUFFSIZE_L, password, true, true);

    printf("Retype password: ");
    get_input(BUFFSIZE_L, password_check, true, true);

    if (strcmp(password, password_check) != 0) {
        printf("Mismatch password, please retry!\n");
        goto retype_pass;
    }

    printf("\nWhat will be her/his role in the company?\n");
    printf("1) Warehouse clerk\n");
    printf("2) Order processor\n");
    printf("3) Manager\n");

    choice = multi_choice("Pick an option", "123", 3);

    switch (choice) {
    case '1': snprintf(role, 4, "ADM"); break;
    case '2': snprintf(role, 4, "OPP"); break;
    case '3': snprintf(role, 4, "MNG"); break;
    default:
        fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
        abort();
    }

    putchar('\n');

    if (attempt_add_employee_account(username, password, role))
        printf("Employee account for %s [%s] was succesfylly created\n", username, role);
    else
        printf("Operation failed\n");

    printf("Press enter key to get back to menu ...\n");
    getchar();
}

void run_as_chief_of_staff(char *username)
{
    struct configuration cnf;
    char choice;

    memset(&cnf, 0, sizeof(cnf));
    memset(curr_user, 0, sizeof(curr_user));

    strncpy(curr_user, username, BUFFSIZE_L);

    if (parse_config("config/cos.user", &cnf, "=")) {
        fprintf(stderr, "Invalid configuration file selected (COS)\n");
        exit(EXIT_FAILURE);
    }

    if(mysql_change_user(conn, cnf.username, cnf.password, cnf.database)) {
        fprintf(stderr, "Unable to switch privileges\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        init_screen(true);
        printf("Welcome %s\n\n", curr_user);
        printf("*** What do you wanna do? ***\n\n");
        printf("a) Add an employee account\n");
        printf("p) Change password\n");
        printf("q) Quit\n");

        choice = multi_choice("Pick an option", "apq", 3);

        switch (choice) {
            case 'a': add_employee_account(); break;
            case 'p': change_password(curr_user); break;
            case 'q': printf("Bye bye!\n\n\n"); return;
            default:
                fprintf(stderr, "Invalid condition at %s:%d\n", __FILE__, __LINE__);
                abort();
        }
    }
}
