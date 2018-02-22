/*
 * Martin Gao & Juliet Yue
 * date: 11/27/17
 *
 * Interface of Um module
 *
 */

#ifndef UM_H_
#define UM_H_

#include <stdio.h>
#include <stdbool.h>

struct Um;

typedef struct Um *Um;

Um Um_new(FILE *input);
void Um_free(Um *machine);
bool run_next(Um machine);

#endif
