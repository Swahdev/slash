#ifndef CMD_H
#define CMD_H

#include "ev.h"


extern int pwd(int argc, char** argv, struct EV* ev);
extern int cd(int argc, char** argv, EV* ev);
extern int ls(char** args, EV* ev);

extern int error(EV* ev, char* msg, char* value);

#endif