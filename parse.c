#include "ev.h"
#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "cmd.h"
#include <sys/wait.h>
#include <signal.h>
#include "file.h"


void parse(char *line, char sep, char **tab, int *tabLength, EV *ev);
void extract(char *line, char *path, unsigned int number);
int isJoker(char *line);
int cmdSearch(char **tab, char *string, int *nbArgs, EV *ev);
int gestionDesJokers(char **tab, char *path, char *prefix, char *string, int *nbArgs, int verif);
int gestionDoubleStar(char **tab, char *completePath, char *prefix, char *lookingFor, int *nbArgs, int inos[], int *nbRep);
int executecmd(char **cmd, int argc, EV *ev, int input, int output, int error);

// Fonction qui parse la ligne de commande en fonction du séparateur *
int isJoker(char *line)
{
    int i = 0;
    int nbasterisk = 0;
    while (line[i] != '\0')
    {
        if (line[i] == '*')
        {
            nbasterisk++;
        }
        i++;
    }
    return nbasterisk;
}

void parse(char *line, char sep, char **tab, int *tabLength, EV *ev)
{

    int nbArgs = 0;
    int wordLength = 0;
    int ignore = 1;
    int guill = 0;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == sep && !ignore && !guill)
        {
            tab[nbArgs][wordLength] = '\0';

            if (nbArgs == 0)
            {
                char cpy[strlen(tab[0]) + 1];
                strcpy(cpy, tab[0]);
                int res = cmdSearch(tab, cpy, &nbArgs, ev);
                if (res != 0)
                {
                    ev->STATUS_CODE = res;
                    ev->STATUS_MSG = "Command not found";
                    tab[0][0] = '\0';
                    *tabLength = 0;
                    return;
                }
            }
            else
            {
                char prefix[MAX_ARGS_STRLEN];
                prefix[0] = '\0';
                char path[MAX_PATH_LEN];
                path[0] = '\0';
                if (tab[nbArgs][0] != '/')
                    strcpy(path, ev->PWD);

                char cpy[strlen(tab[nbArgs]) + 1];
                strcpy(cpy, tab[nbArgs]);

                gestionDesJokers(tab, path, prefix, cpy, &nbArgs, 0);
            }
            wordLength = 0;

            ignore = 1;
        }
        else if(line[i] != sep)
        {
            if(line[i]!='\"') tab[nbArgs][wordLength++] = line[i];
            else if(guill) guill=0;
            else guill = 1;
            ignore = 0;
        }
    }

    tab[nbArgs][wordLength] = '\0';

    if (nbArgs == 0)
    {
        char cpy[strlen(tab[0]) + 1];
        strcpy(cpy, tab[0]);
        int res = cmdSearch(tab, cpy, &nbArgs, ev);

        if (res != 0)
        {
            ev->STATUS_CODE = res;
            ev->STATUS_MSG = "Command not found";
            tab[0][0] = '\0';
            *tabLength = 0;
            return;
        }
    }
    else
    {
        char prefix[MAX_ARGS_STRLEN];
        prefix[0] = '\0';
        char path[MAX_PATH_LEN];
        path[0] = '\0';
        if (tab[nbArgs][0] != '/')
            strcpy(path, ev->PWD);

        char cpy[strlen(tab[nbArgs]) + 1];
        strcpy(cpy, tab[nbArgs]);

        gestionDesJokers(tab, path, prefix, cpy, &nbArgs, 0);
    }
    nbArgs--;

    if (ignore)
        nbArgs--;

    *tabLength = nbArgs;
}

void extract(char *line, char *path, unsigned int number)
{
    char nb[MAX_LENGTH_LINE];

    char *color = number == 0 ? KGRN : KRED;
    if(number == 255) sprintf(nb,  "%s[%s]%s", color, "SIG", KCYN);
    else sprintf(nb, "%s[%u]%s", color, number, KCYN); // 2x5 = COLOR_LENGTH
    int path_len = strlen(path);

    int suplen = strlen(nb) - COLOR_LENGTH;

    if (suplen + path_len + 2 <= MAX_LENGTH_LINE)
    {
        strcpy(line, nb);
        strcat(line, path);
        strcat(line, "\033[00m$ ");
    }
    else
    {
        int end = MAX_LENGTH_LINE + COLOR_LENGTH - 2;
        memcpy(line, nb, suplen + COLOR_LENGTH);
        memcpy(line + suplen + COLOR_LENGTH, "...", 3);
        int pos = suplen + COLOR_LENGTH + 3;
        memcpy(line + pos, path + path_len - (end - pos), end - pos);
        memcpy(line + end, "\033[00m$ ", 8); // strlen("\033[00m$ ") = 7
    }
}

