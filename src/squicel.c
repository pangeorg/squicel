#include "squicel.h"

void process_input(InputBuffer *input_buffer, Table *table) {
  // check if we do a meta command
  if (input_buffer->buffer[0] == '.') {
    switch (do_meta_command(input_buffer)) {
    case (META_COMMAND_SUCCESS):
      return;
    case (META_COMMAND_UNRECOGNIZED_COMMAND):
      printf("Unrecognized command '%s'\n", input_buffer->buffer);
      return;
    }
  }

  // statement
  Statement statement;
  switch (prepare_statement(input_buffer, &statement)) {
  case (PREPARE_SUCCESS):
    return;
  case (PREPARE_UNRECOGNIZED_STATEMENT):
    printf("Unrecognized statement '%s'\n", input_buffer->buffer);
    return;
  }
  execute_statement(&statement, table);
}
