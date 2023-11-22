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
//    | type | nkeys | pointers   | offsets    | key-values
//    | 2B   | 2B    | nkeys * 8B | nkeys * 2B | ...
//
//    This is the format of the KV pair. Lengths followed by data.
//    | klen | vlen | key | val |
//    | 2B | 2B | ... | ... |
//

typedef struct {
  void *data[BTREE_PAGE_SIZE];
} BNode;

typedef struct {
  uint16_t key_size;
  uint16_t val_size;
  void *key;
  void *val;
} KVPair;

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

size_t header_size(){
  return sizeof(NodeType) + sizeof(uint16_t);
}

NodeType node_type(BNode *node) { return *(NodeType *)node->data; }

uint16_t *node_nkeys(BNode *node) {
  return (uint16_t *)(node->data + sizeof(NodeType));
}

// returns the position of an offset at index idx
//                     size until->
//    | type | nkeys | pointers   | offsets    | key-values
//    | 2B   | 2B    | nkeys * 8B | nkeys * 2B | ...
size_t offset_pos(BNode *node, int idx) {
  uint16_t nkeys = *node_nkeys(node);
  assert(1 <= idx && idx <= nkeys);
  return header_size() + nkeys * sizeof(BNode*) + sizeof(uint16_t) * (idx - 1);;
}

// The offset is relative to the position of the first KV pair.
// • The offset of the first KV pair is always zero, so it is not stored in the
//   list. 
// • We store the offset to the end of the last KV pair in the offset
//   list, which is used to determine the size of the node.

// returns the offset starting from the first kv pair
uint16_t get_offset(BNode *node, int idx) {
  if (idx == 0) {
    return 0;
  }
  size_t data_offset = offset_pos(node, idx);
  uint16_t ptr;
  memcpy(&ptr, (char *)node->data + data_offset, sizeof(uint16_t));
  return ptr;
}

void set_offset(BNode *node, int idx, uint16_t offset) {
  size_t data_offset = offset_pos(node, idx);
  memcpy((char *)node->data + data_offset, &offset, sizeof(uint16_t));
}

uint16_t get_kv_pos(BNode* node, int idx) {
  size_t nkeys = *node_nkeys(node);
  assert(idx <= nkeys);

  uint16_t offset = get_offset(node, idx);
  // header + pointers + offsets + offset[idx]
  return header_size() + nkeys * sizeof(BNode*) + nkeys * sizeof(uint16_t) + offset;
}

char *get_key(BNode *node, int idx){
  uint16_t pos = get_kv_pos(node, idx);
  char* ptr;
  memcpy(&ptr, (char *)node->data + pos + 2 * sizeof(uint16_t), sizeof(char*));
  return ptr;
}

uint16_t get_key_size(BNode *node, int idx){
  uint16_t pos = get_kv_pos(node, idx);
  uint16_t key_size;
  memcpy(&key_size, (char *)node->data + pos, sizeof(uint16_t));
  return key_size;
}

char *get_val(BNode *node, int idx){
  uint16_t pos = get_kv_pos(node, idx);
  uint16_t key_size;
  memcpy(&key_size, (char *)node->data + pos, sizeof(uint16_t));
  char* ptr;
  memcpy(&ptr, (char *)node->data + pos + 2 * sizeof(uint16_t) + key_size, sizeof(char*));
  return ptr;
}

// returns a pointer to a node stored in the data at index idx
// returns the position of an offset at index idx
//                   ->x
//    | type | nkeys | pointers   | offsets    | key-values
//    | 2B   | 2B    | nkeys * 8B | nkeys * 2B | ...

BNode *get_child_node(BNode *node, int idx) {
  assert(idx < *node_nkeys(node));
  size_t offset = header_size() + idx * sizeof(BNode *);
  BNode *ptr;
  memcpy(&ptr, (char *)node->data + offset, sizeof(BNode *));
  return ptr;
}

// stores a pointer to a node in the data at index idx
void set_child_node(BNode *node, BNode *child, int idx) {
  assert(idx < *node_nkeys(node));
  size_t offset = header_size() + (idx * sizeof(BNode *));
  memcpy((char *)node->data + offset, &child, sizeof(BNode *));
}

void set_header(BNode *node, NodeType *type, uint16_t *nkeys) {
  memcpy(node->data, (int *)type, sizeof(NodeType));
  memcpy(node->data + sizeof(NodeType), nkeys, sizeof(uint16_t));
}

// returns the index of the first kid node whose range intersects the key. (kid[i] <= key)
// TODO: bisect
uint16_t node_ins_idx(BNode *node, char* key){
  size_t nkeys = *node_nkeys(node);
  uint16_t found = 0;

  for (uint16_t i = 0; i < nkeys; ++i){
    uint16_t key_size = get_key_size(node, i);
    char *node_key = get_key(node, i);
    int cmp = memcmp(get_key(node, i), key, key_size);
    if (cmp <= 0) {
      found = i;
    }
    if (cmp > 0){
      break;
    }
  }

  return found;
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

  printf("Offsets    :\n");
  for (int i = 0; i < nkeys; ++i) {
    uint16_t offset = get_offset(node, i);
    printf("[%d]        :%d\n", i, offset);
  }

  return;
}

void run_tests() {
  printf("Running tests...\n");
  BNode node = {0};
  NodeType type = NODE;
  uint16_t nkeys = 2;
  set_header(&node, &type, &nkeys);

  assert(NODE == node_type(&node));
  assert(2 == *node_nkeys(&node));

  BNode child_node_1 = {0};
  BNode child_node_2 = {0};

  set_child_node(&node, &child_node_1, 0);
  set_child_node(&node, &child_node_2, 1);
  set_offset(&node, 1, 12);

  assert(&child_node_1 == get_child_node(&node, 0));
  assert(&child_node_2 == get_child_node(&node, 1));
  assert(12 == get_offset(&node, 1));

  printf("All tests passed !\n");
}

int main() {
  run_tests();

  return 0;
}
