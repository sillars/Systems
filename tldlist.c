#include "tldlist.h"
#include "date.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

struct tldlist{
	struct tldnode *root;
	struct date *begin;
	struct date *end;
	long add_counter;
};
struct tldnode{
	//should these be pointers?
	char *hostname;
	int balance;
	int height;
	struct tldnode *l;
	struct tldnode *r;
	struct tldnode *parent;
	//could be issue that this isn't initialised
	long numvisits;

};

struct tlditerator{
	struct queue *q;
};

/*
* REBALANCING
*/

int height(TLDNode *n){
	//need to change all instances of -> height now
	if (n == NULL){
		return -1;
	}
	return n->height;
}


//helper function - calc max of 2 ints
int max(int a, int b){
	if (a > b){
		return a;
	}
	else{
	return b;
	}
}

void reheight(TLDNode *n){
	if (n != NULL){
		n->height = 1 + max(height(n->l), height(n->r)); 
	}
}




void setBalance(TLDNode *n){
	reheight(n);
	n->balance = height(n->r) - height(n->l);
}



TLDNode *rotateLeft(TLDNode *a){
	struct tldnode *b = a->r;
	b->parent = a->parent;

	a->r = b->l;

	if (a->r != NULL){
		a->r->parent = a;
	}

	b->l = a;
	a->parent = b;

	if (b->parent != NULL){
		if (b->parent->r == a){
			b->parent->r = b;
		} else {
			b->parent->l = b;
		}
	}

	setBalance(a);
	setBalance(b);
	//do i want to return a pointer here?
	return(b);
}

TLDNode *rotateRight(TLDNode *a){
	struct tldnode *b = a->l;
	b->parent = a->parent;

	a->l = b->r;

	if (a->l != NULL){
		a->l->parent = a;
	}

	b->r = a;
	a->parent = b;

	if (b->parent != NULL){
		if (b->parent->r == a){
			b->parent->r = b;
		} else {
			b->parent->l = b;
		}
	}

	setBalance(a);
	setBalance(b);
	return(b);
}

TLDNode *rotateLeftThenRight(TLDNode *n){
	n->l = rotateLeft(n->l);
	return rotateRight(n);
}

TLDNode *rotateRightThenLeft(TLDNode *n){
	n->r = rotateRight(n->r);
	return rotateLeft(n);
}

void rebalance(TLDNode *n, TLDList *tldlist){
	setBalance(n);

	if (n->balance == -2){
		if(height(n->l->l) >= height(n->l->r)){
			n = rotateRight(n);
		} else{
			n = rotateLeftThenRight(n);
		}
	} else if (n->balance == 2) {
		if(height(n->r->r) >= height(n->r->l)){
			n = rotateLeft(n);
		} else {
			n = rotateRightThenLeft(n);
		}
	}

	if (n->parent != NULL){
		rebalance(n->parent, tldlist);
	} else {
		tldlist->root = n;
	}
}

/*
* END OF REBALANCING
*/

TLDList *tldlist_create(Date *begin, Date *end){
	struct tldlist *list;
	if ((list = (struct tldlist *)malloc(sizeof(struct tldlist))) != NULL){
		list->root = NULL;
		list->begin = begin;
		list->end = end;
		list->add_counter = 0;
	}
	return list;
}
/*
 * tldlist_destroy destroys the list structure in `tld'
 *
 * all heap allocated storage associated with the list is returned to the heap
 */

//recursion to destroy tldnode
void tldnode_rec_destroy(TLDNode *n){
	if (n->l != NULL){
		tldnode_rec_destroy(n->l);
	}
	if (n->r != NULL){
		tldnode_rec_destroy(n->r);
	}
	//DESTROY THE NODE!!!
	//free the hostname
	free(n);
}

void tldlist_destroy(TLDList *tld){
	tldnode_rec_destroy(tld->root);
	//date_destroy(tld->begin);
	//date_destroy(tld->end);
	//DESTROY THE DATE IN TLDMONITOR
	free(tld);
}


//helper function - return tld name
char *tldname(char *hostname){
	char *tldn = (char *)malloc(sizeof(char)*4);
	int i  = strlen(hostname)-1;
	int j = 2; //index from the end of tld
	char curr = hostname[i];
	int runtimes = 0;
	while (curr != '.'){
		
		tldn[j] = tolower(hostname[i]);

		i --;
		j --;
		runtimes ++;
		curr = hostname[i]; //moving this to top removes free error
		
	}
	//printf("%druntimes\n ", runtimes);
	//printf("%d", runtimes == 3 || runtimes == 4);
	if (runtimes == 2){  
		char *two_tldn = (char *)malloc(sizeof(char) *3);
		
		//TEST
		//printf("running");
		//END
		if (two_tldn != NULL){
			two_tldn[0] = tldn[1];
			two_tldn[1] = tldn[2];
			two_tldn[2] = '\0';
		}
		free(tldn);
		return two_tldn;
	}
	if(runtimes == 3){
		tldn[3] = '\0';
		return tldn;
	}
	return NULL;
	
}

