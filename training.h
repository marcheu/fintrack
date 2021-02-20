#ifndef _TRAINING_H_
#define _TRAINING_H_

#include "neural_net.h"
#include "problem.h"

void training_annealing (problem & p, neural_net & n);
void training_adagrad (problem & p, neural_net & n);
void training_adagrad_annealing (problem & p, neural_net & n);
void training_backprop (problem & p, neural_net & n);
void training_backprop_annealing (problem & p, neural_net & n);
void training_prune (problem & p, neural_net & n);

#endif
