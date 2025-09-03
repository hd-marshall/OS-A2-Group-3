// mscopier.cpp — Task 2, Subtask 1 ONLY (C++17 + POSIX pthreads).
// Usage: ./mscopier n source_file destination_file
// Spawns n reader threads and n writer threads.
// Shared queue capacity = 20 lines. Readers/writers "take turns" (no mutex/condvar yet).
// Subtasks 2–3: you'll add pthread_mutex / pthread_cond and remove spins — see TODOs below.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using std::string;
using std::vector;

static constexpr int QCAP = 20;

struct Line
{
    char *buf = nullptr;
    size_t len = 0;
};

// ------------------ QUEUE STATE ------------------
static Line qbuf[QCAP];
static int qhead = 0;  // next index to pop
static int qtail = 0;  // next index to push
static int qcount = 0; // number of queued lines

// TODO(Subtask 2): Declare mutexes *right here*:
//   pthread_mutex_t q_mtx;   // protects qhead/qtail/qcount and qbuf[]
//   pthread_mutex_t in_mtx;  // serializes getline(fin)
//   pthread_mutex_t out_mtx; // serializes fwrite(fout) for one whole line
// TODO(Subtask 3): Declare condition variables bound to q_mtx here:
//   pthread_cond_t  not_full;   // readers wait when queue is full
//   pthread_cond_t  not_empty;  // writers wait when queue is empty

// ------------------ TURN TOKENS (Subtask 1 only) ------------------
static volatile int reader_turn = 0; // which reader id may read next
static volatile int writer_turn = 0; // which writer id may write next
// TODO(Subtask 3): REMOVE reader_turn / writer_turn and all turn-wait loops;
//   condvars will replace turn-taking + busy-waiting entirely.

static volatile int done_reading = 0; // set when EOF observed
static volatile int global_err = 0;   // sticky error -> exit non-zero

static FILE *fin = nullptr;
static FILE *fout = nullptr;

struct ThreadArg
{
    int id;
    int n;
};

static inline void polite_yield()
{
    // Courtesy yield (Subtask 1 only). Will be replaced by condvars in Subtask 3.
    sched_yield(); // advisory under SCHED_OTHER; fine here for demo purposes. :contentReference[oaicite:1]{index=1}
}

static inline bool queue_full() { return qcount == QCAP; }
static inline bool queue_empty() { return qcount == 0; }

// NOTE: In Subtask 2+3, these helpers assume CALLER HOLDS q_mtx.
static void enqueue_line(const char *src, size_t len)
{
    // TODO(Subtask 2): Require q_mtx locked by caller before touching qbuf/qhead/qtail/qcount.
    qbuf[qtail].buf = static_cast<char *>(std::malloc(len));
    if (!qbuf[qtail].buf)
    {
        std::fprintf(stderr, "malloc failed\n");
        global_err = 1;
        return;
    }
    std::memcpy(qbuf[qtail].buf, src, len);
    qbuf[qtail].len = len;
    qtail = (qtail + 1) % QCAP;
    qcount++;
}

static bool dequeue_line(char **out, size_t *olen)
{
    // TODO(Subtask 2): Require q_mtx locked by caller before touching qbuf/qhead/qtail/qcount.
    if (queue_empty())
        return false;
    *out = qbuf[qhead].buf;
    *olen = qbuf[qhead].len;
    qbuf[qhead].buf = nullptr;
    qbuf[qhead].len = 0;
    qhead = (qhead + 1) % QCAP;
    qcount--;
    return true;
}

