#include "cmd.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int logical_reference(EV* ev);
int physical_reference(EV* ev);
int error(EV* ev, char* msg, char* value);

int pwd(int argc, char** argv, EV* ev) {

    if(argc > 1) return error(ev, "Error : pwd takes only 1 argument -> pwd [-L | -P]", NULL);
    else if(argc == 0) return logical_reference(ev);
    else if(argv[1][0] == '-'){
        switch(argv[1][1]) {
            case 'P': return physical_reference(ev);
            case 'L': return logical_reference(ev);
            default : return error(ev, "Error : invalid argument ", argv[1]);
        }
    }
    
    return error(ev, "Error : invalid argument ", argv[1]);
}

int logical_reference(EV* ev) {
    strcpy(ev->STATUS_MSG, ev->PWD);
    return 0;
}

int physical_reference(EV* ev) {
    char root[MAX_PATH_LEN];
    strcpy(root, ev->PWD);
    strcat(root, "/.");

    int res = construct_physical_path(root, ev->STATUS_MSG);

    switch(res) {
        case 0: return 0;
        case FILE_NOT_FOUND_EXCEPTION: return error(ev, "Error : no such file or directory", NULL);
        case PERMISSION_DENIED_EXCEPTION: return error(ev, "Error : permission denied", NULL);
        case NAME_TOO_LONG_EXCEPTION: return error(ev, "Error : file name too long", NULL);
        default: return error(ev, "Error : unable to execute pwd", NULL);
    }
}

int error(EV* ev, char* msg, char* value) {
    strcpy(ev->STATUS_MSG, msg);
    if(value != NULL)
        strcat(ev->STATUS_MSG, value);
    return 1;
} 