
/****************************************************************

  AVL tree implementation test routines
  by Ron Niles
  November 29, 1994

  this software is placed in the public domain
  provided that you use it at your own risk

****************************************************************/

#include "avlsearch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

void permgen(unsigned base, unsigned index, unsigned * output) {
  unsigned i, j;

  /* Decompose the index in reverse, by taking successively larger mods */
  i = base;
  while (i--) {
    /* We could use div or ldiv here, but we want library independence */
    output[i] = index % (base - i);
    index = index / (base - i);
  }

  /*
   * This next loop makes sure each mod is unique, by incrementing it
   * once for each smaller value which preceeds it
   */
  i = base;
  while (i--) {
    for (j = i + 1; j < base; j++)
      if (output[j] >= output[i])
        output[j]++;
  }
}

typedef struct mytree_ {
  struct avltree tree;
  unsigned key;
} mytree;

typedef struct mynode_ {
  struct avlbind node;
  unsigned key;
  unsigned count;
  int leftheight;
  int rightheight;
} mynode;

static int inline max(int a, int b) {
  return a > b ? a : b;
}
static int compare(struct avltree *tree, struct avlbind *node) {
  unsigned lhs = ((mytree*)tree)->key;
  unsigned rhs = ((mynode*)node)->key;
  return lhs - rhs;
}

/****************************************************************
 IsAVL
 Checks that the tree is an AVL tree.
 returns number of nodes in the tree, or -1 for error
 ****************************************************************/
static int IsAVL(mynode *Node) {
  int NumEntries = 0;
  if (Node == NULL)
    return NumEntries;

  mynode *left = (mynode*)Node->node.left;
  mynode *right = (mynode*)Node->node.right;
  NumEntries++;
  if (left == NULL)
    Node->leftheight = 0;
  else {
    assert(left->key < Node->key);
    int num = IsAVL(left);
    if (num < 0)
      return -1;
    NumEntries += num;
    Node->leftheight = max(((mynode*)Node->node.left)->leftheight,
                           ((mynode*)Node->node.left)->rightheight) + 1;
  }
  if (right == NULL)
    Node->rightheight = 0;
  else {
    assert(right->key > Node->key);
    int num = IsAVL(right);
    if (num < 0)
      return -1;
    NumEntries += num;
    Node->rightheight = max(((mynode*)Node->node.right)->leftheight,
                            ((mynode*)Node->node.right)->rightheight) + 1;
  }
  assert(Node->node.balance == Node->rightheight - Node->leftheight);

  return Node->node.balance + 1 <= 2 ? NumEntries : -1;
}

/****************************************************************
 GetNode()
 Returns a node from the allocation pool
 ****************************************************************/
#define MAX_NODES 1024
mynode Nodes[MAX_NODES] = { 0 };
mynode *Avail = NULL;
int NodesInitialized = 0;

mynode *GetNode(void) {
  mynode *tmp;
  int i;

  if (!NodesInitialized) {
    NodesInitialized = 1;
    Avail = Nodes;
    for (i = 0; i < MAX_NODES - 1; i++) {
      Nodes[i].node.left = &(Nodes + i + 1)->node;
    }
    Nodes[i].node.left = NULL;
  }

  if (Avail == NULL) {
    return NULL;
  }
  tmp = Avail;
  Avail = (mynode *)(Avail->node.left);
  return tmp;
}

/****************************************************************
 FreeNode()
 ****************************************************************/
void FreeNode(mynode *node) {
  assert(
      node >= Nodes && ((char*)node-(char*)Nodes) % sizeof(mynode) == 0 && node-Nodes < MAX_NODES);
  node->node.left = &Avail->node;
  Avail = node;
}

int FreeNodeCount(void) {
  int count = 0;
  mynode *ptr = Avail;
  while (ptr) {
    ++count;
    ptr = (mynode*)ptr->node.left;
  }
  return count;
}

/****************************************************************
 FreeTree()
 ****************************************************************/
