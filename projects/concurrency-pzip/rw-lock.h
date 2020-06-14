
typedef struct _rwlock_t {
    sem_t writelock;
    sem_t lock;
    sem_t wait_for_wlock;
    int readers;
} rwlock_t;

void rwlock_init(rwlock_t *lock) {
    lock->readers = 0;
    Sem_init(&lock->lock, 1); 
    Sem_init(&lock->writelock, 1); 
    Sem_init(&lock->wait_for_wlock, 1); 
}

void rwlock_acquire_readlock(rwlock_t *lock) {
    Sem_wait(&lock->wait_for_wlock);
    Sem_post(&lock->wait_for_wlock);
    Sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
	Sem_wait(&lock->writelock);
    Sem_post(&lock->lock);
}

void rwlock_release_readlock(rwlock_t *lock) {
    Sem_wait(&lock->lock);
    lock->readers--;
    if (lock->readers == 0)
	Sem_post(&lock->writelock);
    Sem_post(&lock->lock);
}

void rwlock_acquire_writelock(rwlock_t *lock) {
    Sem_wait(&lock->wait_for_wlock);
    Sem_wait(&lock->writelock);
    Sem_post(&lock->wait_for_wlock);
}

void rwlock_release_writelock(rwlock_t *lock) {
    Sem_post(&lock->writelock);
}
