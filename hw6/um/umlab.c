/*
 * umlab.c
 * 
 * The functions defined in this lab should be linked against umlabwrite.c
 * to produce a unit test writing program. Any additional functions and unit
 * tests written for the lab go here.
 */

#include <stdint.h>
#include <stdio.h>

#include <assert.h>
#include <seq.h>

#include "bitpack.h"

#define OPSIZE 4
#define INSTR_SIZE 32
#define REGSIZE 3
#define VALSIZE 25

#define BYTE 8

typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MULT, DIV,
        NAND, HALT, MAP, UNMAP, OUT, IN, LOADP, LV
} Um_opcode;

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, Um_register ra, Um_register rb,
                              Um_register rc)
{
        uint64_t result = 0;
        result = Bitpack_newu(result, OPSIZE, INSTR_SIZE - OPSIZE, op);
        result = Bitpack_newu(result, REGSIZE, 2 * REGSIZE, ra);
        result = Bitpack_newu(result, REGSIZE, 1 * REGSIZE, rb);
        result = Bitpack_newu(result, REGSIZE, 0 * REGSIZE, rc);
        return result;
}

Um_instruction loadval(Um_register ra, unsigned val)
{
        uint64_t result = 0;
        result = Bitpack_newu(result, OPSIZE, INSTR_SIZE - OPSIZE, LV);
        result = Bitpack_newu(result, REGSIZE, INSTR_SIZE - OPSIZE - REGSIZE, ra);
        result = Bitpack_newu(result, VALSIZE, 0, val);
        return result;
}

/* Wrapper functions for each of the instructions */

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

Um_instruction output(Um_register c)
{
        return three_register(OUT, 0, 0, c);
}

/* Functions for working with streams */

static inline void emit(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

extern void Um_write_sequence(FILE *output, Seq_T stream)
{
        Um_instruction instruct;
        int len = Seq_length(stream);
        for (int i = 0; i < len; ++i) {
                instruct = (Um_instruction)(uintptr_t)Seq_get(stream, i);
                for (int j = INSTR_SIZE / BYTE - 1; j >= 0; --j) {
                        fputc(Bitpack_getu(instruct, BYTE, BYTE * j), output);
                }
        }
}

/* Unit tests for the UM */

void emit_halt_test(Seq_T stream)
{
        emit(stream, halt());
}

void emit_verbose_halt_test(Seq_T stream)
{
        emit(stream, halt());
        emit(stream, loadval(r1, 'B'));
        emit(stream, output(r1));
        emit(stream, loadval(r1, 'a'));
        emit(stream, output(r1));
        emit(stream, loadval(r1, 'd'));
        emit(stream, output(r1));
        emit(stream, loadval(r1, '!'));
        emit(stream, output(r1));
        emit(stream, loadval(r1, '\n'));
        emit(stream, output(r1));
}

void emit_print_six_test(Seq_T stream)
{
        emit(stream, loadval(r1, 48));
        emit(stream, loadval(r2, 6));
        emit(stream, add(r3, r1, r2));
        emit(stream, output(r3));
        emit(stream, halt());
}

void emit_test_div(Seq_T stream)
{
        emit(stream, loadval(r1, 900));
        emit(stream, loadval(r2, 9));
        emit(stream, three_register(DIV, r3, r1, r2));
        emit(stream, output(r3)); /* output is 'd' */
        emit(stream, halt());
}

void emit_test_nand(Seq_T stream)
{
        emit(stream, loadval(r1, 0x1fffffb));
        emit(stream, loadval(r2, 0x1ffffcf));
        emit(stream, three_register(NAND, r3, r1, r2));
        emit(stream, output(r3)); /* output is '4' */
        emit(stream, halt());
}

void emit_test_mult(Seq_T stream)
{
        emit(stream, loadval(r1, 6));
        emit(stream, loadval(r2, 9));
        emit(stream, three_register(MULT, r3, r1, r2));
        emit(stream, output(r3)); /* output is '6' */

        emit(stream, halt());
}

void emit_test_input(Seq_T stream)
{
        emit(stream, three_register(IN, 0, 0, r1));
        emit(stream, output(r1));
        emit(stream, halt());
}

void emit_test_cmov(Seq_T stream)
{
        emit(stream, loadval(r3, 0));
        emit(stream, loadval(r2, 88)); /* this is 'X' */
        emit(stream, loadval(r1, 89)); /* this is 'Y' */
        emit(stream, three_register(CMOV, r1, r2, r3));
        emit(stream, output(r1)); /* output is 'Y' */
        emit(stream, loadval(r3, 1));
        emit(stream, three_register(CMOV, r1, r2, r3));
        emit(stream, output(r1)); /* output is 'X' */
        emit(stream, halt());
}

void emit_map_unmap(Seq_T stream)
{
        emit(stream, loadval(r3, 100));
        emit(stream, loadval(r0, 48));

        for (int i = 0; i < 50; ++i) {
                emit(stream, three_register(MAP, 0, r1, r3));
                emit(stream, three_register(UNMAP, 0, 0, r1));
                emit(stream, add(r2, r1, r0));
                emit(stream, output(r2));
        }
        emit(stream, halt());
}

void emit_time_test(Seq_T stream)
{
        for (int i = 0; i < 100000; ++i)
        {
                emit(stream, loadval(r0, 1));
                emit(stream, three_register(MAP, 0, r1, r3));
                emit(stream, three_register(UNMAP, 0, 0, r1));
                emit(stream, add(r2, r1, r0));
                emit(stream, three_register(CMOV, r1, r2, r3));
        }
        emit(stream, halt());      
}

void emit_map_unmap_sload_sstore(Seq_T stream)
{
        emit(stream, loadval(r3, 18));
        /* r1 = id to 18 word segment */
        emit(stream, three_register(MAP, 0, r1, r3));
        emit(stream, loadval(r2, 98)); /* this is 'b' */
        emit(stream, loadval(r3, 8)); 
        emit(stream, three_register(SSTORE, r1, r3, r2));
        emit(stream, three_register(SLOAD, r5, r1, r3));
        emit(stream, output(r5)); /* output is 'b' */
        
        emit(stream, loadval(r4, 97)); /* this is 'a' */
        emit(stream, loadval(r3, 14)); 
        emit(stream, three_register(SSTORE, r1, r3, r4));
        emit(stream, three_register(SLOAD, r4, r1, r3));
        emit(stream, output(r4)); /* output is 'a' */

        emit(stream, loadval(r3, 8)); 
        emit(stream, three_register(SLOAD, r4, r1, r3));
        emit(stream, output(r4)); /* output is 'b' */

        /*unMAP*/
        emit(stream, three_register(UNMAP, 0, 0, r1));

        emit(stream, halt());
}
