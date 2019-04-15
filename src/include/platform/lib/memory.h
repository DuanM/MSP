/** 
 * Memory Management Routines 
 *  
 * @file memory.h  
 * @author herry
 *  
 * @addtogroup MEM	Memory Management 
 * @ingroup  
 * @{ 
 */
#ifndef __MEMORY_H
#define __MEMORY_H

typedef struct
{
	void *ptr;	
	uint32_t blk_size;
	uint32_t blk_total_num;
	uint32_t blk_rem_num;
}pool_t;


void *memory_init(uint32_t *mem_total_size);

/**
 * Allocate memory block from heap.
 *  
 * @param size size of memory to allocate
 * 
 * @return pointer of memory allocated on success, NULL on 
 *  	   failure
 */
void *heap_alloc(uint32_t size, bool_t init_flag);

/**
 * Create pool from heap and make pool element.
 * 
 * @param blk_num block number
 * @param blk_size block size
 * 
 * @return pointer of memory allocated on success, NULL on fail
 */
pool_t *pool_create(size_t blk_num, uint32_t blk_size);

/**
 * Allocate pool block.
 *  
 * @param pool pointer
 * 
 * @return pointer of memory allocated on success, NULL on fail
 */
void *pool_alloc(pool_t *pool);

/**
 * Free pool block.
 *  
 * @param pool pointer
 * 
 * @return success on TRUE, and error on FAIL
 */
bool_t pool_free(pool_t *pool, void *ptr);

/**
 * set a memory block with int8 value c
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the buffer 
 */
void mem_set(void *buffer, uint8_t c, uint32_t count);

/**
 * clear a memory block 
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the buffer 
 */
void mem_clr(void *buffer, uint32_t count);


/**
 * copy src memory block to dest memory block
 *  
 * @param mem pointer to memory block
 * 
 * @return pointer of the dest
 */
void mem_cpy(void *dest, void *src, uint32_t count);


/*
*********************************************************************************************************
*                                              Mem_Cmp()
*
* Description : Verifies that ALL data octets in two memory buffers are identical in sequence.
*
* Argument(s) : p1_mem      Pointer to first  memory buffer.
*
*               p2_mem      Pointer to second memory buffer.
*
*               size        Number of data buffer octets to compare (see Note #1).
*
* Return(s)   : DEF_YES, if 'size' number of data octets are identical in both memory buffers.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) Null compares allowed (i.e. zero-length compares); 'DEF_YES' returned to indicate
*                   identical null compare.
*
*               (2) Many memory buffer comparisons vary ONLY in the least significant octets -- e.g.
*                   network address buffers.  Consequently, memory buffer comparison is more efficient
*                   if the comparison starts from the end of the memory buffers which will abort sooner
*                   on dissimilar memory buffers that vary only in the least significant octets.
*
*               (3) For best CPU performance, optimized to compare data buffers using 'CPU_ALIGN'-sized
*                   data words. Since many word-aligned processors REQUIRE that multi-octet words be accessed on
*                   word-aligned addresses, 'CPU_ALIGN'-sized words MUST be accessed on 'CPU_ALIGN'd
*                   addresses.
*
*               (4) Modulo arithmetic is used to determine whether a memory buffer starts on a 'CPU_ALIGN'
*                   address boundary.
*
*                   Modulo arithmetic in ANSI-C REQUIREs operations performed on integer values.  Thus
*                   address values MUST be cast to an appropriately-sized integer value PRIOR to any
*                  'mem_align_mod' arithmetic operation.
*******************************************************************************************************
*/
uint8_t  mem_cmp(void *p1_mem, void *p2_mem, uint32_t size);

#endif
/**
 * @}
 */
