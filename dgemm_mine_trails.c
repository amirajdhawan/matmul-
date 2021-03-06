const char* dgemm_desc = "My awesome dgemm.";

// #include <nmmintrin.h>
#include <immintrin.h>

// Block size that is used to fit submatrices into L1 cache
#ifndef BLOCK_SIZE
#define BLOCK_SIZE ((int) 128)
#endif

// Block size that is used to fit submatrices into register
#ifndef MID_BLOCK_SIZE
#define MID_BLOCK_SIZE ((int) 32)
#endif

// Block size that is used to fit submatrices into register
#ifndef INNER_BLOCK_SIZE
#define INNER_BLOCK_SIZE ((int) 4)
#endif

// Part of AVX implementation, now for reference
// #ifdef USE_SHUFPD
// #  define swap_sse_doubles(a) _mm_shuffle_pd(a, a, 1)
// #else
// #  define swap_sse_doubles(a) (__m128d) _mm_shuffle_epi32((__m128i) a, 0x4e)
// #endif

// void mine_fma_dgemm(const double* restrict A, const double* restrict B,
//                  double* restrict C){
//     /*
//     My kernel function that utilizes the architecture of totient node.
//     It uses the 256 bits register size which accomodate 4 doubles
//     Also, it uses FMA to maximize the computational efficiency.
//     To do this, it assumes an input of A = 4*4 and B = 4*4 with output C = 4*4
//     The size can be changed later for better performance, but 4*4 will be a good choice for prototyping
//     The matrices are all assumed to be stored in column major
//      */
//
//     const int Matrix_size = 4; // 256 bits for 4*64 bits doubles
//     // A command that I got from S14 code. Helps compiler optimize
//     __assume_aligned(A, 32);
//     __assume_aligned(B, 32);
//     __assume_aligned(C, 32);
//     // Load matrix A. The load function is loadu, which doesn't require alignment of the meory
    // __m256d a0 = _mm256_loadu_pd((A + Matrix_size * 0));
    // __m256d a1 = _mm256_loadu_pd((A + Matrix_size * 1));
    // __m256d a2 = _mm256_loadu_pd((A + Matrix_size * 2));
    // __m256d a3 = _mm256_loadu_pd((A + Matrix_size * 3));
    //
    // int i;
    // for (i = 0; i < Matrix_size; i++){
    //   __m256d b = _mm256_loadu_pd((B + Matrix_size * i));
    //   __m256d c = _mm256_loadu_pd((C + Matrix_size * i));
    //   // Routine to compute four dot product once.
    //   // Credit to http://stackoverflow.com/questions/10454150/intel-avx-256-bits-version-of-dot-product-for-double-precision-floating-point
    //   __m256d xy0 = _mm256_mul_pd( a0, b );
    //   __m256d xy1 = _mm256_mul_pd( a1, b );
    //   __m256d xy2 = _mm256_mul_pd( a2, b );
    //   __m256d xy3 = _mm256_mul_pd( a3, b );
    //   // low to high: xy00+xy01 xy10+xy11 xy02+xy03 xy12+xy13
    //   __m256d temp01 = _mm256_hadd_pd( xy0, xy1 );
    //   // low to high: xy20+xy21 xy30+xy31 xy22+xy23 xy32+xy33
    //   __m256d temp23 = _mm256_hadd_pd( xy2, xy3 );
    //   // low to high: xy02+xy03 xy12+xy13 xy20+xy21 xy30+xy31
    //   __m256d swapped = _mm256_permute2f128_pd( temp01, temp23, 0x21 );
    //   // low to high: xy00+xy01 xy10+xy11 xy22+xy23 xy32+xy33
    //   __m256d blended = _mm256_blend_pd(temp01, temp23, 0b1100);
    //   __m256d dotproduct = _mm256_add_pd( swapped, blended );
    //   __m256d sum = _mm256_add_pd( c, dotproduct ); // Try to avoid latency, not sure if this does anything
    //   _mm256_storeu_pd((C + i*Matrix_size),sum); // Store C(:,i)
    // }

    // Functional AVX2 code

    // Load matrix A. The load function is loadu, which doesn't require alignment of the meory
    // __m256d a0 = _mm256_loadu_pd((A + Matrix_size * 0));
    // __m256d a1 = _mm256_loadu_pd((A + Matrix_size * 1));
    // __m256d a2 = _mm256_loadu_pd((A + Matrix_size * 2));
    // __m256d a3 = _mm256_loadu_pd((A + Matrix_size * 3));
    // __m256d bij;
    // __m256d c;
    // int i;
    // for (i = 0; i < Matrix_size; i++){
    //   // Load one column of C, C(:,i)
    //   c = _mm256_loadu_pd((C + Matrix_size*i));
    //
    //   bij = _mm256_set1_pd(*(B+i*Matrix_size+0)); // Perform FMA on A*B(:,i)
    //   c = _mm256_fmadd_pd(a0, bij, c);
    //   bij = _mm256_set1_pd(*(B+i*Matrix_size+1));
    //   c = _mm256_fmadd_pd(a1, bij, c);
    //   bij = _mm256_set1_pd(*(B+i*Matrix_size+2));
    //   c = _mm256_fmadd_pd(a2, bij, c);
    //   bij = _mm256_set1_pd(*(B+i*Matrix_size+3));
    //   c = _mm256_fmadd_pd(a3, bij, c);

    //   _mm256_storeu_pd((C+i*Matrix_size),c); // Store C(:,i)
    // }

