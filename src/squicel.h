#ifndef SQUICEL_H
#define SQUICEL_H

#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

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

// in memory data structure
typedef struct
{
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct
{
  uint32_t num_rows;
  void *pages[TABLE_MAX_PAGES];
} SimpleTable;

SimpleTable *SimpleTable_new();
void SimpleTable_free(SimpleTable *t);

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROW_SIZE;

extern const uint32_t PAGE_SIZE;
extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

void *row_slot(SimpleTable *table, uint32_t row_num);
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

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

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND,
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR,
  PREPARE_STRING_TOO_LONG,
} PrepareResult;

typedef enum {
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

MetaCommandResult do_meta_command(InputBuffer *buffer);
PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement);

ExecuteResult execute_statement(Statement *statement, SimpleTable *table);
ExecuteResult execute_insert(Statement *statement, SimpleTable *table);
ExecuteResult execute_select(Statement *statement, SimpleTable *table);

// main functions
void print_prompt();
void squicel();
void print_row(Row *row);
void print_table(SimpleTable *table);
void *row_slot(SimpleTable *table, uint32_t row_num);
void process_input(InputBuffer *input_buffer, SimpleTable *table);

#endif