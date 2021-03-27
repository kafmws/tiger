#include <string.h>

#include "util.h"

typedef struct tree* Tree;

struct tree {
  Tree left;
  string key;
  Tree right;
};

Tree newTree(Tree l, string k, Tree r) {
  Tree t = checked_malloc(sizeof(*t));
  t->left = l;
  t->key = k;
  t->right = r;
  return t;
}

// seems copy all the nodes in the path of comparation
Tree insert(string key, Tree t) {
  if (t == NULL)
    return newTree(NULL, key, NULL);
  else if (strcmp(key, t->key) < 0)
    return newTree(insert(key, t->left), t->key, t->right);
  else if (strcmp(key, t->key) > 0)
    return newTree(t->left, t->key, insert(key, t->right));
  else
    return newTree(NULL, key, NULL);
}

//a.    Implement a `member` function that returns TRUE
//      if the item is found, else FALSE.
bool member(string key, Tree t) {
  if (t == NULL)
    return FALSE;
  else if (strcmp(key, t->key) < 0)
    return member(key, t->left);
  else if (strcmp(key, t->key) > 0)
    return member(key, t->right);
  else
    return TRUE;
}

//b.    Extend the program to include not just the membership,
//      but the mapping of the keys to bingings.
Tree insert_(string key, void *binding, Tree t){
    
}

void *lookup(string key, Tree t){

}

