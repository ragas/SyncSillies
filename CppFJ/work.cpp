
#include "oneapi/tbb/global_control.h"
#include "oneapi/tbb/parallel_invoke.h"
#include <chrono>
#include <climits>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <random>
#include <ratio>
#include <vector>

struct TestResult {
  int matrix_size;
  int parallelism;
  int work_size;
  double duration;
};

constexpr int MAX_WORK_SIZE = 512;
constexpr int MAX_PARALLELISM = 32;
constexpr int MAX_MATRIX_SIZE = 1024;

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
parallel_multiply(const std::vector<std::vector<long>> &A,
                  const std::vector<std::vector<long>> &B, int rowA, int colA,
                  int rowB, int colB, size_t size, int work_size) {
  std::vector<std::vector<long>> C(size, std::vector<long>(size));

  if (size <= 64) {
    C = multiply(A, B, rowA, colA, rowB, colB, size);
  } else {
    int newSize = size / 2;

    oneapi::tbb::parallel_invoke(

        [&] {
          add(C,
              parallel_multiply(A, B, rowA, colA, rowB, colB, newSize,
                                work_size),
              parallel_multiply(A, B, rowA, colA + newSize, rowB + newSize,
                                colB, newSize, work_size),
              0, 0);
        },
        [&] {
          add(C,
              parallel_multiply(A, B, rowA, colA, rowB, colB + newSize, newSize,
                                work_size),
              parallel_multiply(A, B, rowA, colA + newSize, rowB + newSize,
                                colB + newSize, newSize, work_size),
              0, newSize);
        },
        [&] {
          add(C,
              parallel_multiply(A, B, rowA + newSize, colA, rowB, colB, newSize,
                                work_size),
              parallel_multiply(A, B, rowA + newSize, colA + newSize,
                                rowB + newSize, colB, newSize, work_size),
              newSize, 0);
        },
        [&] {
          add(C,
              parallel_multiply(A, B, rowA + newSize, colA, rowB,
                                colB + newSize, newSize, work_size),
              parallel_multiply(A, B, rowA + newSize, colA + newSize,
                                rowB + newSize, colB + newSize, newSize,
                                work_size),
              newSize, newSize);
        }

    );
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

std::vector<TestResult> test_size(int size) {
  int range = std::sqrt(INT_MAX / size);

  std::uniform_int_distribution<> distrib(1, range);

  std::vector<std::vector<long>> A(size, std::vector<long>(size));
  auto B = A;
  std::random_device rd;
  std::mt19937 gen(rd());

  for (auto i = 0; i < size; i++) {
    for (auto j = 0; j < size; j++) {
      A[i][j] = distrib(gen);
      B[i][j] = distrib(gen);
    }
  }

  std::vector<TestResult> results;

  for (auto parallelism = 1; parallelism <= MAX_PARALLELISM; parallelism *= 2)
    for (auto work_size = 1; work_size <= MAX_WORK_SIZE; work_size *= 2) {
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

int main() {
  std::vector<TestResult> results;

  for (int size = 64; size <= MAX_MATRIX_SIZE; size *= 2) {
    auto result = test_size(size);
    results.insert(results.end(), result.begin(), result.end());
  }

  std::cout << results.size() << "\n";

  std::string filename("performance.csv");
  std::ofstream file;
  file.open(filename);
  file << "size,parallelism,work_size,duration\n";
  for (auto result : results) {
    file << result.matrix_size << "," << result.parallelism << ","
         << result.work_size << "," << result.duration << "\n";
  }
  file.flush();
  file.close();

  return 0;
}