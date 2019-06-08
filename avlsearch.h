
/****************************************************************

	AVL tree implementation
	by Ron Niles
	November 29, 1994

	this software is placed in the public domain
	provided that you use it at your own risk

****************************************************************/

#include <stddef.h>

#ifndef DBG_ASSERT
#define DBG_ASSERT(a) 
#endif

struct avlbind
{
	struct avlbind *left;
	struct avlbind *right;
	int balance;
};

struct avltree
{
	int (*compare_key_tree)(struct avltree *tree, struct avlbind *node);
	struct avlbind *root;
	unsigned num_nodes;
};

struct avlsearch
{
	struct avlbind **path_taken[40];
	int dir_taken[40];
	int current_level;
	struct avlbind **current_node;
};

/****************************************************************
	scroll_down_left()
	continues down a tree always taking the left branch
****************************************************************/
static struct avlbind *scroll_down_left(struct avlsearch *search)
{
	struct avlbind *tmp;

	if (*search->current_node == NULL)
		return NULL;

	for(;;)
	{
		tmp = *search->current_node;
		if (tmp->left == NULL)
			break;
		search->path_taken[search->current_level] = search->current_node;
		search->dir_taken[search->current_level] = -1;
		search->current_level++;
		search->current_node = &tmp->left;
	}
	return tmp;
}

/****************************************************************
	scroll_down_right()
	continues down a tree always taking the right branch
****************************************************************/
static struct avlbind *scroll_down_right(struct avlsearch *search)
{
	struct avlbind *tmp;

	if (*search->current_node == NULL)
		return NULL;

	for(;;)
	{
		tmp = *search->current_node;
		if (tmp->right == NULL)
			break;
		search->path_taken[search->current_level] = search->current_node;
		search->dir_taken[search->current_level] = 1;
		search->current_level++;
		search->current_node = &tmp->right;
	}
	return tmp;
}

/****************************************************************
	avl_get_first()
	initializes a search structure for the first (smallest)
	element in the tree
****************************************************************/
struct avlbind *avl_get_first(struct avltree *tree, struct avlsearch *search)
{
	search->current_level = 0;
	search->current_node = &tree->root;
	return scroll_down_left(search);
}

/****************************************************************
	avl_get_last()
	initializes a search structure for the last (largest)
	element in the tree
****************************************************************/
struct avlbind *avl_get_last(struct avltree *tree, struct avlsearch *search)
{
	search->current_level = 0;
	search->current_node = &tree->root;
	return scroll_down_right(search);
}

/****************************************************************
	walk_upstairs()
	move up the tree, continuing until the specified direction
	is reached, or the root is reached
****************************************************************/
static struct avlbind *walk_upstairs(struct avlsearch *search, int dir)
{
	for(;;)
	{
		if (search->current_level == 0)
		{
			search->current_node = NULL;
			return NULL;
		}
		search->current_level--;
		if (search->dir_taken[search->current_level] == dir)
		{
			search->current_node = search->path_taken[search->current_level];
			return *search->current_node;
		}
	}
}

/****************************************************************
	avl_get_next()
	single step through a search structure for the next (larger)
	element in the tree
****************************************************************/
struct avlbind *avl_get_next(struct avlsearch *search)
{
	if (*search->current_node == NULL)
		return NULL;

	/* if current position has a right subtree, traverse it */
	if ((*search->current_node)->right)
	{
		search->path_taken[search->current_level] = search->current_node;
		search->dir_taken[search->current_level] = 1;
		search->current_level++;
		search->current_node = &(*search->current_node)->right;
		return scroll_down_left(search);
	}

	/* back upstairs until coming up from left */
	return walk_upstairs(search, -1);
}

/****************************************************************
	avl_get_prev()
	single step through a search structure for the previous (smaller)
	element in the tree
****************************************************************/
struct avlbind *avl_get_prev(struct avlsearch *search)
{
	if (*search->current_node == NULL)
		return NULL;

	/* if current position has a left subtree, traverse it */
	if ((*search->current_node)->left)
	{
		search->path_taken[search->current_level] = search->current_node;
		search->dir_taken[search->current_level] = -1;
		search->current_level++;
		search->current_node = &(*search->current_node)->left;
		return scroll_down_right(search);
	}