void FreeTree(struct avlbind *tree) {
  if (tree == NULL)
    return;
  FreeTree(tree->left);
  FreeTree(tree->right);
  FreeNode((mynode*)tree);
}

void PrintTree(struct avlbind *node, unsigned level) {
  unsigned k;

  if (node == NULL)
    return;

  PrintTree(node->right, level + 1);
  for (k = 0; k < level; k++)
    printf("      ");
  printf("%u %i\n", ((mynode *)node)->key, node->balance);
  PrintTree(node->left, level + 1);
}

void insert_value(mytree *tree, int val) {
  mynode *node = GetNode();
  assert(node != NULL);
  tree->key = node->key = val;
  avl_insert(&tree->tree, &node->node);
}

void delete_value(mytree *tree, int val) {
  mynode *node;
  tree->key = val;
  node = (mynode *)avl_delete(&tree->tree);
  assert(node);
  assert(node->key == val);
  FreeNode(node);
}

void MakeTestTree(mytree *tree) {
  unsigned k;
  static unsigned seq[] = { 32, 16, 40, 8, 24, 36, 44, 4, 12, 20, 28, 34, 38,
      42, 46, 2, 6, 10, 14, 18, 22, 26, 30 };
  tree->tree.root = NULL;
  for (k = 0; k < 23; k++) {
    insert_value(tree, seq[k]);
    int NumEntries = IsAVL((mynode*)tree->tree.root);
    assert(NumEntries > 0);
    assert(NumEntries == k + 1);
  }
}

void TreeTest(void) {
  int i, j, k, NumEntries;
  unsigned fact[16];
  unsigned buf[16];

  mytree tree;
  printf("Inserting into test tree at strategic positions\n");
  for (i = 1; i < 47; i += 2) {
    memset(&tree, 0, sizeof(tree));
    tree.tree.compare_key_tree = compare;
    MakeTestTree(&tree);
    if (i == 1) {
      PrintTree(tree.tree.root, 0);
    }
    insert_value(&tree, i);
    NumEntries = IsAVL((mynode*)tree.tree.root);
    assert(NumEntries >= 0);
    assert(NumEntries == 24);
    FreeTree(tree.tree.root);
    assert(FreeNodeCount() == MAX_NODES);
  }
  printf("Test passed\n");

  printf("Inserting ascending values\n");
  memset(&tree, 0, sizeof(tree));
  tree.tree.compare_key_tree = compare;
  for (i = 0; i < MAX_NODES; i++) {
    insert_value(&tree, i);
    NumEntries = IsAVL((mynode*)tree.tree.root);
    assert(NumEntries >= 0);
    assert(NumEntries == i + 1);
  }
  FreeTree(tree.tree.root);
  assert(FreeNodeCount() == MAX_NODES);
  printf("Test passed\n");

  printf("Inserting descending values\n");
  memset(&tree, 0, sizeof(tree));
  tree.tree.compare_key_tree = compare;
  for (i = MAX_NODES; i > 0; i--) {
    insert_value(&tree, i);
    NumEntries = IsAVL((mynode*)tree.tree.root);
    assert(NumEntries >= 0);
    assert(NumEntries + i == MAX_NODES + 1);
  }
  FreeTree(tree.tree.root);
  assert(FreeNodeCount() == MAX_NODES);
  printf("Test passed\n");

  printf("Inserting random values\n");
  memset(&tree, 0, sizeof(tree));
  tree.tree.compare_key_tree = compare;
  for (i = 0; i < MAX_NODES; i++) {
    insert_value(&tree, rand());
    NumEntries = IsAVL((mynode*)tree.tree.root);
    assert(NumEntries >= 0);
    assert(NumEntries == i + 1);
  }
  FreeTree(tree.tree.root);
  assert(FreeNodeCount() == MAX_NODES);
  printf("Test passed\n");

  printf("Inserting values in all possible permutations\n");
  j = 1;
  for (i = 1; i <= 15; i++) {
    j *= i;
    fact[i] = j;
  }

  /* table was initialized to 15, but values beyond 8 can take a long time */
  for (i = 2; i <= 8; i++) {
    for (j = 0; j < fact[i]; j++) {
      permgen(i, j, buf);
      memset(&tree, 0, sizeof(tree));
      tree.tree.compare_key_tree = compare;
      for (k = 0; k < i; k++) {
        insert_value(&tree, buf[k]);
        NumEntries = IsAVL((mynode*)tree.tree.root);
        assert(NumEntries >= 0);
        assert(NumEntries == k + 1);
      }
      printf("\r%07u/%07u Size: %u", j + 1, fact[i], NumEntries);
      FreeTree(tree.tree.root);
    }
    printf("\n");
  }
  assert(FreeNodeCount() == MAX_NODES);
  printf("Test passed\n");
}

