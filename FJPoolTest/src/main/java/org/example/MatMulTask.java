package org.example;

import java.util.concurrent.CountedCompleter;
import java.util.concurrent.ForkJoinTask;

class MatMulTask extends CountedCompleter<long[][]> {

  static int MAX_ARRAY_SIZE = 8;

  private final long[][] A;
  private final long[][] B;
  private final int rowA;
  private final int colA;
  private final int rowB;
  private final int colB;
  private final int size;
  long[][] result;

  // All the child tasks.
  ForkJoinTask<long[][]> A1;
  ForkJoinTask<long[][]> A2;
  ForkJoinTask<long[][]> B1;
  ForkJoinTask<long[][]> B2;
  ForkJoinTask<long[][]> C1;
  ForkJoinTask<long[][]> C2;
  ForkJoinTask<long[][]> D1;
  ForkJoinTask<long[][]> D2;

  MatMulTask(
      MatMulTask t, long[][] A, long[][] B, int rowA, int colA, int rowB, int colB, int size) {
    super(t);
    this.A = A;
    this.B = B;
    this.rowA = rowA;
    this.colA = colA;
    this.rowB = rowB;
    this.colB = colB;
    this.size = size;
  }

  @Override
  public void compute() {

    long[][] C = new long[size][size];
    if (size <= MAX_ARRAY_SIZE) {
      C = RecursiveMultiplyMatrix.multiply(A, B, rowA, colA, rowB, colB, size);

    } else {
      int newSize = size / 2;
      addToPendingCount(8);

      A1 = new MatMulTask(this, A, B, rowA, colA, rowB, colB, newSize).fork();
      A2 = new MatMulTask(this, A, B, rowA, colA + newSize, rowB + newSize, colB, newSize).fork();

      B1 = new MatMulTask(this, A, B, rowA, colA, rowB, colB + newSize, newSize).fork();
      B2 =
          new MatMulTask(this, A, B, rowA, colA + newSize, rowB + newSize, colB + newSize, newSize)
              .fork();

      C1 = new MatMulTask(this, A, B, rowA + newSize, colA, rowB, colB, newSize).fork();
      C2 =
          new MatMulTask(this, A, B, rowA + newSize, colA + newSize, rowB + newSize, colB, newSize)
              .fork();

      D1 = new MatMulTask(this, A, B, rowA + newSize, colA, rowB, colB + newSize, newSize).fork();
      D2 =
          new MatMulTask(
                  this,
                  A,
                  B,
                  rowA + newSize,
                  colA + newSize,
                  rowB + newSize,
                  colB + newSize,
                  newSize)
              .fork();
    }

    this.result = C;
    tryComplete();
  }

  @Override
  public void onCompletion(CountedCompleter<?> caller) {
    if (size <= MAX_ARRAY_SIZE) {
      return;
    }
    int newSize = size / 2;
    long[][] C = new long[size][size];

    RecursiveMultiplyMatrix.add(C, D1.getRawResult(), D2.getRawResult(), newSize, newSize);
    RecursiveMultiplyMatrix.add(C, C1.getRawResult(), C2.getRawResult(), newSize, 0);
    RecursiveMultiplyMatrix.add(C, B1.getRawResult(), B2.getRawResult(), 0, newSize);
    RecursiveMultiplyMatrix.add(C, A1.getRawResult(), A2.getRawResult(), 0, 0);

    A1 = A2 = B1 = B2 = C1 = C2 = D1 = D2 = null;
    result = C;
  }

  @Override
  public long[][] getRawResult() {
    return result;
  }
}
