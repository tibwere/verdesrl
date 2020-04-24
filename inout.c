#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

static volatile sig_atomic_t signo;

static void handler(int s) { signo = s; }

size_t get_input(unsigned int length, char *string, bool hide)
{
    char c;
    unsigned int i;

	struct sigaction sa, savealrm, saveint, savehup, savequit, saveterm, savetstp, savettin, savettou;
	struct termios term, oterm;

    if (hide)
    {
        (void) fflush(stdout);
        (void) sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_INTERRUPT;
        sa.sa_handler = handler;

		(void) sigaction(SIGALRM, &sa, &savealrm);
		(void) sigaction(SIGINT, &sa, &saveint);
		(void) sigaction(SIGHUP, &sa, &savehup);
		(void) sigaction(SIGQUIT, &sa, &savequit);
		(void) sigaction(SIGTERM, &sa, &saveterm);
		(void) sigaction(SIGTSTP, &sa, &savetstp);
		(void) sigaction(SIGTTIN, &sa, &savettin);
		(void) sigaction(SIGTTOU, &sa, &savettou);

		if (tcgetattr(fileno(stdin), &oterm) == 0) 
        {
			(void) memcpy(&term, &oterm, sizeof(struct termios));
			term.c_lflag &= ~(ECHO|ECHONL);
			(void) tcsetattr(fileno(stdin), TCSAFLUSH, &term);
		} 
        else 
        {
			(void) memset(&term, 0, sizeof(struct termios));
			(void) memset(&oterm, 0, sizeof(struct termios));
		}
    }

    for (i = 0; i < length; ++i) 
    {
        (void) fread(&c, sizeof(char), 1, stdin);
        if (c == '\n') 
        {
            string[i] = '\0';
            break;
        } 
        else
            string[i] = c;    
    }

    if (i == length - 1)
        string[i] = '\0';

    if (strlen(string) >= length) 
    {	
        do {
            c = getchar();
        } while (c != '\n');
    }

    if (hide) 
    {
		(void) write(fileno(stdout), "\n", 1);
		(void) tcsetattr(fileno(stdin), TCSAFLUSH, &oterm);
		(void) sigaction(SIGALRM, &savealrm, NULL);
		(void) sigaction(SIGINT, &saveint, NULL);
		(void) sigaction(SIGHUP, &savehup, NULL);
		(void) sigaction(SIGQUIT, &savequit, NULL);
		(void) sigaction(SIGTERM, &saveterm, NULL);
		(void) sigaction(SIGTSTP, &savetstp, NULL);
		(void) sigaction(SIGTTIN, &savettin, NULL);
		(void) sigaction(SIGTTOU, &savettou, NULL);

		if (signo)
			(void) raise(signo);
	}
	
	return i;
}

char multi_choice(const char *question, const char *choices, int no_choices)
{
	char *choices_str = malloc(2 * no_choices * sizeof(char));
	int i, j = 0;

	for (i = 0; i < no_choices; ++i) 
    {
		choices_str[j++] = choices[i];
		choices_str[j++] = '/';
	}
	choices_str[j-1] = '\0';

	while(true) 
    {
		printf("%s [%s]: ", question, choices_str);

		char c;
		get_input(1, &c, false);

		for (i = 0; i < no_choices; i++) 
        {
			if(c == choices[i])
            {
                free(choices_str);
				return c;
            }
		}

		printf("Sorry not compliant input, please retry!\n");
	}
}