//
// Created by parallels on 2019/12/25.
//

#include "aioutil.h"

int read_async(int fd, char str[], int buf_size, int sig, struct aiocb *aio) {

    int read_result;

    //初始化结构体
    memset(aio, 0, sizeof(*aio));

    aio->aio_fildes = fd;

    aio->aio_offset = 0;

    aio->aio_buf = str;

    aio->aio_nbytes = buf_size;

    if (sig != -1) {
        aio->aio_sigevent.sigev_notify = SIGEV_SIGNAL;

        aio->aio_sigevent.sigev_signo = sig;

        aio->aio_sigevent.sigev_value.sival_ptr = aio;
    }

    if ((read_result = aio_read(aio) < 0)) {
        perror("aio read: ");
    }

    return read_result;
}

int read_back_thread_async(int fd, char str[], int buf_size, struct aiocb *aio, void (*callback)(sigval_t sigval)) {
    int read_result;

    //初始化结构体
    memset(aio, 0, sizeof(*aio));

    aio->aio_fildes = fd;

    aio->aio_offset = 0;

    aio->aio_buf = str;

    aio->aio_nbytes = buf_size;

    if (callback != NULL) {
        aio->aio_sigevent.sigev_notify = SIGEV_THREAD;

        aio->aio_sigevent.sigev_notify_function = callback;

        aio->aio_sigevent.sigev_value.sival_ptr = aio;
    }

    if ((read_result = aio_read(aio) < 0)) {
        perror("aio read: ");
    }

    return read_result;
}

int write_async(int fd, char *str, int sig) {

    struct aiocb aio;

    int write_result;

    //初始化结构体
    memset(&aio, 0, sizeof(aio));

    aio.aio_fildes = fd;

    aio.aio_buf = str;

    aio.aio_nbytes = strlen(str);

    if (sig != -1) {
        aio.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
        aio.aio_sigevent.sigev_signo = sig;
        aio.aio_sigevent.sigev_value.sival_ptr = &aio;
    }

    if ((write_result = aio_write(&aio)) < 0) {
        perror("aio write: ");
    }

    return write_result;

}
