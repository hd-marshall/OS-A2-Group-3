#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>

using std::vector;

static constexpr int QCAP = 20;

struct Line {
    char *buf = nullptr;
    size_t len = 0;
    size_t seq = 0;
};


static std::map<size_t, Line> queue;  // sequence -> Line
static size_t next_seq_to_read = 0;   // next sequence to assign
static size_t next_seq_to_write = 0;  // next sequence to write

pthread_mutex_t q_mtx;
pthread_mutex_t in_mtx;
pthread_mutex_t out_mtx;

pthread_cond_t not_full;   // readers wait if queue is full
pthread_cond_t not_empty;  // writers wait if queue is empty or next seq is not ready

static bool done_reading = false;
static int global_err = 0;

static FILE *fin = nullptr;
static FILE *fout = nullptr;

struct ThreadArg {
    int id;
};

// ---------------- POSIX getline for Windows ----------------
#ifdef _WIN32
ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    if (!lineptr || !n || !stream) return -1;
    if (!*lineptr) { *n = 128; *lineptr = (char*)malloc(*n); }
    if (!*lineptr) return -1;

    int c;
    size_t i = 0;
    while ((c = fgetc(stream)) != EOF)
    {
        if (i + 1 >= *n)
        {
            *n *= 2;
            char *tmp = (char*)realloc(*lineptr, *n);
            if (!tmp) return -1;
            *lineptr = tmp;
        }
        (*lineptr)[i++] = c;
        if (c == '\n') break;
    }
    if (i == 0 && c == EOF) return -1;
    (*lineptr)[i] = '\0';
    return i;
}
#endif
// ---------------------------------------------------------

void *reader_main(void *vp) {
    char *line = nullptr;
    size_t cap = 0;

    while (true) {
        pthread_mutex_lock(&in_mtx);
        ssize_t nread = getline(&line, &cap, fin);
        pthread_mutex_unlock(&in_mtx);

        if (nread < 0) {
            pthread_mutex_lock(&q_mtx);
            done_reading = true;
            pthread_cond_broadcast(&not_empty); // wake all writers
            pthread_mutex_unlock(&q_mtx);
            break;
        }

        pthread_mutex_lock(&q_mtx);
        while (queue.size() >= QCAP && !global_err)
            pthread_cond_wait(&not_full, &q_mtx);

        if (global_err) {
            pthread_mutex_unlock(&q_mtx);
            break;
        }

        // Add to queue with a sequence number
        Line l;
        l.len = (size_t)nread;
        l.buf = static_cast<char *>(malloc(l.len));
        if (!l.buf) {
            global_err = 1;
            pthread_mutex_unlock(&q_mtx);
            break;
        }
        memcpy(l.buf, line, l.len);
        l.seq = next_seq_to_read++;

        queue[l.seq] = l;

        pthread_cond_broadcast(&not_empty);  // wake writers
        pthread_mutex_unlock(&q_mtx);
    }

    free(line);
    return nullptr;
}

void *writer_main(void *vp) {
    while (true) {
        pthread_mutex_lock(&q_mtx);

        // Wait until the next required line is available
        while ((queue.count(next_seq_to_write) == 0) && !done_reading && !global_err)
            pthread_cond_wait(&not_empty, &q_mtx);

        if (queue.count(next_seq_to_write)) {
            Line l = queue[next_seq_to_write];
            queue.erase(next_seq_to_write);
            next_seq_to_write++;

            pthread_cond_signal(&not_full); // wake a reader
            pthread_mutex_unlock(&q_mtx);

            pthread_mutex_lock(&out_mtx);
            size_t off = 0;
            while (off < l.len) {
                size_t wrote = fwrite(l.buf + off, 1, l.len - off, fout);
                if (wrote == 0 && ferror(fout)) {
                    global_err = 1;
                    break;
                }
                off += wrote;
            }
            pthread_mutex_unlock(&out_mtx);
            free(l.buf);
        } else if (done_reading && queue.empty()) {
            pthread_mutex_unlock(&q_mtx);
            break;
        } else {
            pthread_mutex_unlock(&q_mtx);
        }

        if (global_err) break;
    }

    return nullptr;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s n source_file destination_file\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n = strtol(argv[1], nullptr, 10);
    if (n < 2 || n > 10) {
        fprintf(stderr, "n must be between 2 and 10\n");
        return EXIT_FAILURE;
    }

    fin = fopen(argv[2], "r");
    if (!fin) {
        fprintf(stderr, "Cannot open source file '%s'\n", argv[2]);
        return EXIT_FAILURE;
    }

    fout = fopen(argv[3], "w");
    if (!fout) {
        fclose(fin);
        fprintf(stderr, "Cannot open destination file '%s'\n", argv[3]);
        return EXIT_FAILURE;
    }

    // Initialize synchronization
    pthread_mutex_init(&q_mtx, nullptr);
    pthread_mutex_init(&in_mtx, nullptr);
    pthread_mutex_init(&out_mtx, nullptr);
    pthread_cond_init(&not_full, nullptr);
    pthread_cond_init(&not_empty, nullptr);

    // Start threads
    vector<pthread_t> readers(n), writers(n);
    for (int i = 0; i < n; ++i) {
        pthread_create(&readers[i], nullptr, reader_main, nullptr);
        pthread_create(&writers[i], nullptr, writer_main, nullptr);
    }

    for (int i = 0; i < n; ++i) {
        pthread_join(readers[i], nullptr);
        pthread_join(writers[i], nullptr);
    }

    fflush(fout);
    fclose(fout);
    fclose(fin);

    pthread_mutex_destroy(&q_mtx);
    pthread_mutex_destroy(&in_mtx);
    pthread_mutex_destroy(&out_mtx);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return global_err ? EXIT_FAILURE : EXIT_SUCCESS;
}
