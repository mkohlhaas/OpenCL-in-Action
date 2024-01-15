#include "mmio.h"
#include <stdio.h>
#include <stdlib.h>

#define MATRIX_FILE "bcsstk05.mtx"

/* Rearrange data to be sorted by row instead of by column */
void sort(int num, int *rows, int *cols, float *values) {
  int index = 0;
  for (int i = 0; i < num; i++) {
    for (int j = index; j < num; j++) {
      if (rows[j] == i) {
        if (j == index) {
          index++;
        }

        /* Swap row/column/values as necessary */
        else if (j > index) {
          int int_swap = rows[index];
          rows[index] = rows[j];
          rows[j] = int_swap;

          int_swap = cols[index];
          cols[index] = cols[j];
          cols[j] = int_swap;

          float float_swap = values[index];
          values[index] = values[j];
          values[j] = float_swap;
          index++;
        }
      }
    }
  }
}

FILE *openMatrixFile(char *matrix_file) {
  FILE *mm_handle = fopen(matrix_file, "r");
  if (!mm_handle) {
    perror("Couldn't open the MatrixMarket file.");
    exit(EXIT_FAILURE);
  }
  return mm_handle;
}

void printMatrixCharacteristics(FILE *mm_handle, MM_typecode *code) {
  mm_read_banner(mm_handle, code);
  if (mm_is_matrix(*code)) {
    printf("This is a matrix.\n");

    if (mm_is_sparse(*code)) {
      printf("It is sparse, ");
    } else {
      printf("It is dense, ");
    }

    if (mm_is_complex(*code)) {
      printf("complex-valued, ");
    } else {
      printf("real-valued, ");
    }

    if (mm_is_symmetric(*code)) {
      printf("and symmetric.\n");
    } else {
      printf("and not symmetric.\n");
    }
  } else {
    printf("This is not a matrix.\n");
  }
}

void getMatrixDimensions(FILE *mm_handle, MM_typecode *code, int *num_rows, int *num_cols, int *num_values) {
  mm_read_mtx_crd_size(mm_handle, num_rows, num_cols, num_values);
  if (mm_is_symmetric(*code) || mm_is_skew(*code) || mm_is_hermitian(*code)) {
    *num_values += *num_values - *num_rows;
  }
  printf("It has %d rows, %d columns, and %d non-zero elements.\n", *num_rows, *num_cols, *num_values);
}

int main(int argc, char *argv[]) {

  FILE *mm_handle = openMatrixFile(MATRIX_FILE);

  MM_typecode code;
  printMatrixCharacteristics(mm_handle, &code);

  int num_rows;
  int num_cols;
  int num_values;
  getMatrixDimensions(mm_handle, &code, &num_rows, &num_cols, &num_values);

  // clang-format off
  int   *rows   = malloc(num_values * sizeof(int));
  int   *cols   = malloc(num_values * sizeof(int));
  float *values = malloc(num_values * sizeof(float));
  // clang-format on

  /* Read matrix data, sort data, and close file */
  int i = 0;
  while (i < num_values) {
    fscanf(mm_handle, "%d %d %f\n", &rows[i], &cols[i], &values[i]);
    cols[i]--;
    rows[i]--;
    if ((rows[i] != cols[i]) && (mm_is_symmetric(code) || mm_is_skew(code) || mm_is_hermitian(code))) {
      i++;
      rows[i] = cols[i - 1];
      cols[i] = rows[i - 1];
      values[i] = values[i - 1];
    }
    i++;
  }
  sort(num_values, rows, cols, values);
  fclose(mm_handle);

  for (i = 0; i < num_values; i++) {
    printf("(%d, %d): %f\n", rows[i], cols[i], values[i]);
  }

  free(rows);
  free(cols);
  free(values);
}