	/* back upstairs until coming up from right */
	return walk_upstairs(search, 1);
}

/****************************************************************
	avl_search()
	search the tree for a matching element. If not found, the
	insertion point is traced in the search structure.
****************************************************************/
static struct avlbind *avl_search(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind *tmp;
	int cmp;

	search->current_level = 0;
	search->current_node = &tree->root;

	while ((tmp=*search->current_node) != NULL)
	{
		cmp = (*tree->compare_key_tree)(tree, tmp);
		if (cmp < 0)
		{
			search->path_taken[search->current_level] = search->current_node;
			search->dir_taken[search->current_level] = -1;
			search->current_node = &tmp->left;
		}
		else if (cmp > 0)
		{
			search->path_taken[search->current_level] = search->current_node;
			search->dir_taken[search->current_level] = 1;
			search->current_node = &tmp->right;
		}
		else
			break;
		search->current_level++;
	}
	return *search->current_node;
}

/****************************************************************
	avl_get_less()
	search a tree for the largest element less than the compare
	value
****************************************************************/
struct avlbind *avl_get_less(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind *tmp;

	tmp = avl_search(tree, search);
	if (tmp)
		return avl_get_prev(search);

	/* back upstairs until coming up from right */
	return walk_upstairs(search, 1);
}

/****************************************************************
	avl_get_less_equal()
	search a tree for the largest element less than or equal to
	the compare	value
****************************************************************/
struct avlbind *avl_get_less_equal(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind *tmp;

	tmp = avl_search(tree, search);
	if (tmp)
		return tmp;

	/* back upstairs until coming up from right */
	return walk_upstairs(search, 1);
}

/****************************************************************
	avl_get_greater()
	search a tree for the smallest element greater than the 
	compare	value
****************************************************************/
struct avlbind *avl_get_greater(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind *tmp;

	tmp = avl_search(tree, search);
	if (tmp)
		return avl_get_next(search);

	/* back upstairs until coming up from left */
	return walk_upstairs(search, -1);
}

/****************************************************************
	avl_get_greater_equal()
	search a tree for the smallest element greater than or equal
	to the compare	value
****************************************************************/
struct avlbind *avl_get_greater_equal(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind *tmp;

	tmp = avl_search(tree, search);
	if (tmp)
		return tmp;

	/* back upstairs until coming up from left */
	return walk_upstairs(search, -1);
}

/****************************************************************
	avl_insert()
		inserts a node into the tree
****************************************************************/
struct avlbind *avl_insert(struct avltree *tree, struct avlbind *node)
{
	struct avlbind *tmp, *p3, *p4, **Pivot;
	struct avlsearch search;

	tmp = avl_search(tree, &search);
	if (tmp != NULL)
		return tmp;				/* no repeats allowed */

	node->balance = 0;
	node->left = node->right = NULL;

	/* Insert it into the tree */
	*search.current_node = node;
	tree->num_nodes++;

	/* Walk back up */
	while(search.current_level--)
	{
		tmp = *(Pivot = search.path_taken[search.current_level]);
		if (search.dir_taken[search.current_level] == 1)
		{
			/* coming up from right */
			if (tmp->balance != 1)
			{	/* if node is -1, set balance to 0 and stop */
				if (++tmp->balance == 0)
					return node;
				/* if node is 0, set balance to 1 and continue */
				continue;
			}

			/* Same direction, single rotate */
			if (search.dir_taken[search.current_level+1] == 1)
			{
				p3 = tmp->right;
				tmp->right = p3->left;
				p3->left = tmp;
				tmp->balance = 0;
				p3->balance = 0;
				*Pivot = p3;
				return node;
			}
			/* Need to do a double rotation */
			p3 = tmp->right;
			p4 = p3->left;
			if (p4->balance == 1)
			{
				p3->balance = 0;
				tmp->balance = -1;
			}
			else if (p4->balance == 0)
			{
				p3->balance = 0;
				tmp->balance = 0;
			}
			else
			{
				p3->balance = 1;
				tmp->balance = 0;
			}
			p4->balance = 0;
			tmp->right = p4->left;
			p3->left = p4->right;
			p4->left = tmp;
			p4->right = p3;
			*Pivot = p4;
			return node;
		}
		else
		{
			/* coming up from left */
			if (tmp->balance != -1)
			{
				/* if node is 1, set balance to 0 and stop */
				if (--tmp->balance == 0)
					return node;
				/* if node is 0, set balance to -1 and continue */
				continue;
			}
			/* Same direction, single rotate */
			if (search.dir_taken[search.current_level+1] == -1)
			{
				p3 = tmp->left;
				tmp->left = p3->right;
				p3->right = tmp;
				tmp->balance = 0;
				p3->balance = 0;
				*Pivot = p3;
				return node;
			}
			/* Need to do a double rotation */
			p3 = tmp->left;
			p4 = p3->right;
			if (p4->balance == -1)
			{
				p3->balance = 0;
				tmp->balance = 1;
			}
			else if (p4->balance == 0)
			{
				p3->balance = 0;
				tmp->balance = 0;
			}
			else
			{
				p3->balance = -1;
				tmp->balance = 0;
			}
			p4->balance = 0;
			tmp->left = p4->right;
			p3->right = p4->left;
			p4->right = tmp;
			p4->left = p3;
			*Pivot = p4;
			return node;
		}
	}
	return node;
}

