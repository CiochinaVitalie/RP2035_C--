#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int _read(int file, char *ptr, int len) {
    return 0;
}

int _write(int file, char *ptr, int len) {
    return len;
}

int _close(int file) {
    return -1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    return 1;
}

void _exit(int status) {
    while (1) {}  // Бесконечный цикл, т.к. выхода из программы нет
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int _getpid(void) {
    return 1;
}

void *_sbrk(int incr) {
    extern char __end__; // Символ конца данных в памяти
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &__end__;
    }
    prev_heap_end = heap_end;

    heap_end += incr;
    return (void *)prev_heap_end;
}