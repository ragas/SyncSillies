package org.example;

import org.apache.commons.csv.CSVFormat;
import org.apache.commons.csv.CSVPrinter;

import java.io.FileWriter;
import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.concurrent.*;
import java.util.random.RandomGenerator;

public class Main {

  static final int MIN_MATRIX_SIZE = 64;
  static final int MAX_MATRIX_SIZE = 512;
  static final int MAX_WORK_SIZE_TEST = 256;
  static final int MAX_PARALLELISM_TEST = 32;

  public static void main(String[] args) {
    try {
      test();
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  static void test() throws Exception {
    ArrayList<TestResult> results = new ArrayList<>();
    for (var rows = MIN_MATRIX_SIZE; rows <= MAX_MATRIX_SIZE; rows = rows * 2) {
      var result = testSize(rows);
      results.addAll(result);
      System.out.printf("Done matrix size:%d\n", rows);
    }

    try (CSVPrinter printer =
        new CSVPrinter(new FileWriter("performance.csv"), CSVFormat.DEFAULT)) {
      printer.printRecord("size", "max_work_size", "duration", "parallelism", "steal_count", "tag");
      for (var r : results) {
        printer.printRecord(
            r.matrixSize,
            r.maxWorkSize,
            r.duration.getSeconds() * 1_000_000_000 + r.duration.getNano(),
            r.parallelism,
            r.stealCount,
            r.tag);
      }
    }
  }

  static ArrayList<TestResult> testSize(int rows) throws Exception {

    var randomGenerator = RandomGenerator.getDefault();

    long[][] A = new long[rows][rows];
    long[][] B = new long[rows][rows];

    int max = (int) Math.sqrt((double) Integer.MAX_VALUE / rows);

    for (var i = 0; i < rows; i++) {
      for (var j = 0; j < rows; j++) {
        A[i][j] = randomGenerator.nextInt(1, max);
        B[i][j] = randomGenerator.nextInt(1, max);
      }
    }

    var start = System.nanoTime();
    var ignore = RecursiveMultiplyMatrix.multiply(A, B, 0, 0, 0, 0, A.length);
    var end = System.nanoTime();

    var baseLine = Duration.of(end - start, ChronoUnit.NANOS);
    ArrayList<TestResult> results = new ArrayList<>();

    var baseResult = new TestResult(rows, 0, 0, baseLine, 0, "baseline");
    results.add(baseResult);

    for (int parallel = 1; parallel <= MAX_PARALLELISM_TEST; parallel = parallel * 2) {
      for (int split = 1; split <= MAX_WORK_SIZE_TEST; split = split * 2) {
        var resultWorkSize = test_forkJoinPool(A, B, parallel, split);
        results.add(resultWorkSize.withTag("work_size"));
      }
    }

    return results;
  }

  static TestResult test_forkJoinPool(long[][] A, long[][] B, int parallelism, int split)
      throws Exception {
    try (var forkJoinPool = ForkJoinPool.commonPool()) {
      forkJoinPool.setParallelism(parallelism);

      MatMulTask.MAX_ARRAY_SIZE = split;
      var task = new MatMulTask(null, A, B, 0, 0, 0, 0, A.length);
      var start = System.nanoTime();
      var result = forkJoinPool.submit(task);
      var ignore = result.get(10, TimeUnit.MINUTES);

      var end = System.nanoTime();
      return new TestResult(
          Duration.of(end - start, ChronoUnit.NANOS),
          parallelism,
          split,
          A.length,
          forkJoinPool.getStealCount());
    }
  }

  record TestResult(
      int matrixSize,
      int parallelism,
      int maxWorkSize,
      Duration duration,
      long stealCount,
      String tag) {
    TestResult(
        Duration duration, int parallelism, int maxWorkSize, int matrixSize, long stealCount) {
      this(matrixSize, parallelism, maxWorkSize, duration, stealCount, null);
    }

    TestResult withTag(String tag) {
      return new TestResult(matrixSize, parallelism, maxWorkSize, duration, stealCount, tag);
    }
  }
}
