#include <stdio.h>

int input_file_handler(char* input_file_path,
                       int (*continuation)(FILE* input_file));
