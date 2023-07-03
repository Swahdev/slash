#include "cmd.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int change_symbolic_directory(char* from, char* to, EV* ev);
int change_physical_path(char* from, char* to, EV* ev, int physical);
int change_directory(char* from, char* to, EV* ev);

int cd(int argc, char** argv, EV* ev) {

    if(argc == 0) {
        return change_symbolic_directory(ev->HOME, "", ev);
    }
    if(argc == 1) {
        if(argv[1][0] == '-') {
            if(argv[1][1] == '\0') return change_symbolic_directory(ev->MEM_DIR, "", ev);
            if(argv[1][1] == 'L' || argv[1][1] == 'P') return change_symbolic_directory(ev->HOME, "", ev);
            return error(ev, "Error : invalid argument ", argv[1]);
        }
        if(argv[1][0] == '/') return change_symbolic_directory("/", argv[1] + 1, ev);
        return change_symbolic_directory(ev->PWD, argv[1], ev);
    }
    if(argc == 2) {
        if(argv[1][0] != '-') return error(ev, "Error : invalid arguments cd [-L | -P] [ref | -]", NULL);
        if(argv[1][1] == 'L') {
            if(argv[2][0] == '/') return change_symbolic_directory("/", argv[2] + 1, ev);
            return change_symbolic_directory(ev->PWD, argv[2], ev);
        }
        if(argv[1][1] == 'P') {
            if(argv[2][0] == '/') return change_physical_path("/", argv[2] + 1, ev, 1);
            return change_physical_path(ev->PWD, argv[2], ev, 1);
        }

        return error(ev, "Error : invalid argument ", argv[1]);
    }

    return error(ev, "Error : invalid arguments cd [-L | -P] [ref | -]", NULL);
}

int get_error(int res, char* to, EV* ev) {
    switch(res) {
        case 0: return 0;
        case FILE_NOT_FOUND_EXCEPTION: return error(ev, "Error : no such directory ", to);
        case PERMISSION_DENIED_EXCEPTION: return error(ev, "Error : permission denied", NULL);
        case NOT_A_DIRECTORY_EXCEPTION: return error(ev, "Error : not a directory ", to);
        default: return error(ev, "Error : failed to change directory ", to);
    }   
}

int change_symbolic_directory(char* from, char* to, EV* ev) {
    char path[MAX_PATH_LEN];
    strcpy(path, from);

    int res = concat_path(path, to, 0);

    if(res == 0) {
        strcpy(ev->MEM_DIR, ev->PWD);
        strcpy(ev->PWD, path);

        ev->STATUS_MSG[0] = '\0';
        chdir(ev->PWD);
        
        return 0;
    }
    return change_physical_path(from, to, ev, 0);
}

int change_physical_path(char* from, char* to, EV* ev, int physical) {
    char pPath[MAX_PATH_LEN];
    if(construct_physical_path(from, pPath) != 0) 
        return error(ev, "Error : failed to change directory ", to);
    
    int res = concat_path(pPath, to, 1);
    get_error(res, to, ev);

    if(res == 0) {
        strcpy(ev->MEM_DIR, ev->PWD);

        char root[MAX_PATH_LEN];
        strcpy(root, pPath);
        strcat(root, "/.");
        construct_physical_path(root, ev->PWD);

        ev->STATUS_MSG[0] = '\0';
        chdir(ev->PWD);
        return 0;
    }

    return 1;
}

int change_directory(char* from, char* to, EV* ev) {
    char path[MAX_PATH_LEN];
    strcpy(path, from);

    int res = concat_path(path, to, 0);
    int err = get_error(res, to, ev);

    if(res == 0) {
        strcpy(ev->MEM_DIR, ev->PWD);
        strcpy(ev->PWD, path);

        ev->STATUS_MSG[0] = '\0';
    }
    return err;
}
