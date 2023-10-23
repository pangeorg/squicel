#ifndef SQINMEM_H_
#define SQINMEM_H_

#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef struct {
  uint32_t num_rows;
  void *pages[TABLE_MAX_PAGES];
} Table;

Table *Table_new();
void Table_free(Table *t);
extern const uint32_t TABLE_MAX_ROWS;


// figure out where to read/write within the table
void *row_slot(Table *table, uint32_t row_num);
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

#endif // !SQINMEM_H
