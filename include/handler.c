#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Starting length of error message buffers, which will be reallocated
// at error-creation time if more space is needed.
#define ERROR_LENGTH 30

int input_file_handler(char* input_file_path,
                       int (*continuation)(FILE* input_file)) {
  FILE* input_file = fopen(input_file_path, "r");
  if (input_file == NULL) {
    // Display an error message and return the appropriate error code.
    char* error_message = malloc(ERROR_LENGTH * sizeof(char));
    int error_message_length =
        snprintf(error_message, ERROR_LENGTH, "Error opening file '%s'",
                 input_file_path);

    // If the message is longer than ERROR_LENGTH, reallocate.
    if (error_message_length >= ERROR_LENGTH) {
      error_message =
          realloc(error_message, (error_message_length + 1) * sizeof(char));
      snprintf(error_message, (error_message_length + 1),
               "Error opening file '%s'", input_file_path);
    }

    perror(error_message);
    free(error_message);
    return errno;
  } else {
    return continuation(input_file);
  }
}