int cmdSearch(char **tab, char *string, int *nbArgs, EV *ev)
{
    int lenpatern = strlen(string) - 1;

    if (string[0] == '*')
    {
        DIR *fd = opendir(ev->PWD);
        struct dirent *entry;
        while ((entry = readdir(fd)))
        {
            int len = strlen(entry->d_name);
            if (len >= lenpatern && entry->d_name[0] != '.')
            {
                char comp[MAX_ARGS_STRLEN] = "\0";
                strcpy(comp, entry->d_name + (len - lenpatern));

                if (strcmp(comp, &string[1]) == 0)
                {
                    strcpy(tab[*nbArgs], "./");
                    strcpy(tab[*nbArgs], entry->d_name);
                    (*nbArgs)++;
                }
            }
        }
        closedir(fd);
    }
    else
    {
        char prefix[MAX_PATH_LEN];
        prefix[0] = '\0';
        char path[MAX_PATH_LEN];
        path[0] = '\0';
        if (tab[*nbArgs][0] != '/')
            strcpy(path, ev->PWD);
        gestionDesJokers(tab, path, prefix, string, nbArgs, 0);
    }

    if (*nbArgs > 1)
        return 127;

    return 0;
}

int gestionDesJokers(char **tab, char *path, char *prefix, char *string, int *nbArgs, int verif)
{
    int nbr = isJoker(string);
    char completePath[MAX_PATH_LEN] = "\0";
    strcpy(completePath, path);
    if (verif != 2)
        strcat(completePath, "/");

    // Cas de la double étoile
    if (string[0] == '*' && string[1] == '*')
    {
        char lookingFor[MAX_ARGS_STRLEN] = "\0";
        strcpy(lookingFor, &string[3]);
        char prefix[MAX_ARGS_STRLEN] = "\0";
	    int inos[MAX_ARGS_STRLEN];
	    int nbTab = 0;
        gestionDoubleStar(tab, completePath, prefix, lookingFor, nbArgs, inos, &nbTab);
        return 0;
    }

    // Cas où l'on a pas de joker
    if (!verif && !nbr)
    {
        (*nbArgs)++;
        return 0;
    }
    // Cas où l'on a plus de joker
    if (!nbr)
    {
        strcat(prefix, string);

        struct stat st;
        if (stat(prefix, &st) == -1)
            return 1;

        strcpy(tab[*nbArgs], prefix);
        (*nbArgs)++;
    }
    else
    {
        char pieceOfPath[MAX_ARGS_STRLEN]="\0";
        int i = 0;
        int base = strlen(prefix);

        // On cherche notre étoile dans la string et on stocke tout ce qui la précède dans prefix
        while (string[i] != '*')
        {
            prefix[base + i] = string[i];
            pieceOfPath[i] = string[i];
            i++;
        }
        pieceOfPath[i] = '\0';
        prefix[base + i] = '\0';
        i++;

        if(verif!=2) 
            strcat(completePath, prefix);
        else strcat(completePath, pieceOfPath);

        // Une fois trouvée, on doit prendre le pattern (ce qui la suit) afin d'identifier
        // quels fichiers/répertoires doivent être pris en compte
        char pattern[MAX_ARGS_STRLEN] = "\0";
        int lenpattern = 0;
        while (string[i] != '/' && string[i] != '\0')
        {
            pattern[lenpattern] = string[i];
            lenpattern++;
            i++;
        }
        pattern[lenpattern] = '\0';

        while (string[i] != '\0' && string[i + 1] == '/')
            i++;

        DIR *fd = opendir(completePath);
        if (fd == NULL)
            return 1;
        char peutEtreIci[MAX_ARGS_NUMBER][MAX_ARGS_STRLEN];
        int indicePeutEtre = 0;
        struct dirent *entry;
        char matching = string[i];
        while ((entry = readdir(fd)))
        {
            if (entry->d_name[0] == '.')
                continue;

            int lenentry = strlen(entry->d_name);
            if (lenentry >= lenpattern)
            {
                char comp[MAX_ARGS_STRLEN] = "\0";
                strcpy(comp, entry->d_name + (lenentry - lenpattern));
                if (strcmp(comp, pattern) == 0)
                {
                    // Cas où l'on cherche des répertoires
                    if (matching == '/')
                    {
                        strcpy(peutEtreIci[indicePeutEtre], entry->d_name);
                        indicePeutEtre += 1;
                    }
                    // Cas où on cherche des fichiers avec une certaine terminaison
                    else if (matching == '\0')
                    {
                        char newprefix[MAX_ARGS_STRLEN] = "\0";
                        strcpy(newprefix, prefix);
                        strcat(newprefix, entry->d_name);
                        strcpy(tab[*nbArgs], newprefix);
                        (*nbArgs)++;
                    }
                }
            }
        }

        closedir(fd);
        for (int j = 0; j < indicePeutEtre; j++)
        {
            char newprefix[MAX_ARGS_STRLEN] = "\0";
            strcpy(newprefix, prefix);
            strcat(newprefix, peutEtreIci[j]);

            if(verif == 2) 
            {
		        char newPath[MAX_ARGS_STRLEN] = "\0";
		        strcpy(newPath, path);
                strcat(newPath, pieceOfPath);
                strcat(newPath, peutEtreIci[j]);
                gestionDesJokers(tab, newPath, newprefix, &string[i], nbArgs, 2);
            }
            else gestionDesJokers(tab, path, newprefix, &string[i], nbArgs, 1);
        }
    }
    return 0;
}

