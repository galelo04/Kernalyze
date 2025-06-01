# 🖥️ Kernalyze - Operating System Simulation

Kernalyze is a comprehensive operating system simulation that demonstrates scheduling algorithms, memory management techniques, and inter-process communication. This project simulates a mini-kernel implementing various process scheduling algorithms and memory allocation techniques.

## ✨ Features and Capabilities

### 🔍 What Kernalyze Supports

-   Multiple CPU scheduling algorithms (Round Robin, SRTN, HPF)
-   Buddy memory allocation system with dynamic memory management
-   Process creation, termination, and state transitions
-   Inter-process communication through various POSIX mechanisms
-   Real-time performance metrics and logging
-   Configurable simulation parameters (time quantum, scheduling algorithm)

## 🧩 Components

### 🔄 Process Management

-   **Process Generator**: Creates processes based on input file specifications
-   **Scheduler**: Implements different scheduling algorithms
-   **Process**: Simulated process that consumes CPU time

### ⏱️ Scheduling Algorithms

1. **Round Robin (RR)**: Time-sharing algorithm with fixed time quantum
2. **Shortest Remaining Time Next (SRTN)**: Preemptive version of SJF
3. **Highest Priority First (HPF)**: Priority-based scheduling

### 💾 Memory Management

-   **Buddy Allocation System**: Efficient memory allocation with minimal fragmentation
-   **Memory Logger**: Tracks memory allocation and deallocation

### 🧰 Utilities

-   **Clock Module**: Simulates system clock for synchronization
-   **Logger**: Records scheduling events and performance metrics
-   **Data Structures**: Custom implementations for various needs

## 🏗️ Project Architecture

![architecture](https://github.com/user-attachments/assets/b5027eaa-0686-4fdc-ac1b-f05296617c53)

## ⚙️ How It Works

1. **Initialization**:

    - Process Generator initializes the system
    - Creates Clock and Scheduler processes
    - Reads process data from input file

2. **Process Creation**:

    - Process Generator spawns processes at their arrival times
    - Checks if memory can be allocated for each process

3. **Scheduling**:

    - Scheduler selects processes based on the chosen algorithm
    - Manages process states (Ready, Running, Finished)
    - Handles context switching

4. **Memory Management**:

    - Allocates memory using Buddy System algorithm
    - Tracks memory usage and handles fragmentation
    - Releases memory when processes terminate

5. **Performance Metrics**:
    - CPU utilization
    - Average waiting time
    - Average weighted turnaround time
    - Standard deviation of weighted turnaround time

## 🏃 How to Run Locally

### 📋 Prerequisites

-   Linux/Unix-based operating system
-   GCC compiler
-   Make

### 🔨 Building the Project

```bash
# Clone the repository
git clone https://github.com/galelo04/kernalyze.git
cd kernalyze

# Build all components
make
```

### ▶️ Running the Simulation

```bash
# Run with Round Robin scheduling
./os-sim -s rr -q 2 -f processes.txt

# Run with Shortest Remaining Time Next
./os-sim -s srtn -f processes.txt

# Run with Highest Priority First
./os-sim -s hpf -f processes.txt
```

### 💻 Command Line Arguments

-   `-s <algorithm>`: Scheduling algorithm (rr, srtn, hpf)
-   `-q <quantum>`: Time quantum for Round Robin (required for RR)
-   `-f <file>`: Input file with process data

### 📊 Generating Test Data

```bash
# Compile the test generator
make test_generator

# Run the test generator
./bin/test_generator
```

### 📄 Input File Format

The input file should have the following format:

```
#id arrival runtime priority memsize
1  2       3       4        5
```

### 📂 Output Files

-   `scheduler.log`: Detailed log of scheduling events
-   `scheduler.perf`: Performance metrics
-   `memory.log`: Memory allocation/deallocation events

## 🎬 Video Demonstration

https://github.com/user-attachments/assets/e2d6bd71-9734-46e0-9dc3-4a4b95ac1a02

## 📁 Project Structure

-   **src/**: Source code
    -   **utils/**: Utility modules (data structures, logging)
    -   **clk.c/h**: Clock module
    -   **scheduler.c/h**: Process scheduler
    -   **process.c**: Process implementation
    -   **process_generator.c**: Main coordinator
    -   **memory_allocator.c/h**: Memory management
    -   **defs.h**: Common definitions
-   **bin/**: Compiled binaries
-   **Makefile**: Build configuration
-   **test_generator.c**: Test data generator

## ⚖️ License

This project is licensed under the MIT License - see the LICENSE file for details.
