#include "file.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

void parsePath(char *line, char sep, char **tab, int *tabLength);

int get_repertory_name(char *rep, char *name)
{

    struct stat st1;
    if (stat(rep, &st1) == -1)
        return FILE_NOT_FOUND_EXCEPTION;

    strcat(rep, "/..");

    DIR *dirp = opendir(rep);
    if (dirp == NULL)
        return FILE_NOT_FOUND_EXCEPTION;

    char path[MAX_PATH_LEN];

    struct dirent *entry;
    while ((entry = readdir(dirp)))
    {
        struct stat st2;

        strcpy(path, rep);
        strcat(path, "/");
        strcat(path, entry->d_name);

        if (stat(path, &st2) == -1)
            continue;

        if (st2.st_ino == st1.st_ino && st1.st_dev == st2.st_dev)
        {
            closedir(dirp);
            strcpy(name, entry->d_name);
            return 0;
        }
    }
    closedir(dirp);

    return FILE_NOT_FOUND_EXCEPTION;
}

// Returns  1 if rep is root
// Returns  0 if not
// Returns -1 on error
int is_root(char *rep)
{
    struct stat st1;
    struct stat st2;

    char path[MAX_PATH_LEN];
    strcpy(path, rep);
    strcat(path, "/..");

    if (stat(rep, &st1) == -1)
        return -1;
    if (stat(path, &st2) == -1)
        return -1;

    if (st1.st_ino == st2.st_ino && st1.st_dev == st2.st_dev)
        return 1;
    return 0;
}

int construct_path(char *currentFile, char *path)
{
    int isRoot = is_root(currentFile);

    if (isRoot)
        return 0;
    if (isRoot == -1)
    {
        switch (errno)
        {
        case ENOENT:
            return FILE_NOT_FOUND_EXCEPTION;
        case EACCES:
            return PERMISSION_DENIED_EXCEPTION;
        default:
            return EXCEPTION;
        }
    }

    char str[MAX_PATH_LEN];
    str[0] = '/';
    str[1] = '\0';

    char fileName[MAX_PATH_LEN];
    int res = get_repertory_name(currentFile, fileName);
    if (res != 0)
        return res;

    strcat(str, fileName);
    strcat(str, path);
    strcpy(path, str);

    return construct_path(currentFile, path);
}

int construct_physical_path(char *root, char *path)
{
    char *res = realpath(root, path);

    if (res == NULL)
    {
        switch (errno)
        {
        case ENOENT:
            return FILE_NOT_FOUND_EXCEPTION;
        case EACCES:
            return PERMISSION_DENIED_EXCEPTION;
        case ENOTDIR:
            return NOT_A_DIRECTORY_EXCEPTION;
        default:
            return EXCEPTION;
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////

unsigned int getLastSlash(char *path)
{
    unsigned int res = 0;
    for (int i = 0; path[i] != '\0' && path[i] != '\n'; i++)
    {
        if (path[i] == '/')
            res = i;
    }

    return res;
}

int open_dir(char *path, char *dir)
{
    DIR *dirp = opendir(path);
    if (dirp == NULL)
    {
        switch (errno)
        {
        case ENOENT:
            return FILE_NOT_FOUND_EXCEPTION;
        case EACCES:
            return PERMISSION_DENIED_EXCEPTION;
        case ENOTDIR:
            return NOT_A_DIRECTORY_EXCEPTION;
        default:
            return EXCEPTION;
        }
    }

    struct dirent *entry;
    while ((entry = readdir(dirp)))
    {
        if (strcmp(entry->d_name, dir) == 0)
        {
            if (path[strlen(path) - 1] != '/')
                strcat(path, "/");
            strcat(path, dir);

            closedir(dirp);
            return 0;
        }
    }

    closedir(dirp);
    return FILE_NOT_FOUND_EXCEPTION;
}

int fri(char **tab, int ret)
{
    for (int i = 0; i < MAX_PATH_LEN; i++)
        free(tab[i]);
    free(tab);

    return ret;
}

int concat_path(char *path1, char *path2, int physical)
{
    int tabLength = 0;
    char **tab = (char **)malloc(MAX_PATH_LEN * sizeof(char *));
    for (int i = 0; i < MAX_PATH_LEN; i++)
        tab[i] = (char *)malloc(MAX_PATH_LEN * sizeof(char));

    parsePath(path2, '/', tab, &tabLength);

    for (int i = 0; i < tabLength + 1; i++)
    {
        if (tab[i][0] == '.')
        {
            if (tab[i][1] == '.')
            {
                if (physical)
                {
                    char tmp[MAX_PATH_LEN] = "\0";
                    strcpy(tmp, path1);
                    int res = construct_physical_path(tmp, path1);
                    if (res != 0)
                        return res;
                }

                unsigned int index = getLastSlash(path1);
                // root achieve
                if (index == 0)
                {
                    path1[0] = '/';
                    index = 1;
                }
                path1[index] = '\0';
            }
        }
        else
        {
            if (strlen(path1) > 1)
                strcat(path1, "/");
            strcat(path1, tab[i]);
        }
    }

    DIR *dirp = opendir(path1);
    if (dirp == NULL)
    {
        switch (errno)
        {
        case ENOENT:
            return fri(tab, FILE_NOT_FOUND_EXCEPTION);
        case EACCES:
            return fri(tab, PERMISSION_DENIED_EXCEPTION);
        case ENOTDIR:
            return fri(tab, NOT_A_DIRECTORY_EXCEPTION);
        default:
            return fri(tab, EXCEPTION);
        }
    }
    closedir(dirp);
    return fri(tab, 0);
}

void parsePath(char *line, char sep, char **tab, int *tabLength)
{

    int nbArgs = 0;
    int wordLength = 0;
    int ignore = 1;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == sep && !ignore)
        {
            tab[nbArgs][wordLength] = '\0';

            nbArgs++;

            wordLength = 0;

            ignore = 1;
        }
        else
        {
            tab[nbArgs][wordLength++] = line[i];
            ignore = 0;
        }
    }

    tab[nbArgs][wordLength] = '\0';

    if (ignore)
        nbArgs--;

    *tabLength = nbArgs;
}
