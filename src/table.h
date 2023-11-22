#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include "data.h"

typedef struct
{
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct
{
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void *pages[TABLE_MAX_PAGES];
} Pager;

typedef struct
{
  uint32_t root_page_num;
  Pager *pager;
} SimpleTable;

typedef struct
{
  SimpleTable *table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;
} Cursor;

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

void *get_page(Pager *pager, uint32_t page_num);
Pager *Pager_open(const char *filename);
void pager_flush(Pager *pager, uint32_t page_num);
void db_close(SimpleTable *table);

SimpleTable *SimpleTable_open(const char *filename);
void SimpleTable_free(SimpleTable *t);

void *cursor_value(Cursor *cursor);
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);
void initialize_leaf_node(void* node);
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);

Cursor *table_start(SimpleTable *table);
Cursor *table_end(SimpleTable *table);
void cursor_advance(Cursor *cursor);

Cursor* table_find(SimpleTable* table, uint32_t key);
Cursor* leaf_node_find(SimpleTable* table, uint32_t page_num, uint32_t key);
Cursor *table_start(SimpleTable *table);
NodeType get_node_type(void* node);
void set_node_type(void* node, NodeType type);

#endif