// }


// void matrix_copy (const int mat_size, const int sub_size, const int i, const int j,
//         const double* restrict Matrix, double* restrict subMatrix){
//   // Get a copy of submatrix
//   const int sub_M = ((i+1)*sub_size > mat_size? mat_size-i*sub_size : sub_size); // Maybe we can do this outside, but I'm not worried about this right now.
//   const int sub_N = ((j+1)*sub_size > mat_size? mat_size-j*sub_size : sub_size);
//   // printf("\n For copy, M is %d, N is %d\n", M, N);
//   // Make a copy
//   int m, n;
//   for (n = 0; n < sub_N; n++){
//     for (m = 0; m < sub_M; m++){
//       subMatrix[n*sub_size + m] = Matrix[(j*sub_size+n)*mat_size + (i*sub_size+m)];
//     }
//   }
//   // Populate the submatrix with 0 to enforce regular pattern in the computation.
//   for (n = sub_N; n < sub_size; n++){
//     for (m = 0; m < sub_size; m++){
//       subMatrix[n*sub_size + m] = 0.0;
//     }
//   }
//   for (n = 0; n < sub_N; n++){
//     for (m = sub_M; m < sub_size; m++){
//       subMatrix[n*sub_size + m] = 0.0;
//     }
//   }
// }
// void matrix_transpose_copy (const int mat_size, const int sub_size, const int i, const int j,
//         const double* restrict Matrix, double* restrict subMatrix){
//   // Get a copy of submatrix
//   const int sub_M = ((i+1)*sub_size > mat_size? mat_size-i*sub_size : sub_size); // Maybe we can do this outside, but I'm not worried about this right now.
//   const int sub_N = ((j+1)*sub_size > mat_size? mat_size-j*sub_size : sub_size);
//   // Make a copy
//   int m, n;
//   for (n = 0; n < sub_N; n++){
//     for (m = 0; m < sub_M; m++){
//       subMatrix[m*sub_size + n] = Matrix[(j*sub_size+n)*mat_size + (i*sub_size+m)];
//     }
//   }
//   // Populate the submatrix with 0 to enforce regular pattern in the computation.
//   for (n = sub_N; n < sub_size; n++){
//     for (m = 0; m < sub_size; m++){
//       subMatrix[m*sub_size + n] = 0.0;
//     }
//   }
//   for (n = 0; n < sub_N; n++){
//     for (m = sub_M; m < sub_size; m++){
//       subMatrix[m*sub_size + n] = 0.0;
//     }
//   }
// }
// void matrix_update (const int mat_size, const int sub_size, const int i, const int j,
//         double* restrict Matrix, const double* restrict subMatrix){
//     int m, n;
//     const int M = ((j+1)*sub_size > mat_size? mat_size-j*sub_size : sub_size);
//     const int N = ((i+1)*sub_size > mat_size? mat_size-i*sub_size : sub_size);
//     // printf("\n M is %d, N is %d\n", M, N);
//     for (m = 0; m < M; m++){
//       for (n = 0; n < N; n++){
//         // printf("\n m is %d, n is %d\n", m, n);
//          Matrix[(j*sub_size+m)*mat_size + (i*sub_size+n)] = subMatrix[m*sub_size + n];
//       }
//     }
// }

