package org.concurries;

public class RecursiveMultiplyMatrix {
  static long[][] multiply(
      long[][] A, long[][] B, int rowA, int colA, int rowB, int colB, int size) {
    long[][] C = new long[size][size];
    if (size == 1) {
      C[0][0] = A[rowA][colA] * B[rowB][colB];
    } else {
      int newSize = size / 2;

      add(
          C,
          multiply(A, B, rowA, colA, rowB, colB, newSize),
          multiply(A, B, rowA, colA + newSize, rowB + newSize, colB, newSize),
          0,
          0);

      add(
          C,
          multiply(A, B, rowA, colA, rowB, colB + newSize, newSize),
          multiply(A, B, rowA, colA + newSize, rowB + newSize, colB + newSize, newSize),
          0,
          newSize);

      add(
          C,
          multiply(A, B, rowA + newSize, colA, rowB, colB, newSize),
          multiply(A, B, rowA + newSize, colA + newSize, rowB + newSize, colB, newSize),
          newSize,
          0);

      add(
          C,
          multiply(A, B, rowA + newSize, colA, rowB, colB + newSize, newSize),
          multiply(A, B, rowA + newSize, colA + newSize, rowB + newSize, colB + newSize, newSize),
          newSize,
          newSize);
    }

    return C;
  }

  static void add(long[][] C, long[][] A, long[][] B, int rowC, int colC) {
    int n = A.length;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        C[i + rowC][j + colC] = A[i][j] + B[i][j];
      }
    }
  }
}
