#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

#include "bt_log.h"
#include "bt_semaphore.h"

#if !defined(EFD_SEMAPHORE)
#define EFD_SEMAPHORE (1 << 0)
#endif

semaphore_t *btmg_semaphore_new(unsigned int value)
{
    semaphore_t *ret = NULL;
    ret = (semaphore_t *)calloc(1, sizeof(semaphore_t));

    if (ret) {
        ret->fd = eventfd(value, EFD_SEMAPHORE);
        if (ret->fd == INVALID_FD) {
            BTMG_ERROR("unable to allocate semaphore: %s", strerror(errno));
            free(ret);
            ret = NULL;
        }
    }

    return ret;
}

void btmg_semaphore_free(semaphore_t *semaphore)
{
    if (!semaphore)
        return;

    if (semaphore->fd != INVALID_FD)
        close(semaphore->fd);

    free(semaphore);
}

int btmg_semaphore_wait_timeout(semaphore_t *semaphore, int timeout_ms)
{
    uint64_t value;
    struct pollfd rfds;
    int ret = BT_OK;

    if (semaphore == NULL || semaphore->fd == INVALID_FD) {
        BTMG_ERROR("sema_wait error, make sure the semaphore is valid or not");
        return BT_ERROR_INVALID_ARGS;
    }
    memset(&rfds, 0, sizeof(struct pollfd));
    rfds.fd = semaphore->fd;
    rfds.events |= POLLIN;
    ret = poll(&rfds, 1, timeout_ms);
    if (ret < 0) {
        BTMG_ERROR("Error poll:%s", strerror(errno));
        goto end;
    }
    if (ret == 0) {
        BTMG_DEBUG("poll time out");
        goto end;
    }
    if (rfds.revents & POLLIN) {
        if (eventfd_read(semaphore->fd, &value) == -1) {
            BTMG_ERROR("unable to wait on semaphore: %s", strerror(errno));
        }
    }
end:
    return BT_ERROR;
}
void btmg_semaphore_wait(semaphore_t *semaphore)
{
    uint64_t value;

    if (semaphore == NULL || semaphore->fd == INVALID_FD) {
        BTMG_ERROR("sema_wait error, make sure the semaphore is valid or not");
        return;
    }

    if (eventfd_read(semaphore->fd, &value) == -1)
        BTMG_ERROR("unable to wait on semaphore: %s", strerror(errno));
}

bool btmg_semaphore_try_wait(semaphore_t *semaphore)
{
    int flags = 0;
    eventfd_t value;

    if (semaphore == NULL || semaphore->fd == INVALID_FD) {
        BTMG_ERROR("sema_try_wait error, make sure the semaphore is valid or not");
        return false;
    }

    flags = fcntl(semaphore->fd, F_GETFL);
    if (flags == -1) {
        BTMG_ERROR("unable to get flags for semaphore fd: %s", strerror(errno));
        return false;
    }

    if (fcntl(semaphore->fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        BTMG_ERROR("unable to set O_NONBLOCK for semaphore fd: %s", strerror(errno));
        return false;
    }

    if (eventfd_read(semaphore->fd, &value) == -1) {
        fcntl(semaphore->fd, F_SETFL, flags); //resetore flags for semaphore fd, add by yqsf
        return false;
    }

    if (fcntl(semaphore->fd, F_SETFL, flags) == -1)
        BTMG_ERROR("unable to resetore flags for semaphore fd: %s", strerror(errno));

    return true;
}

void btmg_semaphore_post(semaphore_t *semaphore)
{
    if (semaphore == NULL || semaphore->fd == INVALID_FD) {
        BTMG_ERROR("sema_post error, make sure the semaphore is valid or not");
        return;
    }

    if (eventfd_write(semaphore->fd, 1ULL) == -1)
        BTMG_ERROR("unable to post to semaphore: %s", strerror(errno));
}

int btmg_semaphore_get_fd(const semaphore_t *semaphore)
{
    if (semaphore == NULL || semaphore->fd == INVALID_FD) {
        BTMG_ERROR("sema_get_fd error, make sure the semaphore is valid or not");
        return BT_ERROR_INVALID_ARGS;
    }

    return semaphore->fd;
}