//helper function to create a node
TLDNode *tldnode_create(TLDNode *parent, char *hostname){
	struct tldnode *node;
	if ((node = (struct tldnode *)malloc(sizeof(struct tldnode))) != NULL){
		char *tldn = tldname(hostname);
		node->hostname = tldn;
		//free(tldn);
		//NEED TO RE ADD THIS?
		//node->height = 1;
		node->parent = parent;
		node->numvisits = 1;
	}
	return	node;
}
//return 1/0 if successful/unsuccessful
//int tldlist_add(TLDList *tld, char *hostname, Date *d){ return 0;}

int tldlist_add(TLDList *tld, char *hostname, Date *d){

	if ((date_compare(d, tld->begin)>=0) && (date_compare(tld->end, d)>=0)){	
		//FIX MALLOCS FURTHER DOWN
		//if there is no root
		if(tld->root == NULL){
			tld->root = tldnode_create(NULL, hostname);
			tld->add_counter ++;
			//maybe want to return null if create failed
			return 1;
		}

		//if there is already a root
		struct tldnode *n;
		n = tld->root;
		struct tldnode *parent;
		while(true){
			parent = n;
			int strcomp = strcmp(tldname(hostname), n->hostname);
			bool goLeft;
			if (strcomp <0){
			//go left, hostname comes before n->hostname
				n = n->l;
				goLeft = true;
			} else if(strcomp > 0){
			//go right, hostname comes after n->hostname
				n = n->r;
				goLeft = false;
			}
			else{
			//the strings are the same
				//printf("adding to numvisits");
				n->numvisits ++;
				tld->add_counter ++;
				return 1;
			}

			if (n == NULL){
				struct tldnode *p;
				//if you went left
				if(goLeft){
					p = tldnode_create(parent, hostname);
					parent->l = p;
					p->parent = parent;
				} else {
					p = tldnode_create(parent, hostname);
					parent->r = p;
					p->parent = parent;
				}
				rebalance(parent, tld);
					//free(p);
				break;
				}
			}
		tld->add_counter ++;
		return 1;
		}
	return 0;
}

/*
 * tldlist_count returns the number of successful tldlist_add() calls since
 * the creation of the TLDList
 */
long tldlist_count(TLDList *tld){
	return tld->add_counter;
}

/*
 * tldlist_iter_create creates an iterator over the TLDList; returns a pointer
 * to the iterator if successful, NULL if not
 */
//ITERATOR THINGS
struct q_elt{
	struct q_elt *next;
	TLDNode *node;
};

struct queue{
	struct q_elt *head;
	struct q_elt *tail;
};
typedef struct queue Queue;


//create a queue for the iterator
Queue *q_create(void){
	struct queue *q;
	
	if ((q = (struct queue *)malloc(sizeof(struct queue))) != NULL){
		q->head = NULL;
		q->tail = NULL;
	}
	return q;
}

int q_add(Queue *q, TLDNode *n){
	struct q_elt *p;

	p = (struct q_elt *)malloc(sizeof(struct q_elt));
	if (p != NULL){
		p->node = n;
		p->next = NULL;
		if (q->head == NULL){
			q->head = p;
		}else{
			q->tail->next = p;
		}
		q->tail = p;
		return 1;
	}
	return 0;
}

TLDNode *q_rm(Queue *q){
	struct q_elt *p;
	TLDNode *n;

	if (q->head == NULL){
		return NULL;
	}
	p = q->head;
	q->head = p->next;
	if (q->head == NULL){
		q->tail = NULL;
	}
	n = p->node;
	free(p);
	return n;
}
//END OF QUEUE CODE

void recursive_add(Queue *q, TLDNode *n){
	if (n->l != NULL){
		recursive_add(q, n->l);
	}
	if (n->r != NULL){
		recursive_add(q, n->r);
	}
	q_add(q, n);
}

TLDIterator *tldlist_iter_create(TLDList *tld){
	struct tlditerator *p;
	if ((p = (struct tlditerator *)malloc(sizeof(struct tlditerator))) != NULL){
		struct queue *q = q_create();
		recursive_add(q, tld->root);
		p->q = q;
	}
	return p;
}


TLDNode *tldlist_iter_next(TLDIterator *iter){
	struct queue *q = iter->q;
	TLDNode *n = q_rm(q);
	return n;
}

void tldlist_iter_destroy(TLDIterator *iter){
	while(iter->q->head != NULL){
		q_rm(iter->q);
	}
}

char *tldnode_tldname(TLDNode *node){
	return node->hostname;
}


long tldnode_count(TLDNode *node){
	return node->numvisits;
}
