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

/* takes  of program binary in bytes and creates corresponding Segment */
static inline Segment new_segment(uint32_t size)
{
        Segment new_seg = calloc(size + 1, sizeof(uint32_t));
        new_seg->seg_size = size;
        return new_seg;
}

static inline Segment read_in_Segment(FILE *program)
{
        assert(program);
        assert(fseek(program, 0, SEEK_END) == 0);
        uint32_t prog_size = ftell(program) / 4;
        rewind(program);

        Segment result = new_segment(prog_size);

        for (unsigned i = 0; i < prog_size; i++) {
                for (int j = 1; j <= 4; j++) {
                        result->memory[i] = Bitpack_newu(result->memory[i], 
                                         BITS, MAX - BITS * j, fgetc(program));
                }
        }

        return result;
}

/*Initialize a new struct Segments_T from size*/
Segments_T Segments_new(FILE *source)
{
        Segments_T new_segs;
        NEW(new_segs);

        Seq_T mapped = Seq_new(0);
        Seq_addhi(mapped, read_in_Segment(source));         /* put in segment 0 */
        new_segs->mapped = mapped;

        new_segs->next_id = 1;

        Seq_T unmapped = Seq_new(0);
        new_segs->unmapped = unmapped;

        return new_segs;
}

/*allocate a new segment of given size, return the new segment id*/
seg_id Segments_map(Segments_T segments, uint32_t size)
{
        assert(segments);
        if (Seq_length(segments->unmapped) == 0) {
                Seq_addhi(segments->mapped, new_segment(size));
                int curr_id = segments->next_id;
                segments->next_id += 1;
                return curr_id;
        }
        seg_id new_id = (uintptr_t)(Seq_remhi(segments->unmapped));
        Seq_put(segments->mapped, new_id, new_segment(size));
        return new_id;
}

/*deallocate segment of a given id*/
void Segments_unmap(Segments_T segments, seg_id segment_id)
{
        assert(segments);
       /* Segment to_free = Seq_get(segments->mapped, segment_id);*/
        Segment to_free = Seq_put(segments->mapped, segment_id, NULL);
        free(to_free);
        Seq_addhi(segments->unmapped, (void *)(uintptr_t)segment_id);
}

/* copies segment origin to target, replacing segment target */
void Segments_copy(Segments_T segments, seg_id origin_id, seg_id target_id)
{
        assert(segments);
        Segments_unmap(segments, target_id);

        Segment origin = Seq_get(segments->mapped, origin_id);
        uint32_t origin_size = origin->seg_size;
        Segment copy = new_segment(origin_size);
        memcpy(copy->memory, origin, origin_size + 1);

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

        for(int i = 0; i < Seq_length((*to_free)->mapped); i++) {
              if(Seq_get((*to_free)->mapped, i) != NULL) {
                        free(Seq_get((*to_free)->mapped, i));
                }

        }

        Seq_free(&((*to_free)->mapped));
        Seq_free(&((*to_free)->unmapped));
        FREE(*to_free);
}