// void submatrix_copy(const int matrix_size, const int block_size,
//         const int I, const int J, const double* restrict matrix, double* restrict submatrix){
//         // submatrix_copy(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, K_inner, J_inner, B, B_mid);
//         // Compute how much does it need to copy
//         const int M = (I + block_size) > matrix_size ? matrix_size - I : block_size;
//         const int N = (J + block_size) > matrix_size ? matrix_size - J : block_size;
//         // Make a copy
//         int m, n;
//         for (n = 0; n < N; n++){
//           for (m = 0; m < M; m++){
//             submatrix[n * block_size + m] = matrix[(J + n) * matrix_size + I + m];
//           }
//         }
//         // Populate the submatrix with 0 to enforce regular pattern in the computation.
//         for (n = N; n < block_size; n++){
//           for (m = 0; m < block_size; m++){
//             submatrix[n * block_size + m] = 0.0;
//           }
//         }
//         for (n = 0; n < N; n++){
//           for (m = M; m < block_size; m++){
//             submatrix[n * block_size + m] = 0.0;
//           }
//         }
// }
// void submatrix_transpose(const int matrix_size, const int block_size,
//         const int I, const int J, const double* restrict matrix, double* restrict submatrix){
//         // Make a transposed copy
//         // Here we assume the matrix to transpose is square and we do not need to worry
//         // about edges (because it has been taken care of in the last level)
//         int m, n;
//         for (n = 0; n < block_size; n++){
//           for (m = 0; m < block_size; m++){
//             submatrix[m * block_size + n] = matrix[(J + n) * matrix_size + I + m];
//           }
//         }
// }
// void submatrix_update(const int matrix_size, const int block_size,
//         const int I, const int J, double* restrict matrix, const double* restrict submatrix){
//         // It's basically the copy function, just in reverse direction
//         // Compute how much does it need to copy
//         const int M = (I + block_size) > matrix_size ? matrix_size - I : block_size;
//         const int N = (J + block_size) > matrix_size ? matrix_size - J : block_size;
//         // Make a copy
//         int m, n;
//         for (n = 0; n < N; n++){
//           for (m = 0; m < M; m++){
//             matrix[(J + n) * matrix_size + I + m] = submatrix[n * block_size + m] ;
//           }
//         }
// }
// void kernel_dgemm(int mid_size, int inner_size, int n_in_blocks,
//                 const double* restrict A, double* restrict sub_A, const double* restrict B, double* restrict sub_B,
//                 double* restrict C, double* restrict sub_C){
//       int sbi, sbj, sbk;
//       for (sbi = 0; sbi < n_in_blocks; sbi++) {
//         const int I_inner = sbi*inner_size; // Starting element index I for inner submatrix (relative to mid submatrix)
//         for (sbk = 0; sbk < n_in_blocks; sbk++) {
//           const int K_inner = sbk*inner_size; // Starting element index K for inner submatrix (relative to mid submatrix)
//           submatrix_transpose(mid_size, inner_size, I_inner, K_inner, A, sub_A);
//           for (sbj = 0; sbj < n_in_blocks; sbj++) {
//             const int J_inner = sbj*inner_size; // Starting element index J for inner submatrix (relative to mid submatrix)
//             submatrix_copy(mid_size, inner_size, I_inner, J_inner, C, sub_C);
//             submatrix_copy(mid_size, inner_size, K_inner, J_inner, B, sub_B);
//             mine_fma_dgemm(sub_A, sub_B, sub_C);
//             submatrix_update(mid_size, inner_size, I_inner, J_inner, C, sub_C);
//           }
//         }
//       }
// }