static void *reader_main(void *vp)
{
    auto *a = static_cast<ThreadArg *>(vp);
    const int my = a->id;
    char *line = nullptr; // getline() will malloc/resize as needed
    size_t cap = 0;

    for (;;)
    {
        // ---- TURN-TAKING (Subtask 1 only) ----
        while (reader_turn != my)
        {
            if (global_err)
            {
                std::free(line);
                return nullptr;
            }
            polite_yield();
        }

        // If someone already hit EOF, pass the token and exit.
        if (done_reading)
        {
            reader_turn = (my + 1) % a->n;
            break;
        }

        // ---- READ ONE LINE ----
        // TODO(Subtask 2): Wrap getline with in_mtx:
        //   pthread_mutex_lock(&in_mtx);
        errno = 0;
        // POSIX getline: includes '\n' if present; buffer must be freed by caller. :contentReference[oaicite:2]{index=2}
        ssize_t nread = getline(&line, &cap, fin);
        //   pthread_mutex_unlock(&in_mtx);

        if (nread < 0)
        {
            if (feof(fin))
            {
                // TODO(Subtask 3): set done_reading under q_mtx, then broadcast not_empty.
                //   pthread_mutex_lock(&q_mtx);
                done_reading = 1;
                //   pthread_cond_broadcast(&not_empty);
                //   pthread_mutex_unlock(&q_mtx);
                reader_turn = (my + 1) % a->n;
                break;
            }
            else
            {
                std::fprintf(stderr, "read error: %s\n", std::strerror(errno));
                global_err = 1;
                reader_turn = (my + 1) % a->n;
                break;
            }
        }

        // ---- ENQUEUE (bounded: capacity 20) ----
        // Subtask 1: spin if full
        while (queue_full())
        {
            if (global_err)
            {
                std::free(line);
                return nullptr;
            }
            polite_yield();
        }
        // TODO(Subtask 2): Replace the above spin with: lock q_mtx -> check -> enqueue -> unlock
        //   (still busy-waiting when full by unlocking and retrying; no condvars yet).
        // TODO(Subtask 3): With condvars, do:
        //   pthread_mutex_lock(&q_mtx);
        //   while (queue_full()) pthread_cond_wait(&not_full, &q_mtx);
        //   enqueue_line(line, static_cast<size_t>(nread)); // q_mtx still held
        //   pthread_cond_signal(&not_empty);                // wake a writer
        //   pthread_mutex_unlock(&q_mtx);
        enqueue_line(line, static_cast<size_t>(nread));

        // Hand turn to next reader (Subtask 1 only; remove in Subtask 3)
        reader_turn = (my + 1) % a->n;
    }

    std::free(line);
    return nullptr;
}

static void *writer_main(void *vp)
{
    auto *a = static_cast<ThreadArg *>(vp);
    const int my = a->id;

    for (;;)
    {
        // ---- TURN-TAKING (Subtask 1 only) ----
        while (writer_turn != my)
        {
            if (global_err)
                return nullptr;
            polite_yield();
        }

        // If queue empty and readers are done, finish.
        if (queue_empty() && done_reading)
        {
            writer_turn = (my + 1) % a->n;
            break;
        }

        // If empty but not done, let another writer try later.
        if (queue_empty())
        {
            writer_turn = (my + 1) % a->n;
            continue;
        }

        // ---- DEQUEUE ONE LINE ----
        // TODO(Subtask 2): Replace empty check + dequeue with lock q_mtx → check → dequeue → unlock.
        // TODO(Subtask 3): With condvars, do:
        //   pthread_mutex_lock(&q_mtx);
        //   while (queue_empty() && !done_reading) pthread_cond_wait(&not_empty, &q_mtx);
        //   if (queue_empty() && done_reading) { pthread_mutex_unlock(&q_mtx); break; }
        //   dequeue_line(&buf, &len);  // while holding q_mtx
        //   pthread_cond_signal(&not_full);
        //   pthread_mutex_unlock(&q_mtx);
        char *buf = nullptr;
        size_t len = 0;
        if (!dequeue_line(&buf, &len))
        {
            writer_turn = (my + 1) % a->n;
            continue;
        }

        // ---- WRITE EXACT BYTES (keep a line together) ----
        // TODO(Subtask 2): Wrap this whole fwrite section with out_mtx lock/unlock
        //   so one writer's line isn't interleaved with another.
        size_t off = 0;
        while (off < len)
        {
            size_t wrote = std::fwrite(buf + off, 1, len - off, fout);
            if (wrote == 0)
            {
                if (std::ferror(fout))
                {
                    std::fprintf(stderr, "write error\n");
                    global_err = 1;
                    break;
                }
            }
            off += wrote;
        }
        std::free(buf);

        // Hand turn to next writer (Subtask 1 only; remove in Subtask 3)
        writer_turn = (my + 1) % a->n;
        if (global_err)
            break;
    }
    return nullptr;
}

