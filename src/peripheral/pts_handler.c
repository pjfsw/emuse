#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <poll.h>
#include <libgen.h>
#include <time.h>
#include <termios.h>
#include <pthread.h>
#include "pts_handler.h"


#define PTS_NAME_MAX_LENGTH 64

typedef struct {
    int fdm;
    pthread_t read_thread;
    uint8_t buffer[256];
    uint8_t read_ptr;
    uint8_t write_ptr;    
    uint16_t headroom;    
} Reader;

struct PtsHandler {
    Reader reader;
    uint8_t output_buffer[1];
    char pts_name[PTS_NAME_MAX_LENGTH+1];
};

static void *read_thread(void *user_data) {
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 1000};

    Reader *reader = (Reader*)user_data;

    while (true) {
        uint8_t leeway = reader->read_ptr - reader->write_ptr;
        uint16_t headroom = leeway;
        if (headroom == 0) {
            headroom = 256;
        }
        reader->headroom = headroom;

        bool buffer_available = headroom > 3;
        if (buffer_available) {
            int n = read(reader->fdm, &reader->buffer[reader->write_ptr], 1);
            if (n > 0) {
                /*char c = data->buffer[data->write_ptr];
                if (c == 10) {
                    fprintf(stderr, "\033[36m<10>\n");
                } else if (c == 13) {
                    fprintf(stderr, "\033[36m<13>\n");
                } else {
                    fprintf(stderr, "\033[36m%c", c);
                }*/
                reader->write_ptr++;
            } 
        } else {
            fprintf(stderr,
                "Buffer not available (read %d, write %d), dropping bytes\n",
                reader->read_ptr,
                reader->write_ptr);
        }
        nanosleep(&delay, NULL);
    }
    return NULL;
}


PtsHandler *ptsHandlerCreate() {
    int fdm = posix_openpt(O_RDWR | O_NONBLOCK);
    if (fdm == -1) {
        fprintf(stderr, "Failed to open pseudo terminal: %s\n", strerror(errno));
        return false;
    }
    struct termios config;
    if(tcgetattr(fdm, &config) < 0) {
        perror("Failed to get termios config");
    } else {
        config.c_lflag &= ~ECHO;
        tcsetattr(fdm, 0, &config);
    }
    PtsHandler *pts = calloc(1, sizeof(PtsHandler));
    Reader *reader = &pts->reader;
    reader->fdm = fdm;

    strncpy(pts->pts_name, ptsname(reader->fdm), PTS_NAME_MAX_LENGTH);
    printf("Serial interface available at pseudo terminal %s\n", pts->pts_name);

    grantpt(reader->fdm);
    unlockpt(reader->fdm);

    int fds = open(ptsname(reader->fdm), O_RDWR);
    close(fds);

    int rc = pthread_create(&reader->read_thread, NULL, read_thread, reader);
    if (rc == -1) {
        printf("Unable to create thread, %d", rc);
        free(pts);
        return NULL;
    }

    return pts;
}

void ptsWriteByte(PtsHandler *pts, uint8_t data) {
    pts->output_buffer[0] = data;

    write(pts->reader.fdm, pts->output_buffer, 1);
}

bool ptsIsByteAvailable(PtsHandler *pts) {
    uint8_t diff = pts->reader.write_ptr - pts->reader.read_ptr;
    if (diff > 0) {
        return true;
    }
    return false;
}

bool ptsReadByte(PtsHandler *pts, uint8_t *byte) {    
    if (ptsIsByteAvailable(pts)) {
        Reader *reader = &pts->reader;
        *byte = pts->reader.buffer[pts->reader.read_ptr];
        reader->read_ptr++;
        return true;
    }
    return false;
}