// void square_dgemm(const int M, const double* restrict A, const double* restrict B, double* restrict C){
//     // Functional AVX2 script with three layer blocking
//
//     // Preallocate spaces for middle submatrices A, B and C;
//     double* A_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
//     double* B_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
//     double* C_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
//     // Preallocate spaces for inner matrices A, B and C;
//     double* A_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
//     double* B_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
//     double* C_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
//     // // functional avx2 script with blocking
//     const int n_blocks = M / BLOCK_SIZE + (M%BLOCK_SIZE? 1 : 0); // # of blocks
//     const int n_mid_blocks = BLOCK_SIZE / MID_BLOCK_SIZE; // # of inner subblocks, use integer multiplier here when choosing blocksizes
//     // const int n_mid_blocks_max = M / MID_BLOCK_SIZE + (M%MID_BLOCK_SIZE? 1 : 0); // # of inner subblocks, use integer multiplier here when choosing blocksizes
//     const int n_inner_blocks = MID_BLOCK_SIZE / INNER_BLOCK_SIZE; // # of inner subblocks, use integer multiplier here when choosing blocksizes
//
//     int bi, bj, bk;
//     int mbi, mbj, mbk;
//     // int sbi, sbj, sbk;
//
//     for (bi = 0; bi < n_blocks; bi++){
//       const int I_outer = bi*BLOCK_SIZE; // Starting element index I for outer submatrix.
//       for (bj = 0; bj < n_blocks; bj++){
//         const int J_outer = bj*BLOCK_SIZE; // Starting element index J for outer submatrix.
//         for (bk = 0; bk < n_blocks; bk++){
//           const int K_outer = bk*BLOCK_SIZE; // Starting element index K for outer submatrix.
//           // Compute number of loops it takes for the mid submatrix to reach outside the boundary
//           int I_m, J_m, K_m;
//           if ((M-I_outer) < MID_BLOCK_SIZE) I_m = (M-I_outer)/MID_BLOCK_SIZE + ((M-I_outer)%MID_BLOCK_SIZE? 1 : 0);
//           else I_m = n_mid_blocks;
//           if ((M-J_outer) < MID_BLOCK_SIZE) J_m = (M-J_outer)/MID_BLOCK_SIZE + ((M-J_outer)%MID_BLOCK_SIZE? 1 : 0);
//           else J_m = n_mid_blocks;
//           if ((M-K_outer) < MID_BLOCK_SIZE) K_m = (M-K_outer)/MID_BLOCK_SIZE + ((M-K_outer)%MID_BLOCK_SIZE? 1 : 0);
//           else K_m = n_mid_blocks;
//           //////////////////////
//           // Start of Mid loop//
//           //////////////////////
//           for(mbi = 0; mbi < I_m; mbi++){
//             const int I_mid = I_outer + mbi*MID_BLOCK_SIZE; // Starting element index I for mid submatrix
//             for(mbj = 0; mbj < J_m; mbj++){
//               const int J_mid = J_outer + mbj*MID_BLOCK_SIZE; // Starting element index J for mid submatrix
//               submatrix_copy(M, MID_BLOCK_SIZE, I_mid, J_mid, C, C_mid); // Copy submatrix C_mid
//               for(mbk = 0; mbk < K_m; mbk++){
//                 const int K_mid = K_outer + mbk*MID_BLOCK_SIZE; // Starting element index K for mid submatrix
//                 submatrix_copy(M, MID_BLOCK_SIZE, I_mid, K_mid, A, A_mid); // Copy submatrix A_mid
//                 submatrix_copy(M, MID_BLOCK_SIZE, K_mid, J_mid, B, B_mid); // Copy submatrix B_mid
//                 /////////////////////////////////////////////////////////////
//                 // Now loop through the mid submatrix using kernel function//
//                 /////////////////////////////////////////////////////////////
//                 // for (sbi = 0; sbi < n_inner_blocks; sbi++) {
//                 //   const int I_inner = sbi*INNER_BLOCK_SIZE; // Starting element index I for inner submatrix (relative to mid submatrix)
//                 //   for (sbk = 0; sbk < n_inner_blocks; sbk++) {
//                 //     const int K_inner = sbk*INNER_BLOCK_SIZE; // Starting element index K for inner submatrix (relative to mid submatrix)
//                 //     submatrix_transpose(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, I_inner, K_inner, A_mid, A_inner);
//                 //     for (sbj = 0; sbj < n_inner_blocks; sbj++) {
//                 //       const int J_inner = sbj*INNER_BLOCK_SIZE; // Starting element index J for inner submatrix (relative to mid submatrix)
//                 //       submatrix_copy(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, I_inner, J_inner, C_mid, C_inner);
//                 //       submatrix_copy(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, K_inner, J_inner, B_mid, B_inner);
//                 //       mine_fma_dgemm(A_inner, B_inner, C_inner);
//                 //       submatrix_update(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, I_inner, J_inner, C_mid, C_inner);
//                 //     }
//                 //   }
//                 // }
//                 kernel_dgemm(MID_BLOCK_SIZE, INNER_BLOCK_SIZE, n_inner_blocks, A_mid, A_inner, B_mid, B_inner, C_mid, C_inner);
//                 ///////////////////////
//                 // End of Inner Loop //
//                 ///////////////////////
//               }
//               submatrix_update(M, MID_BLOCK_SIZE, I_mid, J_mid, C, C_mid);
//             }
//           }
//           //////////////////////
//           // End of Mid loop//
//           //////////////////////
//         }
//       }
//     }
//     // Free _mm_malloc memory
//     _mm_free(A_mid);
//     _mm_free(B_mid);
//     _mm_free(C_mid);
//
//     _mm_free(A_inner);
//     _mm_free(B_inner);
//     _mm_free(C_inner);


    // // Functional AVX2 script with three layer blocking
    // // Preallocate spaces for outer matrices A, B and C;
    // double* A_outer = (double*) _mm_malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(double),32);
    // double* B_outer = (double*) _mm_malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(double),32);
    // double* C_outer = (double*) _mm_malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(double),32);
    // // Preallocate spaces for middle submatrices A, B and C;
    // double* A_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
    // double* B_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
    // double* C_mid = (double*) _mm_malloc(MID_BLOCK_SIZE * MID_BLOCK_SIZE * sizeof(double),32);
    // // Preallocate spaces for inner matrices A, B and C;
    // double* A_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
    // double* B_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
    // double* C_inner = (double*) _mm_malloc(INNER_BLOCK_SIZE * INNER_BLOCK_SIZE * sizeof(double),32);
    // // // functional avx2 script with blocking
    // const int n_blocks = M / BLOCK_SIZE + (M%BLOCK_SIZE? 1 : 0); // # of blocks
    // const int n_mid_blocks = BLOCK_SIZE / MID_BLOCK_SIZE; // # of inner subblocks, use integer multiplier here when choosing blocksizes
    // const int n_inner_blocks = MID_BLOCK_SIZE / INNER_BLOCK_SIZE; // # of inner subblocks, use integer multiplier here when choosing blocksizes
    //
    // int bi, bj, bk;
    // int mbi, mbj, mbk;
    // int sbi, sbj, sbk;
    //
    // // It is probably better to use functions since each layer is the same
    // for (bi = 0; bi < n_blocks; bi++){
    //   for (bj = 0; bj < n_blocks; bj++){
    //     matrix_copy(M, BLOCK_SIZE, bi, bj, C, C_outer);
    //     for (bk = 0; bk < n_blocks; bk++){
    //       matrix_copy(M, BLOCK_SIZE, bi, bk, A, A_outer);
    //       matrix_copy(M, BLOCK_SIZE, bk, bj, B, B_outer);
    //       for (mbi = 0; mbi < n_mid_blocks; mbi++){
    //         for (mbj = 0; mbj < n_mid_blocks; mbj++){
    //           matrix_copy(BLOCK_SIZE, MID_BLOCK_SIZE, mbi, mbj, C_outer, C_mid);
    //           for (mbk = 0; mbk < n_mid_blocks; mbk++){
    //             matrix_transpose_copy(BLOCK_SIZE, MID_BLOCK_SIZE, mbi, mbk, A_outer, A_mid);
    //             matrix_copy(BLOCK_SIZE, MID_BLOCK_SIZE, mbk, mbj, B_outer, B_mid);
    //             for (sbi = 0; sbi < n_inner_blocks; sbi++){
    //               for (sbj = 0; sbj < n_inner_blocks; sbj++){
    //                 matrix_copy (MID_BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbj, C_mid, C_inner);
    //                 for (sbk = 0; sbk < n_inner_blocks; sbk++){
    //                   // matrix_copy (BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbk, A_outer, A_inner);
    //                   matrix_copy (MID_BLOCK_SIZE, INNER_BLOCK_SIZE, sbk, sbi, A_mid, A_inner); // For transposed A
    //                   matrix_copy (MID_BLOCK_SIZE, INNER_BLOCK_SIZE, sbk, sbj, B_mid, B_inner);
    //                   mine_fma_dgemm(A_inner, B_inner, C_inner);
    //                 }
    //                 matrix_update (MID_BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbj, C_mid, C_inner);
    //               }
    //             }
    //           }
    //           matrix_update (BLOCK_SIZE, MID_BLOCK_SIZE, mbi, mbj, C_outer, C_mid);
    //         }
    //       }
    //     }
    //     matrix_update (M, BLOCK_SIZE, bi, bj, C, C_outer);
    //   }
    // }
    //
    // // Free memory for basic kernel and AVX kernel.
    // _mm_free(A_outer);
    // _mm_free(B_outer);
    // _mm_free(C_outer);
    //
    // _mm_free(A_mid);
    // _mm_free(B_mid);
    // _mm_free(C_mid);
    //
    // _mm_free(A_inner);
    // _mm_free(B_inner);
    // _mm_free(C_inner);

    /////////////////////////////////////////////////
    // functional avx2 script for reference
    // const int n_inner_blocks = M / INNER_BLOCK_SIZE + (M%INNER_BLOCK_SIZE? 1 : 0); // # of blocks
    // int sbi, sbj, sbk;
    // for (sbi = 0; sbi < n_inner_blocks; sbi++){
    //   for (sbj = 0; sbj < n_inner_blocks; sbj++){
    //     matrix_copy (M, INNER_BLOCK_SIZE, sbi, sbj, C, C_inner);
    //     for (sbk = 0; sbk < n_inner_blocks; sbk++){
    //       matrix_copy (M, INNER_BLOCK_SIZE, sbi, sbk, A, A_inner);
    //       matrix_copy (M, INNER_BLOCK_SIZE, sbk, sbj, B, B_inner);
    //       mine_fma_dgemm(A_inner, B_inner, C_inner);
    //     }
    //     matrix_update (M, INNER_BLOCK_SIZE, sbi, sbj, C, C_inner);
    //   }
    // }
    // // Assign blocks for kernels to perform fast computation.
    // const int n_blocks = M / BLOCK_SIZE + (M%BLOCK_SIZE? 1 : 0); // # of blocks
    // const int n_inner_blocks = BLOCK_SIZE / INNER_BLOCK_SIZE; // For convenience, choose block size to be multiple of inner block size.
    // int bi, bj, bk;
    // int sbi, sbj, sbk;
    // for (bi = 0; bi < n_blocks; bi++){
    //   for (bj = 0; bj < n_blocks; bj++){
    //     matrix_copy (M, BLOCK_SIZE, bi, bj, C, C_outer);
    //     for (bk = 0; bk < n_blocks; bk++){
    //       matrix_copy (M, BLOCK_SIZE, bi, bk, A, A_outer);
    //       matrix_copy (M, BLOCK_SIZE, bk, bj, B, B_outer);
    //       // Compute the block multiplication by passing submatrices to kernel function
    //       for (sbi = 0; sbi < n_inner_blocks; sbi++){
    //         for (sbj = 0; sbj < n_inner_blocks; sbj++){
    //           matrix_copy (BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbj, C_outer, C_inner);
    //           for (sbk = 0; sbk < n_inner_blocks; sbk++){
    //             matrix_copy (BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbk, A_outer, A_inner);
    //             matrix_copy (BLOCK_SIZE, INNER_BLOCK_SIZE, sbk, sbj, B_outer, B_inner);
    //             mine_fma_dgemm(A_inner, B_inner, C_inner);
    //           }
    //           matrix_update (BLOCK_SIZE, INNER_BLOCK_SIZE, sbi, sbj, C_outer, C_inner);
    //         }
    //       }
    //     }
    //     matrix_update (M, BLOCK_SIZE, bi, bj, C, C_outer);
    //   }
    // }
