#include <stdlib.h>
#include "synch_wrapper.h"


int
init_lock_wrapper(PLOCK_WRAPPER Lock, int Lock_type)
{
    Lock->lock_type = Lock_type;
    switch(Lock_type)
    {
        case TYPE_MUTEX:
        {
            pthread_mutex_init(&Lock->mutex, NULL);
            break;
        }
        case TYPE_SEMAPHORE:
        {
            sem_init(&Lock->semaphore, 0, 1);
            break;
        }
        case TYPE_SPINLOCK:
        {
            pthread_spin_init(&Lock->spinlock, 0);
            break;
        }
	case TYPE_FUTEX:
	{

	    break;
	}
        default:
        {
            exit(-1);
        }
    }

    return 0;
}


int
acquire_lock_wrapper(PLOCK_WRAPPER Lock)
{
    switch(Lock->lock_type)
    {
        case TYPE_MUTEX:
        {
            pthread_mutex_lock(&Lock->mutex);
            break;
        }
        case TYPE_SEMAPHORE:
        {
            sem_wait(&Lock->semaphore);
            break;
        }
        case TYPE_SPINLOCK:
        {
            pthread_spin_lock(&Lock->spinlock);
            break;
        }
	case TYPE_FUTEX:
	{
	    break;
	}
        default:
        {
            exit(-1);
        }
    }

    return 0;
}

int
release_lock_wrapper(PLOCK_WRAPPER Lock)
{
    switch(Lock->lock_type)
    {
        case TYPE_MUTEX:
        {
            pthread_mutex_unlock(&Lock->mutex);
            break;
        }
        case TYPE_SEMAPHORE:
        {
            sem_post(&Lock->semaphore);
            break;
        }
        case TYPE_SPINLOCK:
        {
            pthread_spin_unlock(&Lock->spinlock);
            break;
        }
	case TYPE_FUTEX:
	{
	    break;
	}
        default:
        {
            exit(-1);
        }
    }

    return 0;
}


int
clean_lock_wrapper(PLOCK_WRAPPER Lock)
{
    switch(Lock->lock_type)
    {
        case TYPE_MUTEX:
        {
            pthread_mutex_destroy(&Lock->mutex);
            break;
        }
        case TYPE_SEMAPHORE:
        {
            sem_destroy(&Lock->semaphore);
            break;
        }
        case TYPE_SPINLOCK:
        {
            pthread_spin_destroy(&Lock->spinlock);
            break;
        }
	case TYPE_FUTEX:
	{
	    break;
	}
        default:
        {
            exit(-1);
        }
    }

    return 0;
}
