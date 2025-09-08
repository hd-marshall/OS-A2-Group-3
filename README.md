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

## Work Log -

| Date         | Student name      | Task completed                                                                                    | Hours |
| ------------ | ----------------- | ------------------------------------------------------------------------------------------------- | ----- |
| Aug 28, 2025 | Harry Marshall    | Create teams channel to communicate                                                               | 0.5   |
| Sept 1, 2025 | Dewan shoaib anis | Initial project setup, Task 1 planning                                                            | 2     |
| Sept 2, 2025 | Dewan shoaib anis | mmcopier.cpp implementation, threading logic                                                      | 4     |
| Sept 3, 2025 | Dewan shoaib anis | Task 2 setup, shared queue design                                                                 | 3     |
| Sept 3, 2025 | Dewan shoaib anis | Initial project setup, Task 1 planning                                                            | 2     |
| Sept 3, 2025 | Harry Marshall    | Create Github repo and Assignment folder structure                                                | 0.5   |
| Sept 3, 2025 | Harry Marshall    | Create Makefile for both files                                                                    | 0.5   |
| Sept 4, 2025 | Harry Marshall    | Create Readme.md, test and validate how to use RMIT servers, create instructions on how to use it | 3     |
| Sept 4, 2025 | [Student Name 3]  | mscopier.cpp reader threads implementation                                                        |       |
| Sept 5, 2025 | Harry Marshall    | Writer threads, mutex locks implementation                                                        | 4     |
| Sept 6, 2025 | [Student Name 4]  | Condition variables, avoiding busy waiting                                                        |       |
| Sept 8, 2025 | Dewan shoaib anis | Testing, debugging, README update                                                                 | 1.5   |
| Sept 8, 2025 | [Student Name 2]  | Final testing, documentation, README update                                                       |       |

**Total Hours:**

- Harry Marshall: 8.5 hours
- Ahrar Hossain: Z hours
- Faiz Qasim: W hours
- Dewan shoaib anis: 11 hours
