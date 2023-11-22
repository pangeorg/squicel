#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// page layout:
// 4 bytes for the header
// 4kB for the page
#define HEADER 4
#define BTREE_PAGE_SIZE 4096
#define BTREE_MAX_KEY_SIZE 1000
#define BTREE_MAX_VAL_SIZE 3000

//  A node consists of:
//  1. A fixed-sized header containing the type of the node (leaf node or
//  internal node)
//     and the number of keys.
//  2. A list of pointers to the child nodes. (Used by internal nodes).
//  3. A list of offsets pointing to each key-value pair.
//  4. Packed KV pairs.
//    | type | nkeys | pointers | offsets | key-values
//    | 2B | 2B | nkeys * 8B | nkeys * 2B | ...
//    This is the format of the KV pair. Lengths followed by data.
//    | klen | vlen | key | val |
//    | 2B | 2B | ... | ... |
//

typedef struct {
  void *data[BTREE_PAGE_SIZE];
} BNode;

typedef struct {
  BNode *root; // pointer to non-zero page number
} BTree;

typedef enum {
  NODE, // internal nodes without values
  LEAF, // nodes with values
} NodeType;

BNode get_node(uint64_t prt); // 'dereference' the pointer to a page
uint64_t new_node(void);      // allocate new page
void del_node(BNode *node);   // deallocate a page

void check_init(void) {
  // header + 1*pointer size + 1*offset size + 1*key size + 1*val size + key +
  // val
  int check = HEADER + 8 + 2 + 4 + BTREE_MAX_KEY_SIZE + BTREE_MAX_VAL_SIZE;
  assert(check <= BTREE_PAGE_SIZE);
}

NodeType node_type(BNode *node) { return *(NodeType *)node->data; }

uint16_t *node_nkeys(BNode *node) {
  return (uint16_t *)(node->data + sizeof(NodeType));
}

size_t offset_pos(BNode *node) {
  uint16_t nkeys = *node_nkeys(node);
  return sizeof(NodeType) + sizeof(uint16_t) + nkeys * sizeof(BNode*);
}

// returns a pointer to a node stored in the data at index idx
BNode *get_child_node(BNode *node, int idx) {
  assert(idx < *node_nkeys(node));
  size_t offset = sizeof(NodeType) + sizeof(uint16_t) + idx * sizeof(BNode *);
  BNode *ptr;
  memcpy(&ptr, (char *)node->data + offset, sizeof(BNode *));
  return ptr;
}

// stores a pointer to a node in the data at index idx
void set_child_node(BNode *node, BNode *child, int idx) {
  assert(idx < *node_nkeys(node));
  size_t offset = sizeof(NodeType) + sizeof(uint16_t) + (idx * sizeof(BNode *));
  memcpy((char *)node->data + offset, &child, sizeof(BNode *));
}

void set_header(BNode *node, NodeType *type, uint16_t *nkeys) {
  memcpy(node->data, (int *)type, sizeof(NodeType));
  memcpy(node->data + sizeof(NodeType), nkeys, sizeof(uint16_t));
}

// The offset is relative to the position of the first KV pair.
// • The offset of the first KV pair is always zero, so it is not stored in the
//   list. 
// • We store the offset to the end of the last KV pair in the offset
//   list, which is used to determine the size of the node.

// returns a pointer to an offset stored in the data at index idx

uint16_t* get_offset(BNode *node, int idx) {
  uint16_t nkeys = *node_nkeys(node);
  assert(1 <= idx && idx <= nkeys);

  size_t data_offset = offset_pos(node) + 2 * (idx - 1);
  uint16_t *ptr;

  memcpy(&ptr, (char *)node->data + data_offset, sizeof(uint16_t *));
  return ptr;
}

void print_header(BNode *node) {
  NodeType type = node_type(node);
  uint16_t nkeys = *node_nkeys(node);
  printf("Node header:\n");
  printf("Type       :%d\n", (int)type);
  printf("NKeys      :%d\n", (int)nkeys);
  printf("Pointers   :\n");
  for (int i = 0; i < nkeys; ++i) {
    void *ptr = get_child_node(node, i);
    printf("[%d]        :%p\n", i, ptr);
  }
  return;
}

int main() {
  // one node on the stack
  BNode node = {0};
  NodeType type = NODE;
  uint16_t nkeys = 2;
  set_header(&node, &type, &nkeys);

  BNode *child_node_1 = malloc(sizeof(BNode));
  BNode *child_node_2 = malloc(sizeof(BNode));
  printf("Adress node1   : %p\n", child_node_1);
  printf("Adress node2   : %p\n", child_node_2);

  set_child_node(&node, child_node_1, 0);
  set_child_node(&node, child_node_2, 1);

  print_header(&node);

  return 0;
}
