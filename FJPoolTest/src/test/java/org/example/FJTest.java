package org.example;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.random.RandomGenerator;

public class FJTest {

  @Test
  public void test_correctMultiplication()
      throws ExecutionException, InterruptedException, TimeoutException {
    var randomGenerator = RandomGenerator.getDefault();

    int rows = 256;
    long[][] A = new long[rows][rows];
    long[][] B = new long[rows][rows];

    int max = (int) Math.sqrt((double) Integer.MAX_VALUE / rows);

    for (var i = 0; i < rows; i++) {
      for (var j = 0; j < rows; j++) {
        A[i][j] = randomGenerator.nextInt(1, max);
        B[i][j] = randomGenerator.nextInt(1, max);
      }
    }

    var base = RecursiveMultiplyMatrix.multiply(A, B, 0, 0, 0, 0, A.length);

    try (var forkJoinPool = ForkJoinPool.commonPool()) {
      forkJoinPool.setParallelism(2);
      MatMulTask.MAX_ARRAY_SIZE = 4;
      var task = new MatMulTask(null, A, B, 0, 0, 0, 0, A.length);
      var result = forkJoinPool.submit(task);
      var fjResult = result.get(10, TimeUnit.MINUTES);

      Assertions.assertTrue(Arrays.deepEquals(base, fjResult));
    }
  }
}
