#ifndef __BTMG_SEMAPHORE_H
#define __BTMG_SEMAPHORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INVALID_FD
#define INVALID_FD (-1)
#endif

typedef struct semaphore_t {
    int fd;
} semaphore_t;

semaphore_t *btmg_semaphore_new(unsigned int value);
void btmg_semaphore_free(semaphore_t *semaphore);
void btmg_semaphore_wait(semaphore_t *semaphore);
bool btmg_semaphore_try_wait(semaphore_t *semaphore);
void btmg_semaphore_post(semaphore_t *semaphore);
int btmg_semaphore_get_fd(const semaphore_t *semaphore);
int btmg_semaphore_wait_timeout(semaphore_t *semaphore, int timeout_ms);

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif
