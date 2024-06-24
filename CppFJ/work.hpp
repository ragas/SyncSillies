
#include <cstddef>
#include <random>
#include <vector>

struct TestResult {
  int matrix_size;
  int parallelism;
  int work_size;
  double duration;
};

static void add(std::vector<std::vector<long>> &C,
                const std::vector<std::vector<long>> A,
                const std::vector<std::vector<long>> B, int rowC, int colC);
std::vector<std::vector<long>> multiply(const std::vector<std::vector<long>> &A,
                                        const std::vector<std::vector<long>> &B,
                                        int rowA, int colA, int rowB, int colB,
                                        size_t size);

class ParallelMultiply {
  static const int MIN_WORK_SIZE;
  static const int MAX_WORK_SIZE;
  static const int MIN_PARALLELISM;
  static const int MAX_PARALLELISM;

  static std::random_device rd;
  static std::mt19937 gen;

public:
  ParallelMultiply() {}

  std::vector<std::vector<long>> static parallel_multiply(
      const std::vector<std::vector<long>> &A,
      const std::vector<std::vector<long>> &B, int rowA, int colA, int rowB,
      int colB, size_t size, int work_size);
  static std::vector<TestResult>
  parallel_mult(std::vector<std::vector<long>> A,
                std::vector<std::vector<long>> B);
  static std::vector<std::vector<long>> random_matrix(int size);
};
