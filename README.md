# OS-A2-Group-3

# Tasks Completed

Task 1 - Multithreaded multiple file copying - mmcopier.cpp

Task 2 - Multithreaded single file copying - mscopier.cpp
Subtask 1: Reader/writer teams with shared queue
Subtask 2: Locks implementation
Subtask 3: Condition variables (no busy waiting)

# Locks and Sync Implementation -

File: mmcopier.cpp
Output mutex: DONE

File: mscopier.cpp
Queue mutex: DONE
Queue condition variables: DONE
File I/O mutex: DONE

# Compilation and Usage -

make all # Build both programs
make clean # Remove build files

# Task 1: Copy n files using n threads

./mmcopier 3 tests/source_dir tests/destination_dir

# Task 2: Copy single file using n readers + n writers

./mscopier 4 input.txt output.txt

# Server Compatibility

titan.csit.rmit.edu.au
jupiter.csit.rmit.edu.au
saturn.csit.rmit.edu.au

# To connect and run the program:

ssh s[your_student_number]@[server-prefix].csit.rmit.edu.au

- Enter RMIT account password.

git clone https://github.com/hd-marshall/OS-A2-Group-3

- Or upload the zip file.

scl enable devtoolset-7 bash
make all

- Or one at a time.

Run Task 1 or Task 2 commands. Above in the Compilation and Usage section.

# Locks used
All locks are used in the file mscopier.cpp

1. pthread_mutex_t q_mtx:

Used to protect access to the shared queue (queue) and sequence counters read_next_seq, write_next_seq
Location in code:
reader_main():
Lock when setting done_reading **line 83**
Lock before inserting a new line into the queue **line 90**

writer_main():
Lock before checking queue.count(write_next_seq) **line 123**

To avoid race conditions between multiple readers and writers accessing/modifying the shared queue.

2. pthread_mutex_t in_mtx;

Used to serialize access to the input file (fin) 
Location in code:
reader_main():
pthread_mutex_lock(&in_mtx); before getline(...) **line 78**

Only one reader should access the file at a time to prevent interleaved reads.

3. pthread_mutex_t out_mtx;

Used to serialize access to the output file (fout)

Location in code:
writer_main():
Lock before fwrite(...) **line 137**

To ensure that writers don’t write simultaneously and corrupt output.

# Condition Variables: Waiting for/Signaling Conditions
All condition variables are used in the file mscopier.cpp

1. pthread_cond_t not_full;

Used by readers to wait when the queue is full
Location in code:
reader_main():
**line 92**

writer_main():
**line 134**

If the queue has 20 lines (the QCAP), readers wait until a writer removes one.
A writer signals not_full when it removes a line, letting a reader continue.

2. pthread_cond_t not_empty;
Used by writers to wait when the queue is empty or the needed line is not yet ready

Location in code:
writer_main():
**line 127**

reader_main():
**line 85**
**line 113**

A writer waits until the next expected sequence (write_next_seq) is available.
A reader signals or broadcasts not_empty after adding a new line, waking up waiting writers.

## Work Log - 

| Date | Student name | Task completed | Hours |
|------|--------------|----------------|-------|
| Sept 1, 2025 | Dewan shoaib anis | Initial project setup, Task 1 planning | 2 |
| Sept 2, 2025 | Dewan shoaib anis | mmcopier.cpp implementation, threading logic | 4 |
| Sept 3, 2025 | Dewan shoaib anis | Task 2 setup, shared queue design | 3 |
| Sept 4, 2025 | [Student Name 3] | mscopier.cpp reader threads implementation |  |
| Sept 5, 2025 | [Student Name 2] | Writer threads, mutex locks implementation |  |
| Sept 6, 2025 | Faiz Mohammed Qasim | Condition variables, avoiding busy waiting | 5 |
| Sept 8, 2025 | Dewan shoaib anis | Testing, debugging, README update | 1.5 |
| Sept 8, 2025 | Faiz Mohammed Qasim | Final testing, documentation, README update | 2 |

**Total Hours:**
- Dewan shoaib anis: 11 hours
- Faiz Mohammed Qasim: 7 hours
- [Student Name 3]: Z hours
- [Student Name 4]: W hours