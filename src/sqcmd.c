#include "sqcmd.h"

// command constants
const char *META_EXIT = ".exit";
const char *STMNT_SELECT = "select";
const char *STMNT_INSERT = "insert";

MetaCommandResult do_meta_command(InputBuffer *buffer) {
  if (strcmp(buffer->buffer, META_EXIT) == 0) {
    printf("\nGoodbye!\n");
    InputBuffer_free(buffer);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement) {
  if (strncmp(input_buffer->buffer, STMNT_INSERT, 6) == 0) {
    statement->type = STATEMENT_INSERT;
    return PREPARE_SUCCESS;
  }
  if (strcmp(input_buffer->buffer, STMNT_SELECT) == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCCESS;
  }

  return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_statement(Statement *statement, Table *table) {
  switch (statement->type) {
  case (STATEMENT_INSERT):
    return execute_insert(statement, table);
    break;
  case (STATEMENT_SELECT):
    execute_select(statement, table);
    break;
  }
  return EXECUTE_FAILURE;
}

static ExecuteResult execute_insert(Statement *statement, Table *table) {
  if (statement->type != STATEMENT_INSERT) {
    printf("Trying to execute insert while statement type is different!\n");
    return EXECUTE_FAILURE;
  }

  if (table->num_rows >= TABLE_MAX_ROWS) {
    printf("Table full!\n");
    return EXECUTE_TABLE_FULL;
  }

  Row* row = &(statement->row_insert);

  serialize_row(row, row_slot(table, table->num_rows));
  table->num_rows++;

  return EXECUTE_SUCCESS;
}

static ExecuteResult execute_select(Statement *statement, Table *table) {
  if (statement->type != STATEMENT_SELECT) {
    printf("Trying to execute select while statement type is different!\n");
    return EXECUTE_FAILURE;
  }

  Row row;
  for (uint32_t i = 0; i < table->num_rows; ++i){
    deserialize_row(row_slot(table, i), &row);
    // print_row(&row);
  }

  return EXECUTE_SUCCESS;
}
