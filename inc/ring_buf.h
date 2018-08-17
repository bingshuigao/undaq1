/* The ring buffer is one of the most important classes for the DAQ, it is used
 * by different threads and/or different processes for communication. The
 * synchronization is done by using mutex.
 * To speed up the read and write of the buffer and simplize the
 * implementation, we map two continuous virtual spaces (each equals the size
 * of the buffer) to the physical memory space.  For more information how it
 * works: https://github.com/willemt/cbuffer
 * ------
 * By B.Gao (gaobsh@impcas.ac.cn) Jul 2018 */

#ifndef __RING_BUF_HHH
#define __RING_BUF_HHH

#include <stdint.h>
#include <pthread.h>

class ring_buf
{
public:
	ring_buf(); 
	~ring_buf();

	/* Initialize the buffer: allocate memory, initialize the lock and read
	 * and write pointers. This function must be called before the ring
	 * buffer can be used for anything. 
	 * Note: The size has to be multiple of page size.
	 * @param size2 Twice the size of the buffer (in bytes) 
	 * @return 0 on success and -1 on failure*/
	int init(uint32_t size2); 

	/* read from ring buffer. First, it acquires the lock, then read the
	 * desired amount of (or the maximum available) data . At the end of
	 * read, the corresponding read pointer is updated. In addition, it
	 * checks if the distance between the read pointer and beginning of the
	 * ring buffer is larger than the size of the buffer, if yes decrease
	 * the read and write pointers by the size of the buffer.
	 * @param dst A pointer to a buffer where the data will be read to.
	 * @param size The desired size of data to be read out (bytes). If 0,
	 * read as much as possible.
	 * @return The actual number of bytes read, -1 in case of error.
	 * */
	int32_t read(void* dst, int32_t size);

	/* write to ring buffer. First, it acquires the lock, then write the
	 * desired amount (or maximum available space) of data to the buffer. 
	 * @param src A pointer to a buffer where the data is from.
	 * @param size The desired size of data to write (bytes).
	 * @return The actual number of bytes written. -1 in case of error.
	 * */
	int32_t write(void* src, int32_t size);

	/* Get the size of the ring buffer.  */
	uint32_t get_sz() {return size;} 

	/* Get the available/free space of the ring buffer, this is a wrapper
	 * of the corresponding private version, except that we acquire the
	 * lock before calculating distances between the read and write
	 * pointers.
	 * @return Return the free/used space of the buffer (in bytes). Return
	 * -1 in case of error.
	 */
	uint32_t get_free();
	uint32_t get_used();

private:
	/* allocate memory for the ring buffer and create appropriate maps.
	 * @param sz Size of the ring buffer (in bytes)
	 * @return Return 0 if succeed, otherwise -1. */
	int create_map(uint32_t sz);

	/* Get the available/free space of the ring buffer, this function
	 * should be called with mutex locked.
	 * @return Return the free/used space of the buffer (in bytes). 
	 */
	uint32_t get_used_nolock() {return wr_ptr - rd_ptr;}
	uint32_t get_free_nolock() {return size - get_used_nolock();}

private:
	char* rd_ptr; /* read pointer */
	char* wr_ptr; /* write pointer */
	char* buf_ptr; /* Address of the first byte of the buffer */
	uint32_t size; /* size of the buffer (in bytes)*/
	pthread_mutex_t mutex; /* mutex (for inter-thread synchronization) */
	int mutex_init; /* set to 1 if mutex initialized */
};


#endif