/****************************************************************
	avl_delete_current()
		removes the current node from the tree
		returns the freed binding
****************************************************************/
struct avlbind *avl_delete_current(struct avltree *tree, struct avlsearch *search)
{
	struct avlbind **Nptr;
	struct avlbind *tmp, *p2, *p3, *p4, *freed_node;
	struct avlbind *swap, tmpswap;
	int dir;
	int found_level;

	/* bring the found node down to the bottom of the tree */
	Nptr = search->current_node;
	tmp = *Nptr;
	for(;;)
	{
		DBG_ASSERT(tmp == *Nptr);
		found_level = search->current_level;

		if (tmp->left) /* Didn't get to the bottom */
		{
			/* Save the path */
			search->dir_taken[search->current_level] = -1;
			search->path_taken[search->current_level++] = Nptr;
			Nptr = &tmp->left;

			/* if there is no right subtree, swap adjacent levels */
			if ((swap=*Nptr)->right == NULL)
			{
				Nptr = &swap->left;
			}
			else
			{
				/* otherwise go all the way down the right and swap */
				while ((swap=*Nptr)->right)
				{
					search->dir_taken[search->current_level] = 1;
					search->path_taken[search->current_level++] = Nptr;
					Nptr = &swap->right;
				}
			}

			/* swap contents of nodes */
			tmpswap = *swap;
			*swap = *tmp;
			*tmp = tmpswap;

			/* fix pointer to found node */
			*Nptr = tmp;
			*search->path_taken[found_level] = swap;
			search->path_taken[found_level+1] = &swap->left;
		}
		else if (tmp->right)
		{
			/* Save the path */
			search->dir_taken[search->current_level] = 1;
			search->path_taken[search->current_level++] = Nptr;
			Nptr = &tmp->right;

			/* if there is no left subtree, swap adjacent levels */
			if ((swap=*Nptr)->left == NULL)
			{
				Nptr = &swap->right;
				/* swap contents of nodes */
			}
			else
			{
				/* otherwise go all the way down the left and swap */
				while ((swap=*Nptr)->left)
				{
					search->dir_taken[search->current_level] = -1;
					search->path_taken[search->current_level++] = Nptr;
					Nptr = &swap->left;
				}
			}

			/* swap contents of nodes */
			tmpswap = *swap;
			*swap = *tmp;
			*tmp = tmpswap;

			/* fix pointer to found node */
			*Nptr = tmp;
			*search->path_taken[found_level] = swap;
			search->path_taken[found_level+1] = &swap->right;
		}
		else
			break;
		DBG_ASSERT(tmp);
		DBG_ASSERT(*Nptr);
	}
	DBG_ASSERT(tmp);
	DBG_ASSERT(tmp->balance == 0);
	DBG_ASSERT(tmp->left == NULL);
	DBG_ASSERT(tmp->right == NULL);

	/* Delete the node, this is where the elegance of the double pointer comes
	 in, no special case for root or left or right */
	freed_node = *Nptr;
	*Nptr = NULL;
	tree->num_nodes--;

