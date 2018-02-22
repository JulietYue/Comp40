/*
 * Martin Gao & Juliet Yue
 * date: 11/13/17
 *
 * main function for um
 *
 */

#include "um.h"
#include "assert.h"

int main(int argc, char *argv[]) 
{
        assert(argc == 2);
        FILE *program = fopen(argv[1], "r");
        assert(program);
        Um machine = Um_new(program);
        while (run_next(machine)){
                ;
        }
        Um_free(&machine);
        fclose(program);
}
