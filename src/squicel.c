#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "squicel.h"
#include "sqio.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

const char *META_EXIT = ".exit";
const char *STMNT_SELECT = "select";
const char *STMNT_INSERT = "insert";

void *row_slot(SimpleTable *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == NULL)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    // how far are we into the page
    uint32_t row_offst = row_num % ROWS_PER_PAGE;
    // convert to byte offset
    uint32_t byte_offst = row_offst * ROW_SIZE;
    return page + byte_offst;
}

void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void print_table(SimpleTable *table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        printf("(%d, %s, %s)\n", row.id, row.username, row.email);
    }
}

SimpleTable *SimpleTable_new()
{
    SimpleTable *t = (SimpleTable *)malloc(sizeof(SimpleTable));
    t->num_rows = 0;
    for (int i = 0; i < TABLE_MAX_PAGES; ++i)
    {
        t->pages[i] = NULL;
    }
    return t;
}

void SimpleTable_free(SimpleTable *t)
{
    for (uint32_t page_num = 0; page_num < TABLE_MAX_PAGES; ++page_num)
    {
        free(t->pages[page_num]);
    }
    free(t);
}

void serialize_row(Row *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

MetaCommandResult do_meta_command(InputBuffer *buffer)
{
    if (strcmp(buffer->buffer, META_EXIT) == 0)
    {
        printf("\nGoodbye!\n");
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
        char* id_string = strtok(NULL, " ");
        char* username = strtok(NULL, " ");
        char* email = strtok(NULL, " ");

        if (id_string == NULL || username == NULL || email == NULL){
            return PREPARE_SYNTAX_ERROR;
        }
        int id = atoi(id_string);

        if (strlen(username) > COLUMN_USERNAME_SIZE){
            return PREPARE_STRING_TOO_LONG;
        }
        if (strlen(email) > COLUMN_USERNAME_SIZE){
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

    if (table->num_rows >= TABLE_MAX_ROWS)
    {
        printf("SimpleTable full!\n");
        return EXECUTE_TABLE_FULL;
    }

    Row *row = &(statement->row_insert);
    serialize_row(row, row_slot(table, table->num_rows));
    table->num_rows++;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, SimpleTable *table)
{
    if (statement->type != STATEMENT_SELECT)
    {
        printf("Trying to execute select while statement type is different!\n");
        return EXECUTE_FAILURE;
    }

    Row row;
    for (uint32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}

void process_input(InputBuffer *input_buffer, SimpleTable *table)
{
    // check if we do a meta command
    if (input_buffer->buffer[0] == '.')
    {
        switch (do_meta_command(input_buffer))
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
    execute_statement(&statement, table);
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

void squicel()
{
    InputBuffer *input_buffer = InputBuffer_new();
    SimpleTable *table = SimpleTable_new();
    while (true)
    {
        print_prompt();
        read_input(input_buffer);
        process_input(input_buffer, table);
    }
}