void MakeSevenTree(mytree *tree, unsigned *buf) {
  int NumEntries;
  unsigned k;
  memset(tree, 0, sizeof(*tree));
  tree->tree.compare_key_tree = compare;

  for (k = 0; k < 7; k++) {
    insert_value(tree, buf[k]);
    NumEntries = IsAVL((mynode*)tree->tree.root);
    assert(NumEntries >= 0);
    assert(NumEntries == k + 1);
  }
}

unsigned long NumTrees[6];
/****************************************************************
 MakeSeqTree()
 For enumerating all possible trees of a given height
 in order by index. index must be less than NumTrees[height]
 ****************************************************************/
struct avlbind *MakeSeqTree(unsigned long index, unsigned height) {
  struct avlbind *tmp;
  unsigned long t1, t1sqr, t2;

  assert(index < NumTrees[height]);

  if (height == 0)
    return NULL;

  tmp = &GetNode()->node;
  if (height == 1) {
    tmp->left = tmp->right = NULL;
    tmp->balance = 0;
    return tmp;
  }
  t2 = NumTrees[height - 2];
  t1 = NumTrees[height - 1];
  t1sqr = t1 * t1;
  if (index < t1sqr) {
    tmp->left = MakeSeqTree(index % t1, height - 1);
    tmp->right = MakeSeqTree(index / t1, height - 1);
    tmp->balance = 0;
    return tmp;
  }
  index -= t1sqr;
  if (index < t1 * t2) {
    tmp->left = MakeSeqTree(index % t1, height - 1);
    tmp->right = MakeSeqTree(index / t1, height - 2);
    tmp->balance = -1;
    return tmp;
  }
  index -= t1 * t2;
  tmp->right = MakeSeqTree(index % t1, height - 1);
  tmp->left = MakeSeqTree(index / t1, height - 2);
  tmp->balance = 1;
  return tmp;
}

unsigned SeqVal = 0;

/****************************************************************
 FillSeqTree
 Fills the tree generated by MakeSeqTree with valid numbers
 ****************************************************************/
void FillSeqTree(struct avlbind *node) {
  if (node == NULL)
    return;
  FillSeqTree(node->left);
  ((mynode *)node)->key = SeqVal++;
  FillSeqTree(node->right);
}