// }

// Functional Version for basic implementation
void basic_dgemm(const int lda, const int M, const int N, const int K,
                 const double* restrict A, const double* restrict B,
                 double* restrict C){
  /* For the new kernel function, A must be stored in row-major
    lda is the leading dimension of the matrix (the M of square_dgemm).
    A is M-by-K, B is K-by-N and C is M-by-N */
    int i, j, k;
    for(j = 0; j < N; ++j){
      for(i = 0; i < M; ++i){
        double cij = C[i + j*lda];
        for (k = 0; k < K; ++k){
          cij += A[i * BLOCK_SIZE + k] * B[k + j * lda];
        }
        C[i + j*lda] = cij;
      }
    }
}
void do_block(const int lda,
              const double* restrict A, const double* restrict B, double* restrict C,
              const int i, const int j, const int k){
    // Determine the size of each sub-block
    const int M = (i+BLOCK_SIZE > lda? lda-i : BLOCK_SIZE);
    const int N = (j+BLOCK_SIZE > lda? lda-j : BLOCK_SIZE);
    const int K = (k+BLOCK_SIZE > lda? lda-k : BLOCK_SIZE);

    basic_dgemm(lda, M, N, K, A, B + k + j*lda, C + i + j*lda);
    // mine_dgemm(A,B,C);
}
void square_dgemm(const int M, const double* restrict A, const double* restrict B, double* restrict C){
    double* A_transposed = (double*) _mm_malloc(BLOCK_SIZE * BLOCK_SIZE * sizeof(double),32);
    const int n_blocks = M / BLOCK_SIZE + (M%BLOCK_SIZE? 1 : 0); // # of blocks
    int bi, bj, bk;
    for (bi = 0; bi < n_blocks; ++bi){
      const int i = bi * BLOCK_SIZE;
      for (bk = 0; bk < n_blocks; ++bk){
        const int k = bk * BLOCK_SIZE;
        const int M_sub = (i+BLOCK_SIZE > M? M-i : BLOCK_SIZE);
        const int K = (k+BLOCK_SIZE > M? M-k : BLOCK_SIZE);
        int it, kt;
        for (it = 0; it < M_sub; ++it){
          for (kt = 0; kt < K; ++kt)
            A_transposed[it*BLOCK_SIZE + kt] = A[i + k*M + it + kt*M];
        }
        for (bj = 0; bj < n_blocks; ++bj){
          const int j = bj * BLOCK_SIZE;
          do_block(M, A_transposed, B, C, i, j, k);
        }
      }
    }
    _mm_free(A_transposed);
}

