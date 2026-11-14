#ifndef MAT_H_
#define MAT_H_

#include <stdint.h>

typedef enum {
    MAT_OK = 0,
    MAT_ERR_SHAPE,
    MAT_ERR_ALLOC,
    MAT_ERR_NULL,
    MAT_ERR_INVALID,
    MAT_ERR_SINGULAR, // 表示矩阵奇异（不可逆 / 不满秩）
    MAT_ERR_SIZE 
} MatStatus;

typedef struct {
    uint32_t row,col;
    float *data;
}Mat;


const char *mat_strerror(MatStatus st);

void mat_print(Mat mat);
int mat_valid(Mat m);

MatStatus mat_zeros(uint32_t row, uint32_t col, Mat *out);
MatStatus mat_create(uint32_t row, uint32_t col, const float *init, Mat *out);
MatStatus mat_ones(uint32_t row, uint32_t col, Mat *out);
MatStatus mat_eye(uint32_t n, Mat *out);
MatStatus mat_I(uint32_t row, uint32_t col, Mat *out);
MatStatus mat_T(const Mat *a, Mat *out);
MatStatus mat_add(Mat a, Mat b, Mat *c);
MatStatus mat_sub(Mat a, Mat b, Mat *c);
MatStatus mat_mul(Mat a, Mat b, Mat *c);
MatStatus mat_fill(Mat *a, float v);
MatStatus mat_fill_new(uint32_t row, uint32_t col, float v, Mat *out);
MatStatus mat_diag(uint32_t n, const float *v, Mat *out);
MatStatus mat_get_diag(const Mat *a, Mat *out);
MatStatus mat_copy(const Mat *a, Mat *out);
void mat_free(Mat *mat);

#endif // MAT_H_
