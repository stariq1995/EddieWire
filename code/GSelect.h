/** @file GSelect.h                                                                  
 *  @brief Function prototypes for the Select wrapper
 *                                                                             
 *  This is the header file of the Select() generic wrapper. It contains the
 *  prototypes for the GSelect module and eventually any macros, constants,
 *  or global variables needed.                                         
 *                                                                            
 *  @author Aliaa Essameldin <aeahmed@qatar.cmu.edu> 
 *  @bug No known bugs.                                                                         
 */

#ifndef __GSELECT_H__
#define __GSELECT_H__
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

typedef struct Pool_element ELEMENT;
typedef struct Pool_FD POOL;

/* Pool_element: A member of the pool monitored by Select() */
struct Pool_element
{
  int FD;                                 /* File descriptor */
  void *state;                            /* Pointer to state */
  int (*callback)(POOL *, int, void *);   /* callback function */
};

/* Pool_FD: A pool of descriptors to be monitored and maintained by
 *          the Select function
 *
 * The Struct was copied from class23, 15-213 slides by Khaled Harras
 * and slightly modified to fit the purposes of this wrapper
 */
struct Pool_FD
{
    int maxFD;                  /* largest descriptor in read_set */
    fd_set readSet;             /* set of all active descriptors */
    fd_set postreadSet;        /* subset of descriptors ready for reading */
    ELEMENT elements[FD_SETSIZE];     /* set of active read buffers */
};

/* Function prototypes */
/* new_pool: Returns an empty pool */
POOL new_pool(void);

/* add_fd : adds a file descriptor to the pool
 *
 * input: - pool: pointer to pool to add it to
 *        - FD: file descriptor to add
 *        - callback: poitner to callback function, must take int (FD)
 *                            and void pointer (state pointer) as arguments. It
 *                            must return 0 upon sucessful execution and -1
 *                            otherwise.
 *        - state: takes initial state of the file descriptor or NULL
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
int add_fd(POOL *pool, int FD, int (*callback) (POOL *, int, void *),
	       void *state);

/* remove_fd : removes a file descriptor from the pool, does NOT close it!
 *
 * input: - pool: pointer to pool to remove the descriptor from
 *        - FD: file descriptor
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
int remove_fd(POOL *pool, int FD);

/* RSelect: Monitors the pool and invokes callback function when appropriate.
 *          Blocks execution. Should be called in a loop
 *
 * input: pool: a pointer to the pool to be monitored
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
int GSelect(POOL *pool);

#endif //__GSELECT_H__
