#include "openblas_residue.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "cblas.h"
#include "gmp.h"

#include "arith_utils.h"
#include "mods.c"


openblasResidue_t * openblas_init(long nrows, long ncols, long p)
{
  openblasResidue_t * A = malloc(sizeof(openblasResidue_t));
  A->_data = malloc(nrows*ncols*sizeof(double));
  A->nrows = nrows;
  A->ncols = ncols;
  modulus_init(&A->mod, p);

  return A;
}

void openblas_fini(openblasResidue_t * A)
{
  free(A->_data);
  free(A);
}

long openblas_getEntry(openblasResidue_t const * A, long idx)
{
  return A->_data[idx];
}

void openblas_setEntry(openblasResidue_t * A, long idx, long val)
{
  A->_data[idx] = val;
}

void openblas_copy(openblasResidue_t * dst, openblasResidue_t const * src)
{
  long nrows = dst->nrows;
  long ncols = dst->ncols;
  assert(dst->nrows == src->nrows && dst->ncols == src->ncols);
  cblas_dcopy(nrows*ncols, src->_data, 1, dst->_data, 1);
}

void openblas_fromMpzMatrix(openblasResidue_t * dst, mpzMatrix_t const * src)
{
  long i;
  mpz_t const * data = mpzMatrix_constData(src);
  assert(dst->nrows == src->nrows && dst->ncols == src->ncols);
  for (i = 0; i < mpzMatrix_numElems(src); ++i) {
    dst->_data[i] = modsMpzL(data[i], dst->mod.p);
  }
}

void openblas_identity(openblasResidue_t * A)
{
  long i;
  long n = A->ncols;
  assert(A->ncols == A->nrows);
  openblas_zero(A);
  for (i = 0; i < n; ++i) {
    A->_data[i + n*i] = 1.0;
  }
}

void openblas_zero(openblasResidue_t * A)
{
  memset(A->_data, 0, sizeof(double)*A->nrows*A->ncols);
}

int openblas_isZero(openblasResidue_t const * A)
{
  long i;
  for (i = 0; i < A->nrows*A->ncols; ++i) {
    if (A->_data[i] != 0) { return 0; }
  }
  return 1;
}

void openblas_print(FILE * stream, openblasResidue_t const * A)
{
  long i, j;
  for (i = 0; i < A->nrows; ++i) {
    for (j = 0; j < A->ncols; ++j) {
      fprintf(stream, "%4.0f ", A->_data[i*A->ncols+j]);
    }
    fprintf(stream, "\n");
  }
}

void openblas_add(openblasResidue_t * dst, openblasResidue_t const * src, long k)
{
  long nrows = dst->nrows;
  long ncols = dst->ncols;
  cblas_daxpy(nrows*ncols, k, src->_data, 1, dst->_data, 1);
  openblas_mods(dst);
}

long openblas_determinant(openblasResidue_t * A)
{
  assert(A->ncols == A->nrows);
  return mDeterminant(A->mod.p, A->_data, A->ncols); /* A is destroyed in place. */
}

static void gemm(double const * A, double const * B, double * C, long alpha, long beta, long m, long n, long k)
{
  /* C := alpha*A*B + beta*C */
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k,
             (double)alpha, A, k,
             B, n,
             (double)beta, C, n);
}

void openblas_gemm(openblasResidue_t * dst, openblasResidue_t const * A, openblasResidue_t const * B)
{
  openblas_gemm_helper(dst, A, B, 1, 0);
  openblas_mods(dst);
}

void openblas_gemm_helper(openblasResidue_t * dst, openblasResidue_t const * A, openblasResidue_t const * B, long alpha, long beta)
{
  assert(dst->nrows == A->nrows);
  assert(dst->ncols == B->ncols);
  assert(A->ncols == B->nrows);
  gemm(A->_data, B->_data, dst->_data, alpha, beta, dst->nrows, dst->ncols, A->ncols);
}

int openblas_inverse(openblasResidue_t * A)
{
  int rc;
  assert(A->ncols == A->nrows);
  rc = mInverse(A->mod.p, A->_data, A->ncols); /* mInverse works in place */
  if(rc) {
    openblas_mods(A);
  }
  return rc;
}

void openblas_mods(openblasResidue_t * dst)
{
  long i;
  long nrows = dst->nrows;
  long ncols = dst->ncols;
  double * data = dst->_data;
  modulus_t A = dst->mod;
  for (i = 0; i < nrows*ncols; ++i) {
    data[i] = double_mods(data[i], A);
  }
}

void openblas_quadLift(openblasResidue_t * dst, openblasResidue_t const * T, openblasResidue_t const * A, openblasResidue_t const * M, long xinv)
{
  long n = dst->ncols;
  assert(dst->ncols == dst->nrows);
  cblas_dcopy(n*n, T->_data, 1, dst->_data, 1);
  gemm(A->_data, M->_data, dst->_data, -1, 1, n, n, n);
  openblas_mods(dst);
  cblas_dscal(n*n, (double)xinv, dst->_data, 1);
  openblas_mods(dst);
}

void openblas_scale(openblasResidue_t * dst, long k)
{
  long nrows = dst->nrows;
  long ncols = dst->ncols;
  cblas_dscal(nrows*ncols, k, dst->_data, 1);
  openblas_mods(dst);
}

void openblas_set(openblasResidue_t * dst, long r, long c, double src)
{
  dst->_data[r * dst->ncols + c] = src;
  //dst->_data[c * dst->nrows + r] = src;
}

double openblas_get(openblasResidue_t * dst, long r, long c)
{
  return dst->_data[r * dst->ncols + c];
  //return dst->_data[c * dst->nrows + r];
}
