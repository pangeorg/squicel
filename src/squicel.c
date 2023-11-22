#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "squicel.h"
#include "sqio.h"
#include "table.h"

const char *META_EXIT = ".exit";
const char *STMNT_SELECT = "select";
const char *STMNT_INSERT = "insert";

void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void print_table(SimpleTable *table)
{
    Row row;
    Cursor *c = table_start(table);
    while (!c->end_of_table)
    {
        deserialize_row(cursor_value(c), &row);
        printf("(%d, %s, %s)\n", row.id, row.username, row.email);
        cursor_advance(c);
    }
    free(c);
}

MetaCommandResult do_meta_command(InputBuffer *buffer, SimpleTable *table)
{
    if (strcmp(buffer->buffer, META_EXIT) == 0)
    {
        printf("\nGoodbye!\n");
        db_close(table);
        InputBuffer_free(buffer);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement)
{
    if (strncmp(input_buffer->buffer, STMNT_INSERT, 6) == 0)
    {
        statement->type = STATEMENT_INSERT;
        strtok(input_buffer->buffer, " ");
        char *id_string = strtok(NULL, " ");
        char *username = strtok(NULL, " ");
        char *email = strtok(NULL, " ");

        if (id_string == NULL || username == NULL || email == NULL)
        {
            return PREPARE_SYNTAX_ERROR;
        }
        int id = atoi(id_string);

        if (strlen(username) > COLUMN_USERNAME_SIZE)
        {
            return PREPARE_STRING_TOO_LONG;
        }
        if (strlen(email) > COLUMN_USERNAME_SIZE)
        {
            return PREPARE_STRING_TOO_LONG;
        }

        statement->row_insert.id = id;
        strcpy(statement->row_insert.username, username);
        strcpy(statement->row_insert.email, email);
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, STMNT_SELECT) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_statement(Statement *statement, SimpleTable *table)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);
        break;
    case (STATEMENT_SELECT):
        execute_select(statement, table);
        break;
    }
    return EXECUTE_FAILURE;
}

ExecuteResult execute_insert(Statement *statement, SimpleTable *table)
{
    if (statement->type != STATEMENT_INSERT)
    {
        printf("Trying to execute insert while statement type is different!\n");
        return EXECUTE_FAILURE;
    }

    void *node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS)
    {
        return EXECUTE_TABLE_FULL;
    }

    Row *row_to_insert = &(statement->row_insert);
    uint32_t key_to_insert = row_to_insert->id;
    Cursor *cursor = table_find(table, key_to_insert);

    if (cursor->cell_num < num_cells)
    {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert)
        {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);
    free(cursor);

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, SimpleTable *table)
{
    if (statement->type != STATEMENT_SELECT)
    {
        printf("Trying to execute select while statement type is different!\n");
        return EXECUTE_FAILURE;
    }
    Cursor *c = table_start(table);

    Row row;
    while (!c->end_of_table)
    {
        deserialize_row(cursor_value(c), &row);
        print_row(&row);
        cursor_advance(c);
    }

    free(c);
    return EXECUTE_SUCCESS;
}

void process_input(InputBuffer *input_buffer, SimpleTable *table)
{
    // ignore empty lines
    if (strlen(input_buffer->buffer) == 0)
    {
        return;
    }

    // check if we do a meta command
    if (input_buffer->buffer[0] == '.')
    {
        switch (do_meta_command(input_buffer, table))
        {
        case (META_COMMAND_SUCCESS):
            return;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
            printf("Unrecognized command '%s'\n", input_buffer->buffer);
            return;
        }
    }

    // statement
    Statement statement;
    switch (prepare_statement(input_buffer, &statement))
    {
    case (PREPARE_SUCCESS):
        break;
    case (PREPARE_UNRECOGNIZED_STATEMENT):
        printf("Unrecognized statement '%s'\n", input_buffer->buffer);
        return;
    case (PREPARE_SYNTAX_ERROR):
        printf("SYNTAX ERROR '%s'\n", input_buffer->buffer);
        return;
    case (PREPARE_STRING_TOO_LONG):
        printf("STRING TOO LONG '%s'\n", input_buffer->buffer);
        return;
    }
    switch (execute_statement(&statement, table))
    {
    case (EXECUTE_SUCCESS):
        printf("Executed.\n");
        break;
    case (EXECUTE_DUPLICATE_KEY):
        printf("Error: Duplicate key.\n");
        break;
    case (EXECUTE_TABLE_FULL):
        printf("Error: Table full.\n");
        break;
    }
}

InputBuffer *InputBuffer_new()
{
    InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void InputBuffer_free(InputBuffer *buffer)
{
    free(buffer->buffer);
    free(buffer);
}

void read_input(InputBuffer *input_buffer)
{
    size_t bytes_read =
        getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    // Ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void print_prompt() { printf("clite > "); }

void squicel(const char *filename)
{
    InputBuffer *input_buffer = InputBuffer_new();
    SimpleTable *table = SimpleTable_open(filename);
    while (true)
    {
        print_prompt();
        read_input(input_buffer);
        process_input(input_buffer, table);
    }
}