// Fonctionnement similaire à l'algo pour l'étoile simple, on force juste la récursion sur toute l'arborescence
int gestionDoubleStar(char **tab, char *completePath, char *prefix, char *lookingFor, int *nbArgs, int inos[], int *nbRep)
{
    DIR *fd = opendir(completePath);
    if (fd == NULL)
        return 1;
    char peutEtreIci[MAX_ARGS_NUMBER][MAX_ARGS_STRLEN];
    int indicePeutEtre = 0;
    struct dirent *entry;
    while ((entry = readdir(fd)))
    {
	    int verif = 1;
        struct stat st;
        char newPath[MAX_ARGS_STRLEN] = "\0";
        strcpy(newPath, completePath);
        strcat(newPath, entry->d_name); 
        stat(newPath, &st);
        if (entry->d_name[0] == '.')
            continue;
        if (S_ISDIR(st.st_mode))
        {
            for(int i=0; i<*nbRep; i++)
            {
                if(inos[i]==st.st_ino)
                {
                    verif=0;
                    break;
                }
            }
                if(verif)
                {
                    strcpy(peutEtreIci[indicePeutEtre], entry->d_name);
                    indicePeutEtre += 1;
                    inos[*nbRep]=st.st_ino;
                    (*nbRep)++;
	            }
        }
    }
    closedir(fd);
    if(strcmp(lookingFor, "\0")!=0)
	    gestionDesJokers(tab, completePath, prefix, lookingFor, nbArgs, 2);

    for (int j = 0; j < indicePeutEtre; j++)
    {

        char newPath[MAX_ARGS_STRLEN] = "\0";
        strcpy(newPath, completePath);
        strcat(newPath, peutEtreIci[j]);
        strcat(newPath, "/");

        char newprefix[MAX_ARGS_STRLEN] = "\0";
        strcpy(newprefix, prefix);
        strcat(newprefix, peutEtreIci[j]);
	    strcat(newprefix, "/");

        if(strcmp(lookingFor, "\0")==0){
            strcpy(tab[*nbArgs], newprefix);
            (*nbArgs)++;
        }

        gestionDoubleStar(tab, newPath, newprefix, lookingFor, nbArgs, inos, nbRep);
    }
    return 0;
}

// Fonction qui va recuperer le tab du premier parse
int parseRedirection(char **tab, int argc, EV *ev, int in, int out)
{
    int index = argc;
    int err = STDERR_FILENO;
    int fd = -1;

    for (int i = argc; i >= 0; i--)
    {
        switch (tab[i][0])
        {
        case '<':
            if(i == index) return 2;

            fd = open(tab[i+1], O_RDONLY);
            if (fd == -1)
            {
                return 1;
            }
            in = fd;
            index = i - 1;

            break;

        case '>':
            fd = -1;
            switch (tab[i][1])
            {
            case '|':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                break;

            case '>':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_APPEND | O_WRONLY, 0666);
                break;
            case '\0':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_EXCL | O_WRONLY, 0666);
                break;
            default:
                break;
            }

            if (fd == -1)
                return 1;
            out = fd;
            index = i - 1;
            break;

        case '2':

            if (tab[i][1] != '>')
                break;

            fd = -1;
            
            switch (tab[i][2])
            {
            case '>':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_APPEND | O_WRONLY, 0666);
                break;

            case '\0':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_EXCL | O_WRONLY, 0666);
                break;
            case '|':
                if(i == index) return 2;
                fd = open(tab[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
                break;
            default:
                break;
            }
            if (fd == -1)
                return 1;
            err = fd;
            index = i - 1;
            break;
        default:
            break;
        }
    }

    return executecmd(&tab[0], index, ev, in, out, err);
}

