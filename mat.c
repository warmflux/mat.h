#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mat.h"


const char *mat_strerror(MatStatus st) {
    switch (st) {
    case MAT_OK:           return "ok";
    case MAT_ERR_SHAPE:    return "matrix shape mismatch";
    case MAT_ERR_ALLOC:    return "out of memory";
    case MAT_ERR_NULL:     return "null pointer argument";
    case MAT_ERR_INVALID:  return "invalid or uninitialized matrix";
    case MAT_ERR_SINGULAR: return "matrix is singular";
    case MAT_ERR_SIZE:     return "invalid matrix size";
    }
    return "unknown error";
}


void mat_print(Mat mat) {
    for (size_t i = 0; i < mat.row; ++i) {
        for (size_t j = 0; j < mat.col; ++j) {
            printf("%.3f  ",mat.data[i * mat.col + j]);
        }
        printf("\n");
    }
}

static float *mat_alloc(uint32_t row, uint32_t col) {
    size_t n = (size_t)row * col;
    if (n == 0 || n > SIZE_MAX / sizeof(float)) {
        return NULL;
    }
    return (float *)malloc(sizeof(float) * n);
}

static void mat_install(Mat *out, uint32_t row, uint32_t col, float *new_data) {
    free(out->data);
    out->data = new_data;
    out->row = row;
    out->col = col;
}

int mat_valid(Mat m) {
    return m.data != NULL && m.row > 0 && m.col > 0;
}

static MatStatus mat_alloc_into(uint32_t row, uint32_t col, Mat *out) {
    if (out == NULL) {
        return MAT_ERR_NULL;
    }
    if (row == 0 || col == 0) {
        return MAT_ERR_SIZE;
    }

    float *data = mat_alloc(row, col);
    if (data == NULL) {
        return MAT_ERR_ALLOC;
    }

    mat_install(out, row, col, data);
    return MAT_OK;
}


MatStatus mat_zeros(uint32_t row, uint32_t col, Mat *out) {
    MatStatus st = mat_alloc_into(row, col, out);
    if (st != MAT_OK) {
        return st;
    }

    memset(out->data, 0, sizeof(float) * (size_t)row * col);
    return MAT_OK;
}


MatStatus mat_create(uint32_t row, uint32_t col, const float *init, Mat *out) {
    if (init == NULL) {
        return mat_zeros(row, col, out);
    }

    MatStatus st = mat_alloc_into(row, col, out);
    if (st != MAT_OK) {
        return st;
    }

    memcpy(out->data, init, sizeof(float) * (size_t)row * col);
    return MAT_OK;
}

MatStatus mat_ones(uint32_t row, uint32_t col, Mat *out) {
    MatStatus st = mat_alloc_into(row, col, out);
    if (st != MAT_OK) {
        return st;
    }

    size_t n = (size_t)row * col;
    for (size_t i = 0; i < n; ++i) {
        out->data[i] = 1.0f;
    }
    return MAT_OK;
}

MatStatus mat_eye(uint32_t n, Mat *out) {
    MatStatus st = mat_zeros(n, n, out);
    if (st != MAT_OK) {
        return st;
    }

    for (uint32_t i = 0; i < n; ++i) {
        out->data[(size_t)i * out->col + i] = 1.0f;
    }
    return MAT_OK;
}



MatStatus mat_I(uint32_t row, uint32_t col, Mat *out) {
    MatStatus st = mat_zeros(row, col, out);
    if (st != MAT_OK) {
        return st;
    }

    uint32_t k = row < col ? row : col;
    for (uint32_t i = 0; i < k; ++i) {
        out->data[(size_t)i * col + i] = 1.0f;
    }
    return MAT_OK;
}

