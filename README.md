# OS-A2-Group-3

# Tasks Completed

Task 1 - Multithreaded multiple file copying - mmcopier.cpp

Task 2 - Multithreaded single file copying - mscopier.cpp
Subtask 1: Reader/writer teams with shared queue
Subtask 2: Locks implementation
Subtask 3: Condition variables (no busy waiting)

# Locks and Sync Implementation -

File: mmcopier.cpp
Output mutex: FILL

File: mscopier.cpp
Queue mutex: FILL
Queue condition variables: FILL
File I/O mutex: FILL

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

To connect and run the program:

ssh s[your_student_number]@titan.csit.rmit.edu.au
git clone https://github.com/hd-marshall/OS-A2-Group-3
-Or upload the zip file.
make all
-Or one at a time.
-Run Task 1 or Task 2 commands. Above in the Compilation and Usage section.
