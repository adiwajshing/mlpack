/**
 * @file methods/ann/convolution_rules/naive_convolution.hpp
 * @author Shangtong Zhang
 * @author Marcus Edel
 *
 * Implementation of the convolution.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_ANN_CONVOLUTION_RULES_NAIVE_CONVOLUTION_HPP
#define MLPACK_METHODS_ANN_CONVOLUTION_RULES_NAIVE_CONVOLUTION_HPP

#include <mlpack/prereqs.hpp>
#include "border_modes.hpp"

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

/**
 * Computes the two-dimensional convolution. This class allows specification of
 * the type of the border type. The convolution can be compute with the valid
 * border type of the full border type (default).
 *
 * FullConvolution: returns the full two-dimensional convolution.
 * ValidConvolution: returns only those parts of the convolution that are
 * computed without the zero-padded edges.
 *
 * @tparam BorderMode Type of the border mode (FullConvolution or
 * ValidConvolution).
 */
template<typename BorderMode = FullConvolution>
class NaiveConvolution
{
 public:
  /*
   * Perform a convolution (valid mode).
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the convolution.
   * @param output Output data that contains the results of the convolution.
   * @param dW Stride of filter application in the x direction.
   * @param dH Stride of filter application in the y direction.
   * @param dilationW The dilation factor in x direction.
   * @param dilationH The dilation factor in y direction.
   */
  template<typename eT, typename Border = BorderMode>
  static typename std::enable_if<
      std::is_same<Border, ValidConvolution>::value, void>::type
  Convolution(const arma::Mat<eT>& input,
              const arma::Mat<eT>& filter,
              arma::Mat<eT>& output,
              const size_t dW = 1,
              const size_t dH = 1,
              const size_t dilationW = 1,
              const size_t dilationH = 1, const size_t appending = false)
  {
    if (!appending) {
      output = arma::zeros<arma::Mat<eT> >(
        (input.n_rows - (filter.n_rows - 1) * dilationW - 1) / dW + 1,
        (input.n_cols - (filter.n_cols - 1) * dilationH -  1) / dH + 1);
    }
    eT* outputPtr = output.memptr();
    const size_t o_cols = output.n_cols, o_rows = output.n_rows;
    const size_t f_cols = filter.n_cols, f_rows = filter.n_rows;
    // parallelize computation if the output is large enough
    #pragma omp parallel for schedule(static) if (input.n_cols >= 64)
    for (omp_size_t j = 0; j < o_cols; ++j)
    {
      for (size_t i = 0; i < o_rows; ++i)
      {
        const eT* kernelPtr = filter.memptr();
        for (size_t kj = 0; kj < f_cols; ++kj)
        {
         const eT* inputPtr = input.colptr(kj*dilationW + j*dW) + i*dH;
         for (size_t ki = 0; ki < f_rows; ++ki, ++kernelPtr,
              inputPtr += dilationH)
           outputPtr[j*o_rows + i] += *kernelPtr * (*inputPtr);
        }
      }
    }
  }

  /*
   * Perform a convolution (full mode).
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the convolution.
   * @param output Output data that contains the results of the convolution.
   * @param dW Stride of filter application in the x direction.
   * @param dH Stride of filter application in the y direction.
   * @param dilationW The dilation factor in x direction.
   * @param dilationH The dilation factor in y direction.
   */
  template<typename eT, typename Border = BorderMode>
  static typename std::enable_if<
      std::is_same<Border, FullConvolution>::value, void>::type
  Convolution(const arma::Mat<eT>& input,
              const arma::Mat<eT>& filter,
              arma::Mat<eT>& output,
              const size_t dW = 1,
              const size_t dH = 1,
              const size_t dilationW = 1,
              const size_t dilationH = 1, const bool appending = false)
  {
    size_t outputRows = (input.n_rows - 1) * dW + 2 * (filter.n_rows - 1)
        * dilationW + 1;
    size_t outputCols = (input.n_cols - 1) * dH + 2 * (filter.n_cols - 1)
        * dilationH + 1;

    for (size_t i = 0; i < dW; ++i)
    {
      if (((((i + outputRows - 2 * (filter.n_rows - 1) * dilationW - 1) % dW)
          + dW) % dW) == i){
        outputRows += i;
        break;
      }
    }
    for (size_t i = 0; i < dH; ++i)
    {
      if (((((i + outputCols - 2 * (filter.n_cols - 1) * dilationH - 1) % dH)
          + dH) % dH) == i){
        outputCols += i;
        break;
      }
    }

    // Pad filter and input to the working output shape.
    arma::Mat<eT> inputPadded = arma::zeros<arma::Mat<eT> >(outputRows,
        outputCols);
    inputPadded.submat((filter.n_rows - 1) * dilationW, (filter.n_cols - 1)
        * dilationH, (filter.n_rows - 1) * dilationW + input.n_rows - 1,
        (filter.n_cols - 1) * dilationH + input.n_cols - 1) = input;

    NaiveConvolution<ValidConvolution>::Convolution(inputPadded, filter,
        output, 1, 1, dilationW, dilationH, appending);
  }