int parsePipe(char **tab, int argc, EV *ev) {
    int index = 0;

    int input = STDIN_FILENO;

    for(int i = 0; i < argc; i++) {
        if(tab[i][0] == '|') {
            if(i == index) return 2;

            int tube[2];
            pipe(tube); 

            int res = parseRedirection(&tab[index], i - index - 1, ev, input, tube[1]);
            if(res != 0) return res; 
            input = tube[0];

            index = i + 1;
        }
    }

    return parseRedirection(&tab[index], argc - index, ev, input, STDOUT_FILENO);
}

void terminate(EV *ev, char **tab)
{

    int exitStatus = ev->STATUS_CODE;
    if (tab[1][0] != '\0')
    {
        if (tab[1][0] == '0')
            exitStatus = 0;
        else
        {
            int nb = atoi(tab[1]);
            if (nb != 0)
                exitStatus = nb;
        }
    }

    for (int i = 0; i < MAX_ARGS_NUMBER; i++)
        free(tab[i]);
    free(tab);

    // free(ev->HOME);
    free(ev->PWD);
    free(ev->MEM_DIR);
    free(ev->STATUS_MSG);
    // free(ev->ERROR_MSG);
    free(ev);

    exit(exitStatus);
}

int executecmd(char **cmd, int argc, EV *ev, int input, int output, int error)
{       
    if (!strcmp(cmd[0], "exit"))
    {
        terminate(ev, cmd);
    }
    else if (!strcmp(cmd[0], "pwd"))
    {
        ev->STATUS_CODE = pwd(argc, cmd, ev);
    }
    else if (!strcmp(cmd[0], "cd"))
    {
        ev->STATUS_CODE = cd(argc, cmd, ev);
    }

    else if (cmd[0][0] != '\0')
    {
        pid_t fils = fork();
        int returnfils = 0;
        switch (fils)
        {
        case -1:
            strcpy(ev->STATUS_MSG, "Error : fork failed");
            ev->STATUS_CODE = 127;
            break;
        case 0:
	        ev->SIG_GEST->sa_handler = SIG_DFL;
            sigaction(SIGTERM, ev->SIG_GEST, NULL);   
            sigaction(SIGINT, ev->SIG_GEST, NULL);

            if(input != STDIN_FILENO) dup2(input, STDIN_FILENO);
            if(output != STDOUT_FILENO) dup2(output, STDOUT_FILENO);
            if(error != STDERR_FILENO) dup2(error, STDERR_FILENO);

            free(cmd[argc + 1]);
            cmd[argc + 1] = NULL;
            chdir(ev->PWD);

            exit(execvp(cmd[0], cmd));
            break;
        default:
            if (waitpid(fils, &returnfils, 0) != -1)
            {

                ev->STATUS_MSG[0] = '\0';
                if (WIFSIGNALED(returnfils)) ev->STATUS_CODE = 255; 
		        else ev->STATUS_CODE = WEXITSTATUS(returnfils);

            }
            else
            {
                strcpy(ev->STATUS_MSG, "Error : unknow command");
                ev->STATUS_CODE = 127;
            }
            break;
        }
    }
    if (ev->STATUS_MSG[0] != '\0')
    {
        write(output, ev->STATUS_MSG, strlen(ev->STATUS_MSG));
        write(output, "\n", 1);
    }

    if(input != STDIN_FILENO) {
        dup2(STDIN_FILENO, input);
        close(input);
    }
    if (output != STDOUT_FILENO) {
        dup2(STDOUT_FILENO, output);
        close(output);
    }
    if (error != STDERR_FILENO) {
        dup2(STDERR_FILENO, error);
        close(error);
    }

    return ev->STATUS_CODE;
}
