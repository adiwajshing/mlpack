/**
 * @file methods/ann/layer/max_pooling_impl.hpp
 * @author Marcus Edel
 * @author Nilay Jain
 *
 * Implementation of the MaxPooling class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_ANN_LAYER_MAX_POOLING_IMPL_HPP
#define MLPACK_METHODS_ANN_LAYER_MAX_POOLING_IMPL_HPP

// In case it hasn't yet been included.
#include "max_pooling.hpp"

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

template<typename InputDataType, typename OutputDataType>
MaxPooling<InputDataType, OutputDataType>::MaxPooling()
{
  // Nothing to do here.
}

template<typename InputDataType, typename OutputDataType>
MaxPooling<InputDataType, OutputDataType>::MaxPooling(
    const size_t kernelWidth,
    const size_t kernelHeight,
    const size_t strideWidth,
    const size_t strideHeight,
    const bool floor) :
    kernelWidth(kernelWidth),
    kernelHeight(kernelHeight),
    strideWidth(strideWidth),
    strideHeight(strideHeight),
    floor(floor),
    inSize(0),
    outSize(0),
    reset(false),
    inputWidth(0),
    inputHeight(0),
    outputWidth(0),
    outputHeight(0),
    deterministic(false),
    offset(0),
    batchSize(0)
{
  // Nothing to do here.
}

template<typename InputDataType, typename OutputDataType>
template<typename eT>
void MaxPooling<InputDataType, OutputDataType>::Forward(
  const arma::Mat<eT>& input, arma::Mat<eT>& output)
{
  batchSize = input.n_cols;
  inSize = input.n_elem / (inputWidth * inputHeight * batchSize);
  inputTemp = arma::cube(const_cast<arma::Mat<eT>&>(input).memptr(),
      inputWidth, inputHeight, batchSize * inSize, false, false);

  if (floor)
  {
    outputWidth = std::floor((inputWidth -
        (double) kernelWidth) / (double) strideWidth + 1);
    outputHeight = std::floor((inputHeight -
        (double) kernelHeight) / (double) strideHeight + 1);
    offset = 0;
  }
  else
  {
    outputWidth = std::ceil((inputWidth -
        (double) kernelWidth) / (double) strideWidth + 1);
    outputHeight = std::ceil((inputHeight -
        (double) kernelHeight) / (double) strideHeight + 1);
    offset = 1;
  }

  outputTemp = arma::zeros<arma::Cube<eT> >(outputWidth, outputHeight,
      batchSize * inSize);

  if (!deterministic)
  {
    poolingIndices.push_back(outputTemp);
  }

  if (!reset)
  {
    size_t elements = inputWidth * inputHeight;
    indicesCol = arma::linspace<arma::Col<size_t> >(0, (elements - 1),
        elements);

    indices = arma::Mat<size_t>(indicesCol.memptr(), inputWidth, inputHeight);

    reset = true;
  }
  #pragma omp parallel for
  for (omp_size_t s = 0; s < inputTemp.n_slices; s++)
  {
    if (!deterministic)
    {
      PoolingOperation(inputTemp.slice(s), outputTemp.slice(s),
        poolingIndices.back().slice(s));
    }
    else
    {
      PoolingOperation(inputTemp.slice(s), outputTemp.slice(s),
          inputTemp.slice(s));
    }
  }

  output = arma::Mat<eT>(outputTemp.memptr(), outputTemp.n_elem / batchSize,
      batchSize);

  outputWidth = outputTemp.n_rows;
  outputHeight = outputTemp.n_cols;
  outSize = batchSize * inSize;
}

template<typename InputDataType, typename OutputDataType>
template<typename eT>
void MaxPooling<InputDataType, OutputDataType>::Backward(
    const arma::Mat<eT>& /* input */, const arma::Mat<eT>& gy, arma::Mat<eT>& g)
{
  arma::cube mappedError = arma::cube(((arma::Mat<eT>&) gy).memptr(),
      outputWidth, outputHeight, outSize, false, false);

  gTemp = arma::zeros<arma::cube>(inputTemp.n_rows,
      inputTemp.n_cols, inputTemp.n_slices);
  #pragma omp parallel for
  for (omp_size_t s = 0; s < mappedError.n_slices; s++)
  {
    Unpooling(mappedError.slice(s), gTemp.slice(s),
        poolingIndices.back().slice(s));
  }

  poolingIndices.pop_back();

  g = arma::mat(gTemp.memptr(), gTemp.n_elem / batchSize, batchSize);
}

template<typename InputDataType, typename OutputDataType>
template<typename Archive>
void MaxPooling<InputDataType, OutputDataType>::serialize(
    Archive& ar,
    const uint32_t /* version */)
{
  ar(CEREAL_NVP(kernelWidth));
  ar(CEREAL_NVP(kernelHeight));
  ar(CEREAL_NVP(strideWidth));
  ar(CEREAL_NVP(strideHeight));
  ar(CEREAL_NVP(batchSize));
  ar(CEREAL_NVP(floor));
  ar(CEREAL_NVP(inputWidth));
  ar(CEREAL_NVP(inputHeight));
  ar(CEREAL_NVP(outputWidth));
  ar(CEREAL_NVP(outputHeight));
}

} // namespace ann
} // namespace mlpack

#endif