  /*
   * Perform a convolution using 3rd order tensors.
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the convolution.
   * @param output Output data that contains the results of the convolution.
   * @param dW Stride of filter application in the x direction.
   * @param dH Stride of filter application in the y direction.
   * @param dilationW The dilation factor in x direction.
   * @param dilationH The dilation factor in y direction.
   */
  template<typename eT>
  static void Convolution(const arma::Cube<eT>& input,
                          const arma::Cube<eT>& filter,
                          arma::Cube<eT>& output,
                          const size_t dW = 1,
                          const size_t dH = 1,
                          const size_t dilationW = 1,
                          const size_t dilationH = 1)
  {
    arma::Mat<eT> convOutput;
    NaiveConvolution<BorderMode>::Convolution(input.slice(0), filter.slice(0),
        convOutput, dW, dH, dilationW, dilationH);

    output = arma::Cube<eT>(convOutput.n_rows, convOutput.n_cols,
        input.n_slices);
    output.slice(0) = convOutput;

    for (size_t i = 1; i < input.n_slices; ++i)
    {
      NaiveConvolution<BorderMode>::Convolution(input.slice(i), filter.slice(i),
          output.slice(i), dW, dH, dilationW, dilationH);
    }
  }

  /*
   * Perform a convolution using dense matrix as input and a 3rd order tensors
   * as filter and output.
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the convolution.
   * @param output Output data that contains the results of the convolution.
   * @param dW Stride of filter application in the x direction.
   * @param dH Stride of filter application in the y direction.
   * @param dilationW The dilation factor in x direction.
   * @param dilationH The dilation factor in y direction.
   */
  template<typename eT>
  static void Convolution(const arma::Mat<eT>& input,
                          const arma::Cube<eT>& filter,
                          arma::Cube<eT>& output,
                          const size_t dW = 1,
                          const size_t dH = 1,
                          const size_t dilationW = 1,
                          const size_t dilationH = 1)
  {
    arma::Mat<eT> convOutput;
    NaiveConvolution<BorderMode>::Convolution(input, filter.slice(0),
        convOutput, dW, dH, dilationW, dilationH);

    output = arma::Cube<eT>(convOutput.n_rows, convOutput.n_cols,
        filter.n_slices);
    output.slice(0) = convOutput;

    for (size_t i = 1; i < filter.n_slices; ++i)
    {
      NaiveConvolution<BorderMode>::Convolution(input, filter.slice(i),
          output.slice(i), dW, dH, dilationW, dilationH);
    }
  }

  /*
   * Perform a convolution using a 3rd order tensors as input and output and a
   * dense matrix as filter.
   *
   * @param input Input used to perform the convolution.
   * @param filter Filter used to perform the convolution.
   * @param output Output data that contains the results of the convolution.
   * @param dW Stride of filter application in the x direction.
   * @param dH Stride of filter application in the y direction.
   * @param dilationW The dilation factor in x direction.
   * @param dilationH The dilation factor in y direction.
   */
  template<typename eT>
  static void Convolution(const arma::Cube<eT>& input,
                          const arma::Mat<eT>& filter,
                          arma::Cube<eT>& output,
                          const size_t dW = 1,
                          const size_t dH = 1,
                          const size_t dilationW = 1,
                          const size_t dilationH = 1)
  {
    arma::Mat<eT> convOutput;
    NaiveConvolution<BorderMode>::Convolution(input.slice(0), filter,
        convOutput, dW, dH, dilationW, dilationH);

    output = arma::Cube<eT>(convOutput.n_rows, convOutput.n_cols,
        input.n_slices);
    output.slice(0) = convOutput;

    for (size_t i = 1; i < input.n_slices; ++i)
    {
      NaiveConvolution<BorderMode>::Convolution(input.slice(i), filter,
          output.slice(i), dW, dH, dilationW, dilationH);
    }
  }
};  // class NaiveConvolution

} // namespace ann
} // namespace mlpack

#endif
