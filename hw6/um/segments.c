/*
 * Martin Gao & Juliet Yue
 * date: 11/27/17
 *
 * Implementation of Segments ADT
 *
 */

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mem.h"
#include "assert.h"
#include "segments.h"
#include "seq.h"
#include "bitpack.h"

#define BITS 8
#define MAX 32

struct Segments_T {
        Seq_T mapped;
        Seq_T unmapped;
        uint32_t next_id;
};

typedef struct Segment {
        uint32_t seg_size;
        uint32_t memory[];
} *Segment;

typedef uint32_t word;

/* takes size of program binary in words and creates corresponding Segment */
static inline Segment malloc_segment(uint32_t size)
{
        uint32_t seg_size = sizeof(uint32_t) + size * sizeof(word);
        Segment new_seg = malloc(seg_size);
        memset(new_seg->memory, 0, size * sizeof(word));
        new_seg->seg_size = size;
        return new_seg;
}

Segments_T Segments_new()
{
        Segments_T new_segs;
        NEW(new_segs);

        new_segs->mapped = Seq_new(0);
        new_segs->unmapped = Seq_new(0);
        new_segs->next_id = 0;

        return new_segs;
}

/* reads program into segment 0 of Segments_T */
void Segments_read_program(Segments_T segments, FILE *program)
{
        assert(program);
        assert(fseek(program, 0, SEEK_END) == 0);

        /* read_program should be called on an empty Segments_T */
        assert(segments->next_id == 0);

        uint32_t prog_size = ftell(program) / sizeof(uint32_t);
        rewind(program);

        Segment result = malloc_segment(prog_size);
        
        for (unsigned i = 0; i < prog_size; i++) {
                for (int j = 1; j <= 4; j++) {
                        result->memory[i] = Bitpack_newu(result->memory[i], 
                                         BITS, MAX - BITS * j, fgetc(program));
                }
        }

        Seq_addhi(segments->mapped, result);
        segments->next_id = 1;
}

/*allocate a new segment of given size int bytes, return the new segment id*/
seg_id Segments_map(Segments_T segments, uint32_t size)
{
        assert(segments);
        if (Seq_length(segments->unmapped) == 0) {
                Seq_addhi(segments->mapped, malloc_segment(size));
                return (segments->next_id)++;
        }

        seg_id new_id = (uintptr_t)(Seq_remhi(segments->unmapped));
        Segment old_segment = Seq_get(segments->mapped, new_id);

        free(old_segment);
        Seq_put(segments->mapped, new_id, malloc_segment(size));
        return new_id;
}

/*deallocate segment of a given id*/
void Segments_unmap(Segments_T segments, seg_id segment_id)
{
        assert(segments);
        Seq_addhi(segments->unmapped, (void *)(uintptr_t)segment_id);
}

/* copies segment origin to target, replacing segment target */
void Segments_copy(Segments_T segments, seg_id origin_id, seg_id target_id)
{
        assert(segments);
        free(Seq_get(segments->mapped, target_id));
        Segment origin = Seq_get(segments->mapped, origin_id);
        uint32_t origin_size = origin->seg_size;
        Segment copy = malloc_segment(origin_size);
        memcpy(copy->memory, origin->memory, origin_size * sizeof(word));
        Seq_put(segments->mapped, target_id, copy);
}

/*get the memory of the segment of a given id*/
void *Segments_get_mem(Segments_T segments, seg_id segment_id)
{
        assert(segments);
        assert(segment_id <= INT_MAX);
        Segment target = Seq_get(segments->mapped, segment_id);
        return target->memory;
}

/*free a Segments_T struct*/
void Segments_free(Segments_T *to_free)
{
        assert(to_free && *to_free);
        Seq_T mapped_del = (*to_free)->mapped;
        uint32_t mapped_len = Seq_length(mapped_del);
        for (uint32_t i = 0; i < mapped_len; ++i) {
                free(Seq_remhi(mapped_del));
        }
        Seq_free(&mapped_del);
        Seq_free(&((*to_free)->unmapped));
        FREE(*to_free);
}

