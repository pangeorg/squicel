#ifndef SQUICEL_H
#define SQUICEL_H

#include <stdint.h>
#include <stdbool.h>

#include "table.h"

// Input buffer
typedef struct
{
  char *buffer;
  size_t buffer_length;
  size_t input_length;
} InputBuffer;

InputBuffer *InputBuffer_new();
void InputBuffer_free(InputBuffer *buffer);
void read_input(InputBuffer *input_buffer);


// COMMANDS AND STATEMENTS
// command constants

typedef enum
{
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef struct
{
  StatementType type;
  Row row_insert;
} Statement;

typedef enum
{
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND,
} MetaCommandResult;

typedef enum
{
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
} PrepareResult;

typedef enum
{
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_FAILURE,
} ExecuteResult;

typedef struct
{
  MetaCommandResult metaCommandResult;
  PrepareResult prepareResult;
  ExecuteResult executeResult;
} SquicelResult;


MetaCommandResult do_meta_command(InputBuffer *buffer, SimpleTable *table);
PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement);

ExecuteResult execute_statement(Statement *statement, SimpleTable *table);
ExecuteResult execute_insert(Statement *statement, SimpleTable *table);
ExecuteResult execute_select(Statement *statement, SimpleTable *table);

// main functions
void print_prompt();
void squicel(const char *filename);
void print_row(Row *row);
void print_table(SimpleTable *table);
void process_input(InputBuffer *input_buffer, SimpleTable *table);

#endif