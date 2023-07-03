#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "cmd.h"
#include "file.h"
#include "parse.h"

void print_relative_path(EV *ev)
{
    int i;
    for (i = 0; ev->HOME[i] != '\0'; i++)
    {
        if (ev->PWD[i] == '\0')
        {
            printf("%s", ev->PWD);
            return;
        }
    }

    printf("~%s", ev->PWD + i);
}

int main(int argc, char **argv)
{

    rl_outstream = stderr;

    printf("\n");
    printf("--------------------------------------\n");
    printf("\n%s", GRN);
    printf(" $$$$$  $              ////      \\ /  \n");
    printf(" $      $            ////       --X--   \n");
    printf(" $$$$$  $          ////          / \\  \n");
    printf("     $  $        ////                 \n");
    printf(" $$$$$  $$$$$  //// $     $$$$$ $   $ \n");
    printf("             ////  $$$    $     $   $ \n");
    printf("           ////   $   $   $$$$$ $$$$$ \n");
    printf("         ////    $$$$$$$      $ $   $ \n");
    printf("       ////     $       $ $$$$$ $   $ \n");
    printf("\n%s", KNRM);
    printf("--------------------------------------\n");
    printf("\n");

    struct sigaction sa;
    sa.sa_handler=SIG_IGN; 
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    EV *ev = malloc(sizeof(EV));
    ev->HOME = getenv("HOME");
    ev->PWD = malloc(MAX_PATH_LEN);
    ev->MEM_DIR = malloc(MAX_PATH_LEN);
    ev->STATUS_MSG = malloc(MAX_PATH_LEN);
    ev->STATUS_CODE = 0;
    ev->SIG_GEST = &sa;


    char root[MAX_PATH_LEN];
    root[0] = '.';
    root[1] = '\0';
    construct_physical_path(root, ev->PWD);

    while (1)
    {

        char **tab = (char **)malloc(MAX_ARGS_NUMBER * sizeof(char *));
        for (int i = 0; i < MAX_ARGS_NUMBER; i++)
        {
            tab[i] = (char *)malloc(MAX_ARGS_STRLEN * sizeof(char));
            tab[i][0] = '\0';
        }

        char *prompt = (char *)malloc(PROMPT_LENGHT * sizeof(char));

        /* 
        Notre prompt de reve
        printf("%s┌──(%s", KGRN, KBLU);
        print_relative_path(ev);
        printf("%s)\n", KGRN); */

        extract(prompt, ev->PWD, ev->STATUS_CODE);

        char *line = readline(prompt);

        if (line == NULL)
        {
            free(line);
            terminate(ev, tab);
        }

        add_history(line);

        int argc;
        parse(line, ' ', tab, &argc, ev);
        parsePipe(tab, argc, ev);

        for (int i = 0; i < MAX_ARGS_NUMBER; i++)
        {
            free(tab[i]);
        }
        free(tab);
        free(line);
        free(prompt);
    }
}
