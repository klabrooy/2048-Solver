/*
 ============================================================================
 Name        : ai.c
 Author      : Kara La'Brooy 757553
 Description : Artificial intelligence for 2048 game
 ============================================================================
 */
#include <time.h>
#include <stdlib.h>
#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

struct heap h;

void initialize_ai(){
	heap_init(&h);
}

/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */
 int NUMBER_OF_MOVES = 4;

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, int *generated, int *expanded){
	//guard for depth of zero, pick random move
	if (max_depth == 0) {
		move_t best_action = rand() % 4;
		return best_action;
	}

	int i, j;
	int count = 0;
	node_t *newNode0;
	node_t *newNode1;
	node_t *newNode2;
	node_t *newNode3;
	node_t *depthOne0 = NULL;
	node_t *depthOne1 = NULL;
	node_t *depthOne2 = NULL;
	node_t *depthOne3 = NULL;

	//malloc size for the initial start node
	 node_t *node = (node_t*)malloc(sizeof(node_t));

	 //initialize variables
	 node->priority = 0;
	 node->depth = 0;
	 node->num_childs = 0;
	 node->parent = NULL;

	 //copy over tiles on board
	 for (i=0; i < SIZE; i++) {
		 for (j=0; j < SIZE; j++) {
			 node->board[i][j] = board[i][j];
		 }
	 }

	 //initialize the explored array
	 int SIZE_OF_EXPLORED = 50;
	 node_t **explored = (node_t**)malloc(sizeof(node_t*)*SIZE_OF_EXPLORED);

	 heap_push(&h, node);

	 //while there are still nodes in the heap
	 while(h.count != 0) {
		 //pop node off the heap
		 node = heap_delete(&h);

		 //increment the number of expanded nodes
 	 		*expanded += 1;

			//if more space is required in the array, realloc
		 if (count == SIZE_OF_EXPLORED) {
			 SIZE_OF_EXPLORED = SIZE_OF_EXPLORED*2;
			 explored = realloc(explored, sizeof(node_t*)*SIZE_OF_EXPLORED);
		 }

		 //this popped node is being explored
		 explored[count] = node;
		 count++;


		 if (node->depth < max_depth) {
			 //for each board action, create a node for each move
			 //if it is a node of depth 1, store it for ease later
				newNode0 = createNode(node, 0);
				if (newNode0 != NULL && newNode0->depth == 1) {
					depthOne0 = newNode0;
				}
			 	newNode1 = createNode(node, 1);
				if (newNode1 != NULL && newNode1->depth == 1) {
					depthOne1 = newNode1;
				}
				newNode2 = createNode(node, 2);
				if (newNode2 != NULL && newNode2->depth == 1) {
					depthOne2 = newNode2;
				}
				newNode3 = createNode(node, 3);
				if (newNode3 != NULL && newNode3->depth == 1) {
					depthOne3 = newNode3;
				}
				//already checks if the move was valid in createNode will return NULL if failed.
				//four new node generated, increment
				*generated += 4;

				//if the new nodes are not null, push them in the heap, and propagate the score
				if(newNode0 != NULL) {
					heap_push(&h, newNode0);
					if (propagation == max) {
						propagateMaxPriority(newNode0);
					}
					else if (propagation == avg) {
						propagateAveragePriority(newNode0);
					}
				}
				if(newNode1 != NULL) {
					heap_push(&h, newNode1);
					if (propagation == max) {
						propagateMaxPriority(newNode1);
					}
					else if (propagation == avg) {
						propagateAveragePriority(newNode1);
					}
				}
				if(newNode2 != NULL) {
					heap_push(&h, newNode2);
					if (propagation == max) {
						propagateMaxPriority(newNode2);
					}
					else if (propagation == avg) {
						propagateAveragePriority(newNode2);
					}
				}
				if(newNode3 != NULL) {
					heap_push(&h, newNode3);
					if (propagation == max) {
						propagateMaxPriority(newNode3);
					}
					else if (propagation == avg) {
						propagateAveragePriority(newNode3);
					}
				}
		 }
	 }

	 //create an array for finding the best score
	 node_t **depthOne = (node_t**)malloc(sizeof(node_t*)*NUMBER_OF_MOVES);
	 depthOne[0] = depthOne0;
	 depthOne[1] = depthOne1;
	 depthOne[2] = depthOne2;
	 depthOne[3] = depthOne3;

	 move_t bestMove = 0;
	 int bestPriority = 0;

	 //find the next move with the best score
	 for (i=0; i < NUMBER_OF_MOVES; i++) {
		 if (depthOne[i] != NULL) {
			 if (depthOne[i]->priority >= bestPriority) {
				 bestMove = depthOne[i]->move;
				 bestPriority = depthOne[i]->priority;
			 }
		 }
	 }

	 //free memory
	 for (i=0; i < count; i++) {
		 free(explored[i]);
	 }
	 free(explored);

	 free(depthOne);

	 emptyPQ(&h);

	return bestMove;
}

/*Creates a newNode based on a parent node and the desired move*/
node_t *createNode(node_t *parent, move_t move) {
	int i, j;
	//make space for the node
	node_t *newNode = (node_t*)malloc(sizeof(node_t));
	newNode->priority = parent->priority;
	newNode->depth = parent->depth + 1;
	newNode->num_childs = 0;
	newNode->move = move;
	newNode->parent = parent;
	parent->num_childs++;

	//fill in the board array
	for (i=0; i < SIZE; i++) {
		for (j=0; j < SIZE; j++) {
			newNode->board[i][j] = parent->board[i][j];
		}
	}

	if (execute_move_t(newNode->board, &newNode->priority, move)) {
		//do nothing, everything updated
	}
	else {
		//the board has not changed
		//free the node, delete
		free(newNode);
		newNode = NULL;
	}

	return newNode;
}

/*First action is updated to reflect the average score*/
void propagateAveragePriority(node_t *node) {
	int score = node->priority;
	node = node->parent;

	while (node->depth > 1) {
		node->priority = node->priority + score/node->num_childs - node->priority/node->num_childs;
		node = node->parent;
	}

}

/*First action is updated to reflect the maximum score*/
void propagateMaxPriority(node_t *node) {
	while (node->depth > 1) {
		if(node->priority > node->parent->priority) {
			node->parent->priority = node->priority;
		}
		node = node -> parent;
	}
}

/*Free h.heaparr*/
void ai_close() {
	free(h.heaparr);
}
