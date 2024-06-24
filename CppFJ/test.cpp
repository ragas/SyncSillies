#include "catch2/benchmark/catch_benchmark_all.hpp"
#include "catch2/catch_test_macros.hpp"
#include "work.hpp"
constexpr int MIN_MATRIX_SIZE = 256;
constexpr int MAX_MATRIX_SIZE = 256;

const int ParallelMultiply::MIN_WORK_SIZE = 16;
const int ParallelMultiply::MAX_WORK_SIZE = 16;

const int ParallelMultiply::MIN_PARALLELISM = 8;
const int ParallelMultiply::MAX_PARALLELISM = 8;

TEST_CASE("Check the multiplication values are same.") {
  int size = 256;
  auto A = ParallelMultiply::random_matrix(size);
  auto B = ParallelMultiply::random_matrix(size);
  auto X = ParallelMultiply::parallel_multiply(A, B, 0, 0, 0, 0, size, 16);
  auto Y = multiply(A, B, 0, 0, 0, 0, size);
  REQUIRE(X == Y);

  // BENCHMARK("Benchmark one run") { return ParallelMultiply::test_size(64); };
};