static void usage(const char *prog)
{
    std::fprintf(stderr, "Usage: %s n source_file destination_file\n", prog);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *endp = nullptr;
    long n = std::strtol(argv[1], &endp, 10);
    if (!endp || *endp != '\0' || n < 2 || n > 10)
    {
        std::fprintf(stderr, "n must be an integer in [2,10]\n");
        return EXIT_FAILURE;
    }

    fin = std::fopen(argv[2], "r");
    if (!fin)
    {
        std::fprintf(stderr, "cannot open source '%s': %s\n", argv[2], std::strerror(errno));
        return EXIT_FAILURE;
    }
    fout = std::fopen(argv[3], "w");
    if (!fout)
    {
        std::fprintf(stderr, "cannot open destination '%s': %s\n", argv[3], std::strerror(errno));
        std::fclose(fin);
        return EXIT_FAILURE;
    }

    // ---- INIT THREADING PRIMITIVES ----
    // TODO(Subtask 2): Initialize mutexes here (check return codes):
    //   pthread_mutex_init(&q_mtx,  nullptr);
    //   pthread_mutex_init(&in_mtx, nullptr);
    //   pthread_mutex_init(&out_mtx,nullptr);
    // TODO(Subtask 3): Initialize condition variables here:
    //   pthread_cond_init(&not_full,  nullptr);
    //   pthread_cond_init(&not_empty, nullptr);

    // Initialize tokens (Subtask 1 only)
    reader_turn = 0;
    writer_turn = 0;

    vector<pthread_t> rthreads(static_cast<size_t>(n));
    vector<pthread_t> wthreads(static_cast<size_t>(n));
    vector<ThreadArg> rargs(static_cast<size_t>(n));
    vector<ThreadArg> wargs(static_cast<size_t>(n));

    for (long i = 0; i < n; ++i)
    {
        rargs[static_cast<size_t>(i)] = ThreadArg{static_cast<int>(i), static_cast<int>(n)};
        int rc = pthread_create(&rthreads[static_cast<size_t>(i)], nullptr, reader_main, &rargs[static_cast<size_t>(i)]);
        if (rc)
            std::fprintf(stderr, "pthread_create(reader): %s\n", std::strerror(rc));
    }
    for (long i = 0; i < n; ++i)
    {
        wargs[static_cast<size_t>(i)] = ThreadArg{static_cast<int>(i), static_cast<int>(n)};
        int rc = pthread_create(&wthreads[static_cast<size_t>(i)], nullptr, writer_main, &wargs[static_cast<size_t>(i)]);
        if (rc)
            std::fprintf(stderr, "pthread_create(writer): %s\n", std::strerror(rc));
    }

    for (pthread_t t : rthreads)
    {
        int rc = pthread_join(t, nullptr);
        if (rc)
            std::fprintf(stderr, "pthread_join(reader): %s\n", std::strerror(rc));
    }
    for (pthread_t t : wthreads)
    {
        int rc = pthread_join(t, nullptr);
        if (rc)
            std::fprintf(stderr, "pthread_join(writer): %s\n", std::strerror(rc));
    }

    if (std::fflush(fout) == EOF)
    {
        std::fprintf(stderr, "fflush error: %s\n", std::strerror(errno));
        global_err = 1;
    }
    std::fclose(fout);
    std::fclose(fin);

    // ---- CLEANUP ----
    // TODO(Subtask 3): Destroy condvars:
    //   pthread_cond_destroy(&not_full);
    //   pthread_cond_destroy(&not_empty);
    // TODO(Subtask 2): Destroy mutexes:
    //   pthread_mutex_destroy(&out_mtx);
    //   pthread_mutex_destroy(&in_mtx);
    //   pthread_mutex_destroy(&q_mtx);

    return global_err ? EXIT_FAILURE : EXIT_SUCCESS;
}