MatStatus mat_T(const Mat *a, Mat *out) {
    if (a == NULL || out == NULL) return MAT_ERR_NULL;
    if (!mat_valid(*a)) return MAT_ERR_INVALID;

    float *new_data = mat_alloc(a->col, a->row);
    if (new_data == NULL) {
        fprintf(stderr, "mat_T: out of memory!");
        return MAT_ERR_ALLOC;
    }

    for (uint32_t i = 0; i < a->row; ++i) {
        for (uint32_t j = 0; j < a->col; ++j) {
            new_data[(size_t)j * a->row + i] = a->data[(size_t)i * a->col + j];
        }
    }

    mat_install(out, a->col, a->row, new_data);
    return MAT_OK;
}


MatStatus mat_add(Mat a, Mat b, Mat *c) {
    if (c == NULL) return MAT_ERR_NULL;
    if (!mat_valid(a) || !mat_valid(b)) return MAT_ERR_INVALID;

    if (a.row != b.row || a.col != b.col) {
        fprintf(stderr,"these mat dosen't have same col and row!");
        return MAT_ERR_SHAPE;
    }

    float *new_data = mat_alloc(a.row, a.col);
    if (new_data == NULL) {
        fprintf(stderr, "mat_add: out of memory!");
        return MAT_ERR_ALLOC;
    }

    size_t n = (size_t)a.row * a.col;
    for (size_t i = 0; i < n; ++i) {
        new_data[i] = a.data[i] + b.data[i];
    }

    mat_install(c, a.row, a.col, new_data);
    return MAT_OK;
}

MatStatus mat_sub(Mat a, Mat b, Mat *c) {
    if (c == NULL) return MAT_ERR_NULL;
    if (!mat_valid(a) || !mat_valid(b)) return MAT_ERR_INVALID;
    if (a.row != b.row || a.col != b.col) return MAT_ERR_SHAPE;
    

    float *new_data = mat_alloc(a.row, a.col);
    if (new_data == NULL) {
        return MAT_ERR_ALLOC;
    }

    size_t n = (size_t)a.row * a.col;
    for (size_t i = 0; i < n; ++i) {
        new_data[i] = a.data[i] - b.data[i];
    }

    mat_install(c, a.row, a.col, new_data);
    return MAT_OK;
}



MatStatus mat_mul(Mat a, Mat b, Mat *c) {
    if (c == NULL) return MAT_ERR_NULL;
    if (!mat_valid(a) || !mat_valid(b)) return MAT_ERR_INVALID;

    if (a.col != b.row) return MAT_ERR_SHAPE;

    float *new_data = mat_alloc(a.row, b.col);
    if (new_data == NULL) return MAT_ERR_ALLOC;

    for (size_t i = 0; i < a.row; ++i) {
        for (size_t k = 0; k < a.col; ++k) {
            for (size_t j = 0; j < b.col; ++j) {
                new_data[i * b.col + j] += a.data[i * a.col + k] * b.data[k * b.col + j];
            }
        }
    }

    mat_install(c, a.row, b.col, new_data);
    return MAT_OK;
}


MatStatus mat_fill(Mat *a, float v) {
    if (a == NULL) return MAT_ERR_NULL;
    if (!mat_valid(*a)) return MAT_ERR_INVALID;

    size_t n = (size_t)a->row * a->col;
    for (size_t i = 0; i < n; i++) {
        a->data[i] = v;
    }

    return MAT_OK;
}

MatStatus mat_fill_new(uint32_t row, uint32_t col, float v, Mat *out) {
    MatStatus st = mat_alloc_into(row, col, out);
    if (st != MAT_OK) return st;

    size_t n = (size_t) row * col;
    for (size_t i = 0; i < n; ++i) {
        out->data[i] = v;
    }
    return MAT_OK;

}

MatStatus mat_diag(uint32_t n, const float *v, Mat *out);
MatStatus mat_get_diag(const Mat *a, Mat *out);
MatStatus mat_copy(const Mat *a, Mat *out);



void mat_free(Mat *mat) {
    if (mat == NULL) return;
    free(mat->data);
    mat->data = NULL;
    mat->row = 0;
    mat->col = 0;
}
