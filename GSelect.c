/** @file GSelect.c
 *  @brief The C implementation of a Select() wrapper
 * 
 *  This is the GSelect implementation. It's a Select wrapper that ensures 
 *  robustness and facility of use. It stores a pool of file descriptors that 
 *  are used for select(). Each file descriptor is coupled with a callback 
 *  function that is invoked whenever select() detects activity. The callback 
 *  function takes a pointer to the file descriptor's stored state as an 
 *  argument.
 * 
 * @author Aliaa Essameldin <aeahmed@qatar.cmu.edu>
 * @bug No known bugs.
 */

#include "GSelect.h"

/* new_pool: Returns an empty pool */
POOL new_pool(void){
    POOL newPool;
    newPool.maxFD = -1;
    FD_ZERO(&newPool.readSet);

    // set all pool elements to zero
    for (int i = 0; i < FD_SETSIZE; i++){
        newPool.elements[i].FD = -1;
        newPool.elements[i].state = NULL;
        newPool.elements[i].callback = NULL;
    }

    return newPool;
}

/* add_fd : adds a file descriptor to the pool
 *
 * input: - pool: pointer to pool to add it to
 *        - FD: file descriptor to add
 *        - callback: poitner to callback function, must take int (FD)
 *                            and void pointer (state pointer) as arguments
 *        - state: takes initial state of the file descriptor or NULL
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
inline int add_fd(POOL *pool, int FD, int (*callback) (POOL *, int, void *),
                  void *state){
    // find empty slot and use it
    for (int i = 0; i < FD_SETSIZE; i++){
        if (pool->elements[i].FD < 0){ //available
            pool->elements[i].FD = FD;
            pool->elements[i].state = state;
            pool->elements[i].callback = callback;
            FD_SET(FD, &pool->readSet);

            if (FD > pool->maxFD){
                pool->maxFD = FD;
            }
            return 0;
        }
    }

    return -1;
}

/* remove_fd : removes a file descriptor from the pool, NEITHER closes it NOR
 *             frees its state. This must be done by the user before remove_fd
 *             is called.
 *
 * input: - pool: pointer to pool to remove the descriptor from
 *        - FD: file descriptor
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
inline int remove_fd(POOL *pool, int FD){
    for (int i = 0; i < FD_SETSIZE; i++){
        if (pool->elements[i].FD == FD){ // found available FD
            pool->elements[i].FD = -1;
            pool->elements[i].state = NULL;
            pool->elements[i].callback = NULL;
            FD_CLR(FD, &pool->readSet);

            // Here I'm adjusting MaxFD
            if (pool->maxFD == FD){
                pool->maxFD = -1;
                for (int i = 0; i < FD_SETSIZE; i++){
                    if (pool->elements[i].FD > pool->maxFD){
                        pool->maxFD = pool->elements[i].FD;
                    }
                }
            }
            return 0;
        }
    }

    return -1;
}

/* RSelect: Monitors the pool and invokes callback function when appropriate.
 *          Blocks execution. Should be called in a loop
 *
 * input: pool: a pointer to the pool to be monitored
 *
 * Returns 0 upon successful execution and -1 otherwise
 */
inline int GSelect(POOL *pool){
    int maxFD = pool->maxFD;
    int readyCount = 0;
    pool->postreadSet = pool->readSet;

    if ((readyCount = select(maxFD+1, &(pool->postreadSet),
                              NULL, NULL, NULL)) < 0){
        return -1;
    }

    /* invoke callback function for all pending FDs */
    for (int i = 0; i < FD_SETSIZE && readyCount > 0; ++i){ //iterating the set
        if (pool->elements[i].FD >= 0
            && FD_ISSET(pool->elements[i].FD, &(pool->postreadSet))){
            readyCount--;
            pool->elements[i].callback(pool, pool->elements[i].FD, pool->elements[i].state);
        }
    }

    return 0;
}
