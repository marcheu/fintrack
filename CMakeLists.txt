cmake_minimum_required(VERSION 3.18)
project( fintrack LANGUAGES CUDA CXX C)

set (CMAKE_CUDA_ARCHITECTURES 50 60 70)

add_executable( fintrack
  backtest.h
  backtest.cc
  constants.h
  data_series.h
  date.h
  evaluation.cc
  evaluation.h
  includes.h
  loader.cc
  loader.h
  main.cc
  monte_carlo.cu
  monte_carlo.h
  neural_net.cc
  neural_net.h
  portfolio.cc
  portfolio.h
  stochastic_optimization.cc
  stochastic_optimization.h
  tests.cc
  tests.h
  util.cc
  util.h
)

