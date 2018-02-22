/*
 * Martin Gao & Juliet Yue
 * date: 11/13/17
 *
 * segments.h
 *
 * Interface for the Segments ADT
 *
 */

#ifndef SEGMENTS_H_
#define SEGMENTS_H_

#include <stdint.h>
#include <stdio.h>

struct Segment_T;

typedef struct Segments_T *Segments_T;

typedef uint32_t seg_id;

/*Initialize a new struct Segments_T from size*/
Segments_T Segments_new(FILE *program);

/*allocate a new segment of given size, return the new segment id*/
seg_id Segments_map(Segments_T segments, uint32_t size);

/*deallocate segment of a given id*/
void Segments_unmap(Segments_T segments, seg_id segment_id);

/* copies segment origin to target, replacing segment target */
void Segments_copy(Segments_T segments, seg_id origin, seg_id target);

/*get the memory of the segment of a given id*/
void *Segments_get_mem(Segments_T segments, seg_id segment_id);

/*free a Segments_T struct*/
void Segments_free(Segments_T* to_free);


#endif