// Functiosn that are for testing and stuffs
// void mine_dgemm( const double* restrict A, const double* restrict B,
//                  double* restrict C){
//     /*  My kernel function that uses AVX for optimal performance
//         It always assumes an input of A = 2 * P, B = P * 2
//         Template from https://bitbucket.org/dbindel/cs5220-s14/wiki/sse
//         B should be stored in its row form.
//     */
//
//     double C_swap = C[1];
//     C[1] = C[3];
//     C[3] = C_swap;
//
//     // This is really implicit in using the aligned ops...
//     __assume_aligned(A, 16);
//     __assume_aligned(B, 16);
//     __assume_aligned(C, 16);
//
//     // Load diagonal and off-diagonals
//     __m128d cd = _mm_load_pd(C+0);
//     __m128d co = _mm_load_pd(C+2);
//
//     /*
//     Assuming 2*2 case, and we do traditional, naive three loop multiplication.
//     Except in this case we don't need any loop because it's small.
//     */
//
//     __m128d a0 = _mm_load_pd(A);
//     __m128d b0 = _mm_load_pd(B);
//     __m128d td0 = _mm_mul_pd(a0, b0);
//     __m128d bs0 = swap_sse_doubles(b0);
//     __m128d to0 = _mm_mul_pd(a0, bs0);
//
//     __m128d a1 = _mm_load_pd(A+2);
//     __m128d b1 = _mm_load_pd(B+BLOCK_SIZE);
//     __m128d td1 = _mm_mul_pd(a1, b1);
//     __m128d bs1 = swap_sse_doubles(b1);
//     __m128d to1 = _mm_mul_pd(a1, bs1);
//
//     __m128d td_sum = _mm_add_pd(td0, td1);
//     __m128d to_sum = _mm_add_pd(to0, to1);
//
//     cd = _mm_add_pd(cd, td_sum);
//     co = _mm_add_pd(co, to_sum);
//
//     _mm_store_pd(C+0, cd);
//     _mm_store_pd(C+2, co);
//
//     C_swap = C[3];
//     C[3] = C[1];
//     C[1] = C_swap;
// }
