#include <stdbool.h>
#include "squicel.h"

static void print_prompt() { printf("clite > "); }

static void squicel(){
  InputBuffer *input_buffer = InputBuffer_new();
  Table *table = Table_new();
  while (true) {
    print_prompt();
    read_input(input_buffer);
    process_input(input_buffer, table);
  }
}

int main(int argc, char *argv[]) {

  squicel();

  return 0;
}

