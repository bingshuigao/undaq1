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
#include <unistd.h>

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

	/* Clear the contents in the buffer, should hold the lock when doing
	 * this */
	void clear() {rd_ptr = wr_ptr = buf_ptr;}

	/* read from ring buffer. This method is obsoleted because it
	 * encourages the user to play around with the lock. Now the preferred
	 * use it to *always* aquire the lock before operating on the ring
	 * buffer (get used/free, read/write). The method read1 should be used
	 * in stead.
	 * First, it acquires the lock,
	 * then read the desired amount of (or the maximum available) data . At
	 * the end of read, the corresponding read pointer is updated. In
	 * addition, it checks if the distance between the read pointer and
	 * beginning of the ring buffer is larger than the size of the buffer,
	 * if yes decrease the read and write pointers by the size of the
	 * buffer.  
	 * @param dst A pointer to a buffer where the data will be read to.  
	 * @param size The desired size of data to be read out (bytes). If 0,
	 * read as much as possible.  
	 * @param peek If true, do not update the read/write pointers,
	 * otherwise update the pointers after reading (which means remove data
	 * after being read out) @param lock If true, need to acquire the lock,
	 * otherwise don't acquire the lock (this parameter should be set to
	 * true only if the lock has been acquired before calling this
	 * function) 
	 * @return The actual number of bytes read, -1 in case of
	 * error.
	 * */
	int32_t read(void* dst, int32_t size, bool peek=false, bool lock=true);

	/* write to ring buffer. 
	 *  This method is obsoleted because it
	 * encourages the user to play around with the lock. Now the preferred
	 * use it to *always* aquire the lock before operating on the ring
	 * buffer (get used/free, read/write). The method write1 should be used
	 * in stead.
	 * First, it acquires the lock, then write the desired amount (or
	 * maximum available space) of data to the buffer.  
	 * @param src A pointer to a buffer where the data is from.  
	 * @param size The desired size of data to write (bytes).  
	 * @return The actual number of bytes written. -1 in case of error.  
	 * @param lock If true, need to acquire the lock, otherwise don't
	 * acquire the lock (this parameter should be set to true only if the
	 * lock has been acquired before calling this function)
	 * */
	int32_t write(void* src, int32_t size, bool lock=true);

	/* See also read() and write(). This is the recommanded use of read and
	 * write method. Here we never play around the lock, the user should
	 * *always* aquire the lock before calling the read/write and release
	 * the lock when done. */
	int32_t read1(void* dst, int32_t size, bool peek=false)
	{ return read(dst, size, peek, false); }
	int32_t write1(void* src, int32_t size)
	{ return write(src, size, false); }

	
	/* acquire/release the lock, return 0 if succeed, otherwise return -1.*/
	int get_lock()
	{
		if (pthread_mutex_lock(&mutex)) 
			return -1;
		return 0;
	}
	int rel_lock()
	{
		if (pthread_mutex_unlock(&mutex)) 
			return -1;
		return 0;
	}

	/* Get the size of the ring buffer.  */
	uint32_t get_sz() {return size;} 

	/* (Obsoleted, use get_free1/used1 in stead!) 
	 * Get the available/free space of the ring buffer, this is a wrapper
	 * of the corresponding private version, except that we acquire the
	 * lock before calculating distances between the read and write
	 * pointers.
	 * @return Return the free/used space of the buffer (in bytes). Return
	 * -1 in case of error.
	 */
	int32_t get_free();
	int32_t get_used();
	uint32_t get_used_nolock() {return wr_ptr - rd_ptr;}
	uint32_t get_free_nolock() {return size - get_used_nolock();}

	/* Get used/free space of the ring buffer.*/
	uint32_t get_used1() {return wr_ptr - rd_ptr;}
	uint32_t get_free1() {return size - get_used_nolock();}

	/* get the user data buffer */
	char* get_usr_data() {return usr_data;}

	/* blocked until the free space of the buffer is larger than sz. The
	 * calling thread should have the lock released while calling this
	 * function. 
	 * @param sz the size of free space that the ring buffer should have.
	 * @param t_us. The chunk of time that it sleeps if not enough space.
	 * @param t2_us. The timeout after which it returns no matter what. If
	 * negative, wait infinitely.
	 * @return. Return 0 if free space becomes available. otherwise(e.g.
	 * timeout) return -1.
	 * Note: it works well only in cases where only 1 thread writes into
	 * the buffer. This is because when the functions returns, it has
	 * released the lock. If only one thread writes into the buffer (we
	 * assume the calling thread), the free space will only increase before
	 * the calling thread writes into it. So it gurantees that the free
	 * space is larger than sz when the calling thread wants to write into
	 * it.*/
	uint32_t wait_buf_free(uint32_t sz, int t_us = 12, int t2_us = -1);

private:
	/* allocate memory for the ring buffer and create appropriate maps.
	 * @param sz Size of the ring buffer (in bytes)
	 * @return Return 0 if succeed, otherwise -1. */
	int create_map(uint32_t sz);

	/* Get the available/free space of the ring buffer, this function
	 * should be called with mutex locked.
	 * @return Return the free/used space of the buffer (in bytes). 
	 */

private:
	char* rd_ptr; /* read pointer */
	char* wr_ptr; /* write pointer */
	char* buf_ptr; /* Address of the first byte of the buffer */
	uint32_t size; /* size of the buffer (in bytes)*/
	pthread_mutex_t mutex; /* mutex (for inter-thread synchronization) */
	int mutex_init; /* set to 1 if mutex initialized */
	char usr_data[1024]; /* can be used for anything. */
};


#endif