	for(;;)
	{
		if (search->current_level-- == 0)
		{
			/* Reached the top */
			return freed_node;
		}

		tmp = *(Nptr = search->path_taken[search->current_level]);
		dir = search->dir_taken[search->current_level];
		if (tmp->balance == 0)
		{
			tmp->balance -= dir;
			return freed_node;
		}
		if (tmp->balance == dir)
			tmp->balance = 0;
		else
		{
			if (dir == 1)
			{
				/* Coming up from right, do clockwise rotations */
				DBG_ASSERT(tmp->left);
				p2 = tmp;
				p3 = tmp->left;
				if (p3->balance != 1)
				{
					/* Do single rotation */
					p2->left = p3->right;
					p3->right = p2;
					p2->balance -= p3->balance;
					p3->balance++;
					*Nptr = p3;		/* This points the parent of p2 to p3 */
					if (p3->balance != 0)
					{
						/* If the tree has not been shortened */
						return freed_node;
					}
				}
				else
				{
					/* Do double rotation */
					p4 = p3->right;
					p2->left = p4->right;
					p3->right = p4->left;
					p4->left = p3;
					p4->right = p2;
					if (p4->balance == 0)
					{
						p3->balance = 0;
						p2->balance = 0;
					}
					else if (p4->balance == 1)
					{
						p3->balance = -1;
						p2->balance = 0;
					}
					else
					{
						p3->balance = 0;
						p2->balance = 1;
					}
					p4->balance = 0;
					*Nptr = p4;		/* This points the parent of p2 to p4 */
				}
			}
			else
			{
				/* Coming up from left, do counter-clockwise rotations */
				DBG_ASSERT(tmp->right);
				p2 = tmp;
				p3 = tmp->right;
				if (p3->balance != -1)
				{
					/* Do single rotation */
					p2->right = p3->left;
					p3->left = p2;
					if (p3->balance == 0)
						p2->balance = 1;
					else
					p2->balance -= p3->balance;
					p3->balance--;
					*Nptr = p3;		/* This points the parent of p2 to p3 */
					if (p3->balance != 0)
					{
						/* If the tree has not been shortened */
						return freed_node;
					}
				}
				else
				{
					/* Do double rotation */
					p4 = p3->left;
		
					p2->right = p4->left;
					p3->left = p4->right;
					p4->right = p3;
					p4->left = p2;
					if (p4->balance == 0)
					{
						p3->balance = 0;
						p2->balance = 0;
					}
					else if (p4->balance == 1)
					{
						p3->balance = 0;
						p2->balance = -1;
					}
					else
					{
						p2->balance = 0;
						p3->balance = 1;
					}
					p4->balance = 0;
					*Nptr = p4;		/* This points the parent of p2 to p4 */
				}
			}
		}
	}
}

/****************************************************************
	avl_delete()
		searches and removes node from the tree
		returns the freed binding
****************************************************************/
struct avlbind *avl_delete(struct avltree *tree)
{
	struct avlsearch search;
	struct avlbind *tmp;

	/* Find the node in the tree */
	tmp = avl_search(tree, &search);
	if (tmp == NULL)	/* Not found */
		return NULL;
	return avl_delete_current(tree, &search);
}

#if 0
/* Sample code */

struct mytree {
	avltree tree;
	int key;
};

struct mynode {
	avlbind node;
	int key;
};

static int compare(avltree *tree, avlbind *node) {
	int lhs = ((mytree*)tree)->key;
	int rhs = ((mynode*)node)->key;
	return rhs > lhs ? -1 : rhs < lhs ? 1 : 0;
}

void avl_sample(void)
{
	mytree avl;
	memset(&avl, 0, sizeof(avl));
	avl.tree.compare_key_tree = compare;
	avlsearch search;
	avl_get_first(&avl.tree, &search);
	for (unsigned i=0; i<100; i++)
	{
		mynode *newnode = (mynode*)malloc(sizeof(mynode));
		newnode->key = avl.key = rand();
		avl_insert(&avl.tree, &newnode->node);
	}
	avl_get_first(&avl.tree, &search);
	while (search.current_node)
	{
		mynode *cur = (mynode *)(*search.current_node);
		printf("%d\n", key);
		avl_get_next(&search);
	}
}
#endif
