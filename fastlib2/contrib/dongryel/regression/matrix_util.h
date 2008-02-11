#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include "fastlib/fastlib.h"

class MatrixUtil {

  public:

    static double L1Norm(const Matrix &m_mat) {

      double l1_norm = 0;
      for(index_t j = 0; j < m_mat.n_cols(); j++) {
	const double *m_mat_column = m_mat.GetColumnPtr(j);
	double tmp_l1_norm = 0;

	for(index_t i = 0; i < m_mat.n_rows(); i++) {
	  tmp_l1_norm += fabs(m_mat_column[i]);
	}
	l1_norm = std::max(l1_norm, tmp_l1_norm);
      }
      return l1_norm;
    }

    static double L1Norm(const Vector &v_vec) {
      
      double l1_norm = 0;
      for(index_t d = 0; d < v_vec.length(); d++) {
	l1_norm += fabs(v_vec[d]);
      }
      
      return l1_norm;
    }

    static double FrobeniusNorm(const Matrix &m_mat) {

      double l1_norm = 0;
      for(index_t j = 0; j < m_mat.n_cols(); j++) {
	const double *m_mat_column = m_mat.GetColumnPtr(j);

	for(index_t i = 0; i < m_mat.n_rows(); i++) {
	  l1_norm += m_mat_column[i] * m_mat_column[i];
	}
      }
      return l1_norm;
    }

    static double FrobeniusNorm(const Vector &v_vec) {
      
      double l1_norm = 0;
      for(index_t d = 0; d < v_vec.length(); d++) {
	l1_norm += v_vec[d] * v_vec[d];
      }
      
      return l1_norm;
    }

    static void ComponentwiseMin(int length, double *a_vec, double *b_vec, 
				 double *r_vec) {

      for(index_t j = 0; j < length; j++) {
	r_vec[j] = std::min(a_vec[j], b_vec[j]);
      }
    }

    static void ComponentwiseMax(int length, double *a_vec, double *b_vec,
				 double *r_vec) {

      for(index_t j = 0; j < length; j++) {
	r_vec[j] = std::max(a_vec[j], b_vec[j]);
      }
    }

    static void ComponentwiseMin(Matrix &a_mat, Matrix &b_mat, Matrix &r_mat) {

      for(index_t j = 0; j < a_mat.n_cols(); j++) {
	for(index_t i = 0; i < a_mat.n_rows(); i++) {
	  r_mat.set(i, j, std::min(a_mat.get(i, j), b_mat.get(i, j)));
	}
      }
    }

    static void ComponentwiseMax(Matrix &a_mat, Matrix &b_mat, Matrix &r_mat) {

      for(index_t j = 0; j < a_mat.n_cols(); j++) {
	for(index_t i = 0; i < a_mat.n_rows(); i++) {
	  r_mat.set(i, j, std::max(a_mat.get(i, j), b_mat.get(i, j)));
	}
      }
    }
};

#endif
