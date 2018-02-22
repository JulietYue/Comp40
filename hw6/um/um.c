/*
 * Martin Gao & Juliet Yue
 * date: 11/27/17
 *
 * Implementation of Um module
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "um.h"
#include "mem.h"
#include "assert.h"
#include "segments.h"
#include "bitpack.h"

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

typedef uint32_t word;

typedef uint32_t reg_val;

typedef uint32_t Um_instruction;

typedef enum Um_opcode {
              CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
              NAND, HALT, MAP, UNMAP, OUT, IN, LOADP, LV
} Um_opcode;

#define NUM_REGS 8

struct Um {
                Segments_T segments;
                reg_val registers[NUM_REGS];
                reg_val pc;
};

#define OPSIZE 4
#define INSTR_SIZE 32
#define REGSIZE 3
#define VALSIZE 25

static uint32_t get_reg(Um machine, Um_register r);
static void set_reg(Um machine, Um_register r, word val);

static Um_instruction get_next_instr(Um machine);
static void set_pc(Um machine, uint32_t val);

typedef struct Instr_regs {
        Um_register ra, rb, rc;
}Instr_regs;

static void conditional_move(Um machine, Instr_regs regs);
static void segmented_load(Um machine, Instr_regs regs);
static void segmented_store(Um machine, Instr_regs regs);
static void addition(Um machine, Instr_regs regs);
static void multiplication(Um machine, Instr_regs regs);
static void division(Um machine, Instr_regs regs);
static void bitwise_nand(Um machine, Instr_regs regs);
/* halt */
static void map_segment(Um machine, Instr_regs regs);
static void unmap_segment(Um machine, Instr_regs regs);
static void output(Um machine, Instr_regs regs);
static void input(Um machine, Instr_regs regs);
static void load_program(Um machine, Instr_regs regs);
static void load_value(Um machine, Um_register ra, uint32_t val);

typedef void (*gen_instr) (Um, Instr_regs);

const gen_instr INSTRUCTIONS[] = {
        conditional_move, 
        segmented_load, 
        segmented_store,
        addition, 
        multiplication, 
        division,
        bitwise_nand, 
        NULL,        /* halt */
        map_segment, 
        unmap_segment,
        output, 
        input, 
        load_program
};


/* switch statement to judge what instr it is */
static bool run_instr(Um machine, Um_instruction to_run);

static uint32_t get_reg(Um machine, Um_register r)
{
        assert(machine);
        return machine->registers[r];
}

static void set_reg(Um machine, Um_register r, word val)
{
        assert(machine);
        machine->registers[r] = val;
}

static Um_instruction get_next_instr(Um machine)
{
        assert(machine);
        Um_instruction *program = Segments_get_mem(machine->segments, 0);
        return program[(machine->pc)++];
}

static void set_pc(Um machine, uint32_t val)
{
        assert(machine);
        machine->pc = val;
}

static bool run_instr(Um machine, Um_instruction to_run)
{
        Um_opcode instr = Bitpack_getu(to_run, OPSIZE, INSTR_SIZE - OPSIZE);
        
        if (instr == HALT) {
                return false;
        }
        if (instr == LV) {
                Um_register ra = Bitpack_getu(to_run, REGSIZE, 
                     INSTR_SIZE - OPSIZE - REGSIZE);
                uint32_t val = Bitpack_getu(to_run, VALSIZE, 0);

                load_value(machine, ra, val);

                return true;
        }

        Instr_regs regs;

        regs.ra = Bitpack_getu(to_run, REGSIZE, REGSIZE * 2);
        regs.rb = Bitpack_getu(to_run, REGSIZE, REGSIZE * 1);
        regs.rc = Bitpack_getu(to_run, REGSIZE, REGSIZE * 0);

        INSTRUCTIONS[instr](machine, regs);
        return true;
}

static void conditional_move(Um machine, Instr_regs regs)
{
        assert(machine);
        if(get_reg(machine, regs.rc) != 0)
                set_reg(machine, regs.ra, get_reg(machine, regs.rb));
}

static void segmented_load(Um machine, Instr_regs regs)
{
        assert(machine);
        word *seg = Segments_get_mem(machine->segments, 
                                     get_reg(machine, regs.rb));
        set_reg(machine, regs.ra, seg[get_reg(machine, regs.rc)]);
}

static void segmented_store(Um machine, Instr_regs regs)
{
        assert(machine);
        word *seg = Segments_get_mem(machine->segments, 
                                     get_reg(machine, regs.ra));
        seg[get_reg(machine, regs.rb)] = get_reg(machine, regs.rc);
}

static void addition(Um machine, Instr_regs regs)
{
        assert(machine);
        word rb = get_reg(machine, regs.rb);
        word rc = get_reg(machine, regs.rc);
        set_reg(machine, regs.ra, rb + rc);
}

static void multiplication(Um machine, Instr_regs regs)
{
        assert(machine);
        word rb = get_reg(machine, regs.rb);
        word rc = get_reg(machine, regs.rc);
        set_reg(machine, regs.ra, rb * rc);
}
        
static void division(Um machine, Instr_regs regs)
{
        assert(machine);
        word rb = get_reg(machine, regs.rb);
        word rc = get_reg(machine, regs.rc);
        set_reg(machine, regs.ra, rb / rc);
}

static void bitwise_nand(Um machine, Instr_regs regs)
{
        assert(machine);
        word rb = get_reg(machine, regs.rb);
        word rc = get_reg(machine, regs.rc);
        set_reg(machine, regs.ra, ~(rb & rc));
}

static void map_segment(Um machine, Instr_regs regs)
{
        assert(machine);
        seg_id new_id = Segments_map(machine->segments, 
                        get_reg(machine, regs.rc));
        set_reg(machine, regs.rb, new_id);
}

static void unmap_segment(Um machine, Instr_regs regs)
{
        assert(machine);
        Segments_unmap(machine->segments, get_reg(machine, regs.rc));
}

static void output(Um machine, Instr_regs regs)
{
        assert(machine);
        putchar(get_reg(machine, regs.rc));
}

static void input(Um machine, Instr_regs regs)
{
        assert(machine);
        int c = getchar();
        if (c == EOF) {
                set_reg(machine, regs.rc, ~0);
                return;
        }
        set_reg(machine, regs.rc, (unsigned char) c);
}

static void load_program(Um machine, Instr_regs regs)
{
        assert(machine); 
        set_pc(machine, get_reg(machine, regs.rc));
        seg_id origin_id = get_reg(machine, regs.rb);
        if (origin_id == 0)
                return;
        Segments_copy(machine->segments, origin_id, 0);
}

static void load_value(Um machine, Um_register ra, uint32_t val)
{
        set_reg(machine, ra, val);
}

Um Um_new(FILE *program)
{
        Um result;
        NEW(result);

        result->segments = Segments_new();

        Segments_read_program(result->segments, program);
        result->pc = 0;

        for (int i = 0; i < NUM_REGS; ++i) {
                result->registers[i] = 0;
        }

        return result;
}

void Um_free(Um *machinep)
{
        assert(machinep && *machinep);
        Segments_free(&((*machinep)->segments));
        FREE(*machinep);
        machinep = NULL;
}

bool run_next(Um machine)
{
       return run_instr(machine, get_next_instr(machine));   
}
