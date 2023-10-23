#ifndef SQCMD_H_
#define SQCMD_H_

#include "sqio.h"
#include "sqinmem.h"

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;


typedef enum { 
  PREPARE_SUCCESS, 
  PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;

typedef enum { 
  STATEMENT_INSERT, 
  STATEMENT_SELECT 
} StatementType;

typedef struct {
  StatementType type;
  Row row_insert;
} Statement;

typedef enum { 
  EXECUTE_SUCCESS,
  EXECUTE_TABLE_FULL,
  EXECUTE_FAILURE
} ExecuteResult;

MetaCommandResult do_meta_command(InputBuffer* buffer);
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);

ExecuteResult execute_statement(Statement *statement, Table *table);
static ExecuteResult execute_insert(Statement *statement, Table *table);
static ExecuteResult execute_select(Statement *statement, Table *table);

#endif // !SQCMD_H
