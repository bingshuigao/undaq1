#include "ring_buf.h"
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

ring_buf::ring_buf()
{
	/* Initialize the member variables */
	rd_ptr = NULL;
	wr_ptr = NULL;
	buf_ptr = NULL;
	buf_size = 0;
	mutex_init = 0;
	/* The initialization of mutex is delayed to the Init() function where
	 * error checks can be done */
//	mutex = pthread_mutex_init(&mutex, NULL);
}

ring_buf::~ring_buf()
{
	/* Free the mapped virtual memory space for the ring buffer. */
	if (buf_ptr) 
		munmap(buf_ptr, buf_size<<1);
	/* destroy the mutex */
	if (mutex_init)
		pthread_mutex_destroy(&mutex);
}

/* allocate memory for the ring buffer and create appropriate maps.
 * @param sz Twice the size of the ring buffer (in bytes)
 * @return Return 0 if succeed, otherwise -1. */
int ring_buf::create_map(uint32_t sz)
{
	long page_sz = sysconf(_SC_PAGESIZE);
	char path[] = "/tmp/impdaq-XXXXXX";
	int fd;
	void* addr;

	/* truncate size to multiple of page size */
	sz >>= 1;
	if (page_sz == -1) 
		return -1;
	if (sz <= page_sz) 
		buf_size = page_sz;
	else 
		buf_size = sz / page_sz * page_sz;

	/* get a unique file name from the template */
	fd = mkstemp(path);
	if (fd < 0) 
		return -1;

	if (unlink(path)) 
		return -1;

	if (ftruncate(fd, buf_size)) 
		return -1;

	/* alloc memory space and create appropriate maps */
	buf_ptr = (char *)mmap(NULL, buf_size<<1, PROT_NONE, 
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (buf_ptr == MAP_FAILED) {
		buf_ptr = NULL;
		return -1;
	}
	addr = mmap(buf_ptr, buf_size, PROT_READ | PROT_WRITE, 
			MAP_FIXED | MAP_SHARED, fd, 0);
	if (addr != buf_ptr) 
		return -1;
	addr = mmap(buf_ptr + buf_size, buf_size, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_SHARED, fd, 0);
	if (addr != buf_ptr + buf_size) 
		return -1;

	if (close(fd)) 
		return -1;

	return 0;
}


/* Initialize the buffer: allocate memory, initialize the lock and read and
 * write pointers. This function must be called before the ring buffer can be
 * used for anything. 
 * Note: The size has to be multiple of page size. 
 * @param size The size of the buffer (in bytes) 
 * @return 0 on success and -1 on failure*/
int ring_buf::init(uint32_t sz)
{
	/* allocate memory and create appropriate maps for it */
	if (create_map(sz)) {
		return -1;
	}

	/* init the mutex */
	if (pthread_mutex_init(&mutex, NULL)) {
		return -1;
	}
	mutex_init = 1;

	/* Init pointers, */
	rd_ptr = buf_ptr;
	wr_ptr = buf_ptr;

	return 0;
}

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
int32_t ring_buf::read(void* dst, int32_t sz, bool peek, bool lock)
{
	uint32_t sz_used, sz_read;

	if (lock) {
		/* acquire the lock */
		if (pthread_mutex_lock(&mutex)) {
			return -1;
		}
	}

	/* read data from ring buffer */
	sz_used = get_used_nolock();
	if (sz >= sz_used || sz == 0) 
		sz_read = sz_used;
	else
		sz_read = sz;
	memcpy(dst, rd_ptr, sz_read);

	/* if not peek, update the read/write pointers */
	if (!peek) {
		rd_ptr += sz_read;

		/* update read and write pointers if neccessary */
		if (rd_ptr - buf_ptr >= buf_size) {
			rd_ptr -= buf_size;
			wr_ptr -= buf_size;
		}
	}

	if (lock) {
		/* release the lock */
		if (pthread_mutex_unlock(&mutex)) {
			return -1;
		}
	}
	return sz_read;
}

/* write to ring buffer. First, it acquires the lock, then write the desired
 * amount (or maximum available space) of data to the buffer. 
 * @param src A pointer to a buffer where the data is from.
 * @param size The desired size of data to write (bytes).
 * @return The actual number of bytes written. -1 in case of error.
 * */
int32_t ring_buf::write(void* src, int32_t sz, bool lock)
{
	uint32_t sz_write, sz_free;

	if (lock) {
		/* acquire the lock */
		if (pthread_mutex_lock(&mutex)) {
			return -1;
		}
	}

	/* read data from ring buffer */
	sz_free = get_free_nolock();
	if (sz >= sz_free)
		sz_write = sz_free;
	else
		sz_write = sz;
	memcpy(wr_ptr, src, sz_write);
	wr_ptr += sz_write;

	if (lock) {
		/* release the lock */
		if (pthread_mutex_unlock(&mutex)) {
			return -1;
		}
	}
	return sz_write;
}


/* Get the available/free space of the ring buffer, this is a wrapper
 * of the corresponding private version, except that we acquire the
 * lock before calculating distances between the read and write
 * pointers.
 * @return Return the free/used space of the buffer (in bytes). Return
 * -1 in case of error.
 */
int32_t ring_buf::get_free()
{
	uint32_t sz_free;

	/* acquire the lock */
	if (pthread_mutex_lock(&mutex)) {
		return -1;
	}

	sz_free = get_free_nolock();

	/* release the lock */
	if (pthread_mutex_unlock(&mutex)) {
		return -1;
	}

	return sz_free;
}
int32_t ring_buf::get_used()
{
	uint32_t sz_used;

	/* acquire the lock */
	if (pthread_mutex_lock(&mutex)) {
		return -1;
	}

	sz_used = get_used_nolock();

	/* release the lock */
	if (pthread_mutex_unlock(&mutex)) {
		return -1;
	}

	return sz_used;
}

int ring_buf::wait_buf_free(uint32_t sz, int t_us, int t2_us)
{
	uint32_t free_sz;
	int t_out = 0;
begin:
	if (t2_us > 0 && t_out >= t2_us)
		return -1;
	free_sz = get_free();
	if (free_sz == -1)
		return -1;
	if (free_sz >= sz)
		return 0;
	usleep(t_us);
	t_out += t_us;
	goto begin;
}

int32_t ring_buf::skip(int32_t size)
{
	uint32_t sz_used, sz_read;

	/* read data from ring buffer */
	sz_used = get_used_nolock();
	if (size >= sz_used) 
		sz_read = sz_used;
	else
		sz_read = size;

	/*  update the read/write pointers */
	rd_ptr += sz_read;

	/* update read and write pointers if neccessary */
	if (rd_ptr - buf_ptr >= buf_size) {
		rd_ptr -= buf_size;
		wr_ptr -= buf_size;
	}

	return sz_read;
}
