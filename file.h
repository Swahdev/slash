#define MAX_PATH_LEN 4096
#define MAX_ARGS_NUMBER 512
#define MAX_ARGS_STRLEN 1024
#define MAX_LINE_SIZE 4096
#define PROMPT_LENGHT 47

#define EXCEPTION 1
#define FILE_NOT_FOUND_EXCEPTION 2
#define PERMISSION_DENIED_EXCEPTION 3
#define NAME_TOO_LONG_EXCEPTION 4
#define NOT_A_DIRECTORY_EXCEPTION 5

extern int construct_physical_path(char* root, char* path);
extern int concat_path(char* path1, char* path2, int physical);
