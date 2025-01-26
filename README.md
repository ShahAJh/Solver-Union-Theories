# Implementation of a Solver for the Union of Theories of Equality, Lists, and Arrays

## Overview
This project implements a solver for determining the satisfiability of a set of literals in the union of quantifier-free fragments of three theories:

1. **Equality with free symbols**
2. **Non-empty, potentially cyclic lists**
3. **Arrays without extensionality**

The solver utilizes the congruence closure algorithm enhanced with heuristics like a non-recursive FIND function, union by rank with `ccpar` prioritization, and a forbidden list. It supports input in a simple SMT-LIB-like format and synthetic test cases.

---

## Features
- **Congruence Closure Algorithm**:
  - Efficient equivalence checking with optimizations for performance and scalability.
  - Non-recursive FIND function with path compression.
  - UNION by rank and size of `ccpar` for balanced trees.

- **Array Support**:
  - Handles arrays as index-to-value mappings without requiring extensionality.

- **List Support**:
  - Detects and handles cyclic and non-cyclic lists efficiently.

- **Input Parsing**:
  - Supports a subset of SMT-LIB (QF-UF benchmarks).
  - Accepts custom input in simple SMT-LIB-like syntax.

- **Performance Logging**:
  - Logs runtime performance for each test case.

---

## Files Included

### 1. **Source Code**
- `solver.cpp`: The main source file containing the implementation of the solver.

### 2. **Test Cases**
- Folder: `test_cases`
  - `test_case_1.txt`: Basic equivalence and forbidden terms.
  - `test_case_2.txt`: Larger equivalence classes and forbidden terms.
  - `test_case_array.txt`: Tests array operations.
  - `test_case_list.txt`: Tests cyclic and non-cyclic lists.
  - `test_case_mixed.txt`: Mixed test case including forbidden terms, arrays, and lists.

### 3. **Logs**
- `solver_log.txt`: Logs merge operations, forbidden list violations, and performance metrics.

### 4. **Report**
- `Solver AR Report.pdf`: A detailed report describing the implementation, test results, performance analysis, and conclusions.

---

## Compilation and Execution

### Prerequisites
- A C++ compiler (e.g., `g++`).
- A terminal or command-line interface.

### Steps
1. **Compile the Source Code**:
   ```bash
   g++ -o solver solver.cpp
   ```

2. **Run the Solver**:
   ```bash
   ./solver
   ```

3. **Input Format**:
   - Place test case files in the `test_cases` folder.
   - Supported input includes:
     - **Equivalence**: `add T1 T2`
     - **Forbidden Terms**: `forbidden T1`
     - **Array Operations**: `set Array1 0 ValueA`
     - **Lists**: `list List1 [T1,T2,T3]`
     - **SMT-LIB Assertions**: `assert (= T1 T2)`

4. **View Output**:
   - The solver processes all test case files in the `test_cases` folder and outputs results to the terminal.
   - Performance metrics are logged in `solver_log.txt`.

---

## Example Output
```text
Running all test cases from folder: test_cases
Running test case: test_case_1.txt
Skipping merge: T1 or T2 is in the forbidden list.
Equivalence Classes:
Class 3: T6 T7 T8 T3
Class 1: T2 T5
Forbidden Terms (not part of any class): T4 T1
[Performance] Test Case: test_case_1.txt took 2 ms
=====================
```

---

## Future Work
- Extend SMT-LIB support for more complex benchmarks.
- Optimize memory usage for handling large datasets.
- Enhance the synthetic generator to create diverse test cases.

---

## References
- Bradley, A. R., & Manna, Z. (2007). *The Calculus of Computation: Decision Procedures with Applications to Verification*. Springer.
- Nelson, G., & Oppen, D. C. (1980). "Fast decision procedures based on congruence closure." *Journal of the ACM*.
- SMT-LIB benchmarks. Available at: [https://smt-lib.org](https://smt-lib.org).
- Armando, A., Ranise, S., & Bonacina, M. P. (2009). "New results on rewrite-based satisfiability procedures." *ACM Transactions on Computational Logic*.
- Bachmair, L., Tiwari, A., & Vigneron, L. (2003). "Abstract congruence closure." *Journal of Automated Reasoning*.  
