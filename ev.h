#ifndef EV_H
#define EV_H

typedef struct EV {
    char* HOME;
    char* PWD;
    char* MEM_DIR;
    struct sigaction *SIG_GEST;

    unsigned int STATUS_CODE;
    char* STATUS_MSG;
    char* ERROR_MSG;
} EV;

#endif