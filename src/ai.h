#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"
#include <math.h>

void initialize_ai();

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, int *generated, int *expanded);
node_t *createNode(node_t *parent, move_t move);
void propagateMaxPriority(node_t *node);
void propagateAveragePriority(node_t *node);
void ai_close();

#endif
