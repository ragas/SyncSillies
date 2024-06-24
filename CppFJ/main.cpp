#include "work.hpp"
#include <fstream>
#include <iostream>
constexpr int MIN_MATRIX_SIZE = 64;
constexpr int MAX_MATRIX_SIZE = 512;

const int ParallelMultiply::MIN_WORK_SIZE = 1;
const int ParallelMultiply::MAX_WORK_SIZE = 128;

const int ParallelMultiply::MIN_PARALLELISM = 1;
const int ParallelMultiply::MAX_PARALLELISM = 32;

int main() {
  std::vector<TestResult> results;

  ParallelMultiply parallelMultiply;

  for (int size = MIN_MATRIX_SIZE; size <= MAX_MATRIX_SIZE; size *= 2) {
    auto A = ParallelMultiply::random_matrix(size);
    auto B = ParallelMultiply::random_matrix(size);
    auto result = ParallelMultiply::parallel_mult(A, B);
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