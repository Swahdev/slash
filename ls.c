#include "cmd.h"
#include "file.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "string.h"
#include <string.h>

int ls(char** args, EV* ev) {
    DIR* dirp=opendir(ev->PWD);
    ev->STATUS_MSG[0] = '\0';

    struct dirent* entry; 
    while((entry=readdir(dirp))){
        strcat(ev->STATUS_MSG, entry->d_name);
        strcat(ev->STATUS_MSG, "\n");
    }

    closedir(dirp);

    return 0;
}