void DeleteTest(void) {
  int NumEntries;
  unsigned long t;
  unsigned i, j, k;
  unsigned fact[16];
  unsigned buf[16];

  // Test all possible ways of crumbling the full tree of 7
  j = 1;
  for (i = 1; i <= 8; i++) {
    j *= i;
    fact[i] = j;
  }

  mytree tree;

  printf("Testing all possibly ways of crumbling a tree of seven\n");
  for (j = 0; j < fact[7]; j++) {
    permgen(7, j, buf);
    MakeSevenTree(&tree, buf);
    for (k = 0; k < 7; k++) {
      delete_value(&tree, buf[k]);
      NumEntries = IsAVL((mynode*)tree.tree.root);
      assert(NumEntries >= 0);
      assert(NumEntries + k == 6);
    }
    assert(FreeNodeCount() == MAX_NODES);
  }
  printf("Test passed\n");

  // Test deleting one node from all trees of height up to 5
  NumTrees[0] = NumTrees[1] = 1;
  for (i = 2; i < 6; i++)
    NumTrees[i] = NumTrees[i - 1] * (2 * NumTrees[i - 2] + NumTrees[i - 1]);

  printf("Testing deletion from all possible tree shapes to height 5\n");
  for (j = 1; j < 6; j++) {
    for (t = 0; t < NumTrees[j]; t++) {
      for (i = 0;;) {
        mytree tree;
        memset(&tree, 0, sizeof(tree));
        tree.tree.compare_key_tree = compare;
        tree.tree.root = MakeSeqTree(t, j);
        SeqVal = 0;
        FillSeqTree(tree.tree.root);
        delete_value(&tree, i);
        NumEntries = IsAVL((mynode*)tree.tree.root);
        assert(NumEntries >= 0);
        assert(NumEntries == SeqVal - 1);
        FreeTree(tree.tree.root);
        assert(FreeNodeCount() == MAX_NODES);
        if (++i == SeqVal)
          break;
      }
      if ((t & 0xf) == 0) {
        printf("%lu\r", t);
      }
    }
    printf("%lu\n", t);
  }
  printf("Test passed\n");
}

struct avlbind *MakeRandomTree(unsigned height) {
  struct avlbind *tmp;
  unsigned i;

  if (height == 0)
    return NULL;

  tmp = &GetNode()->node;
  assert(tmp);
  if (height == 1) {
    tmp->left = tmp->right = NULL;
    tmp->balance = 0;
    return tmp;
  }

  i = rand() % 3;

  if (i == 0)   // Evenly balanced
      {
    tmp->left = MakeRandomTree(height - 1);
    tmp->right = MakeRandomTree(height - 1);
    tmp->balance = 0;
  } else if (i == 1)  // tilt right
      {
    tmp->left = MakeRandomTree(height - 2);
    tmp->right = MakeRandomTree(height - 1);
    tmp->balance = 1;
  } else // tilt left
  {
    tmp->left = MakeRandomTree(height - 1);
    tmp->right = MakeRandomTree(height - 2);
    tmp->balance = -1;
  }
  return tmp;
}

void RandomPermutation(unsigned base, unsigned * output) {
  unsigned i, j;

  /* Decompose the index in reverse, by taking successively larger mods */
  i = base;
  while (i--)
    output[i] = rand() % (base - i);

  /*
   * This next loop makes sure each mod is unique, by incrementing it
   * once for each smaller value which preceeds it
   */
  i = base;
  while (i--) {
    for (j = i + 1; j < base; j++)
      if (output[j] >= output[i])
        output[j]++;
  }
}

void RandomTreeTest(void) {
  unsigned long i = 0;
  unsigned j;
  int NumEntries;
  static unsigned Perm[MAX_NODES];

  printf("Collapsing random trees of height 9 in random order\n");
  while (i < 100000) {
    mytree tree;
    memset(&tree, 0, sizeof(tree));
    tree.tree.compare_key_tree = compare;
    tree.tree.root = MakeRandomTree(9);
    SeqVal = 0;
    FillSeqTree(tree.tree.root);
    NumEntries = IsAVL((mynode*)tree.tree.root);
    assert(NumEntries == SeqVal);
    if (i == 0) {
      PrintTree(tree.tree.root, 0);
    }
    RandomPermutation(SeqVal, Perm);
    for (j = 0; j < SeqVal; j++) {
      delete_value(&tree, Perm[j]);
      NumEntries = IsAVL((mynode*)tree.tree.root);
      assert(NumEntries + j + 1 == SeqVal);
    }
    assert(tree.tree.root == NULL);
    if (i++ % 16 == 0)
      printf("\r%lu     ", i);
  }
  printf("\r%lu\nTest Passed\n", i);
}

int main(int argc, char *argv[]) {
  TreeTest();
  DeleteTest();
  RandomTreeTest();
  return 0;
}
