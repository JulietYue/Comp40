/*
 * Martin Gao & Juliet Yue
 * date: 11/13/17
 *
 * segments_test.c
 *
 * Tests for functions of the Segments ADT
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "seq.h"
#include "segments.h"
#include "assert.h"

struct Segments_T {
        Seq_T mapped;
        Seq_T unmapped;
        uint32_t next_id;
};

/* function to help testing */
static void Segments_print(Segments_T to_print)
{
        printf("mapped sequence length: %d\n", 
               Seq_length(to_print->mapped));
        printf("unmapped sequence length: %d\n", 
               Seq_length(to_print->unmapped));
        printf("next_id: %u\n", to_print->next_id);
}

void Segments_new_free_test(FILE *source)
{
        Segments_T segs = Segments_new(); 
        Segments_read_program(segs, source, sizeof(uint32_t));

        /* Expected output
           mapped sequence length: 1
           unmapped sequence length: 0
           next_id: 1
        */
        Segments_print(segs);
        Segments_free(&segs);
}

void map_Segments_unmap_test(FILE *source)
{
        Segments_T segs = Segments_new(); 
        Segments_read_program(segs, source, sizeof(uint32_t));

        /* Expected output
           mapped sequence length: 1
           unmapped sequence length: 0
           next_id: 1
        */
        Segments_print(segs);
        uint32_t new_id = Segments_map(segs, 32);
        /* Expected output
           new mapped segment id: 1 
        */
        printf("new mapped segment id: %u\n", new_id);

        /* Expected output
           mapped sequence length: 2
           unmapped sequence length: 0
           next_id: 2 
        */
        Segments_print(segs);

        Segments_unmap(segs, new_id);

        /* Expected output
           mapped sequence length: 2
           unmapped sequence length: 1
           next_id: 2 
        */
        Segments_print(segs);

        new_id = Segments_map(segs, 32);

        /* Expected output
           new mapped segment id: 1
        */
        printf("new mapped segment id: %u\n", new_id);

         /* Expected output
           mapped sequence length: 2
           unmapped sequence length: 0
           next_id: 2 */
        Segments_print(segs);

        Segments_free(&segs);
}

void get_segment_test(FILE *source)
{
        Segments_T segs = Segments_new(); 
        Segments_read_program(segs, source, sizeof(uint32_t));
        char *buffer = Segments_get_mem(segs, 0);
        strcpy(buffer, "Sample value");

        /* Expected output: 
           segment 0 content: Sample value
        */
        printf("segment 0 content: %s\n", 
               (char *)Segments_get_mem(segs, 0));
        Segments_free(&segs);
}

void copy_test(FILE *source)
{
        Segments_T segs = Segments_new(); 
        Segments_read_program(segs, source, sizeof(uint32_t));

        char *buffer = Segments_get_mem(segs, 0);
        strcpy(buffer, "Sample value");

        /* Expected output: 
           segment 0 content: Sample value
        */
        printf("segment 0 content: %s\n", 
               (char *)Segments_get_mem(segs, 0));
        
        uint32_t segid = Segments_map(segs, 4);

        /* Expected output: 
           segment 1 content: Sample value
        */
        Segments_copy(segs, 0, segid);
        printf("segment %d content: %s\n", segid, 
               (char *)Segments_get_mem(segs, 1));

        Segments_free(&segs);
}

void read_in_um_program(FILE *um_program)
{
        Segments_T segs = Segments_new(); 
        Segments_read_program(segs, um_program, sizeof(uint32_t));
        Segments_dump(segs, 0);   
        Segments_free(&segs);
}

int main(int argc, char *argv[])
{
        (void) argc;
        /* FILE *input = fopen("test.txt", "r"); */
        /* Segments_new_free_test(input); */
        /* map_Segments_unmap_test(input); */
        /* get_segment_test(input); */
        /* copy_test(input); */

        FILE *um_program = fopen(argv[1], "r");
        assert(um_program);
        read_in_um_program(um_program);

        return 0;
}
