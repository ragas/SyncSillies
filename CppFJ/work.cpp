
#include "work.hpp"

#include "oneapi/tbb/global_control.h"
#include "oneapi/tbb/parallel_invoke.h"
#include <chrono>
#include <climits>
#include <cmath>
#include <cstddef>

#include <iostream>
#include <random>
#include <ratio>
#include <vector>

static void add(std::vector<std::vector<long>> &C,
                const std::vector<std::vector<long>> A,
                const std::vector<std::vector<long>> B, int rowC, int colC) {
  int n = A.size();
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      C[i + rowC][j + colC] = A[i][j] + B[i][j];
    }
  }
}

std::vector<std::vector<long>> multiply(const std::vector<std::vector<long>> &A,
                                        const std::vector<std::vector<long>> &B,
                                        int rowA, int colA, int rowB, int colB,
                                        size_t size) {
  std::vector<std::vector<long>> C(size, std::vector<long>(size));

  if (size == 1) {
    C[0][0] = A[rowA][colA] * B[rowB][colB];
  } else {
    int newSize = size / 2;

    add(C, multiply(A, B, rowA, colA, rowB, colB, newSize),
        multiply(A, B, rowA, colA + newSize, rowB + newSize, colB, newSize), 0,
        0);

    add(C, multiply(A, B, rowA, colA, rowB, colB + newSize, newSize),
        multiply(A, B, rowA, colA + newSize, rowB + newSize, colB + newSize,
                 newSize),
        0, newSize);

    add(C, multiply(A, B, rowA + newSize, colA, rowB, colB, newSize),
        multiply(A, B, rowA + newSize, colA + newSize, rowB + newSize, colB,
                 newSize),
        newSize, 0);

    add(C, multiply(A, B, rowA + newSize, colA, rowB, colB + newSize, newSize),
        multiply(A, B, rowA + newSize, colA + newSize, rowB + newSize,
                 colB + newSize, newSize),
        newSize, newSize);
  }

  return C;
}

std::vector<std::vector<long>>
ParallelMultiply::parallel_multiply(const std::vector<std::vector<long>> &A,
                                    const std::vector<std::vector<long>> &B,
                                    int rowA, int colA, int rowB, int colB,
                                    size_t size, int work_size) {
  std::vector<std::vector<long>> C(size, std::vector<long>(size));

  if (size <= work_size) {
    C = multiply(A, B, rowA, colA, rowB, colB, size);
  } else {
    int newSize = size / 2;

    std::vector<std::vector<long>> A1, A2, B1, B2, C1, C2, D1, D2;

    oneapi::tbb::parallel_invoke(

        [&] {
          A1 = parallel_multiply(A, B, rowA, colA, rowB, colB, newSize,
                                 work_size);
        },
        [&] {
          A2 = parallel_multiply(A, B, rowA, colA + newSize, rowB + newSize,
                                 colB, newSize, work_size);
        },
        [&] {
          B1 = parallel_multiply(A, B, rowA, colA, rowB, colB + newSize,
                                 newSize, work_size);
        },
        [&] {
          B2 = parallel_multiply(A, B, rowA, colA + newSize, rowB + newSize,
                                 colB + newSize, newSize, work_size);
        },
        [&] {
          C1 = parallel_multiply(A, B, rowA + newSize, colA, rowB, colB,
                                 newSize, work_size);
        },
        [&] {
          C2 = parallel_multiply(A, B, rowA + newSize, colA + newSize,
                                 rowB + newSize, colB, newSize, work_size);
        },
        [&] {
          D1 = parallel_multiply(A, B, rowA + newSize, colA, rowB,
                                 colB + newSize, newSize, work_size);
        },
        [&] {
          D2 = parallel_multiply(A, B, rowA + newSize, colA + newSize,
                                 rowB + newSize, colB + newSize, newSize,
                                 work_size);
        }

    );

    add(C, A1, A2, 0, 0);
    add(C, B1, B2, 0, newSize);
    add(C, C1, C2, newSize, 0);
    add(C, D1, D2, newSize, newSize);
  }

  return C;
}

void print(std::vector<std::vector<long>> M) {
  auto size = M.size();
  for (auto i = 0; i < size; i++) {
    std::cout << "{";
    for (auto j = 0; j < size; j++) {
      std::cout << M[i][j] << ",";
    }
    std::cout << "},";
  }
  std::cout << "---*----*----";
  std::cout << "\n";
}

std::random_device ParallelMultiply::rd;
std::mt19937 ParallelMultiply::gen(rd());

std::vector<std::vector<long>> ParallelMultiply::random_matrix(int size) {
  int range = std::sqrt(INT_MAX / size);
  std::uniform_int_distribution<> distrib(1, range);

  std::vector<std::vector<long>> A(size, std::vector<long>(size));
  for (auto i = 0; i < size; i++) {
    for (auto j = 0; j < size; j++) {
      A[i][j] = distrib(gen);
    }
  }
  return A;
}
std::vector<TestResult>
ParallelMultiply::parallel_mult(std::vector<std::vector<long>> A,
                                std::vector<std::vector<long>> B) {
  int size = A.size();

  std::vector<TestResult> results;

  for (auto parallelism = MIN_PARALLELISM; parallelism <= MAX_PARALLELISM;
       parallelism *= 2)
    for (auto work_size = MIN_WORK_SIZE; work_size <= MAX_WORK_SIZE;
         work_size *= 2) {
      oneapi::tbb::global_control global_limit(
          oneapi::tbb::global_control::max_allowed_parallelism, parallelism);

      const auto start = std::chrono::steady_clock::now();
      auto C = parallel_multiply(A, B, 0, 0, 0, 0, A.size(), work_size);
      const auto end = std::chrono::steady_clock::now();
      const std::chrono::duration<double, std::nano> diff = end - start;
      TestResult result{.matrix_size = size,
                        .parallelism = parallelism,
                        .work_size = work_size,
                        .duration = diff.count()};

      results.push_back(result);
    }

  return results;
}
