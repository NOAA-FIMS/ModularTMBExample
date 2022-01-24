#ifndef INTERFACE_HPP
#define INTERFACE_HPP
/**
 * Interface file. Uses preprocessing macros 
 * to interface with multiple modeling platforms.
 */


#define TMB_MODEL


//traits for interfacing with TMB
#ifdef TMB_MODEL
#include <TMB.hpp>

template<typename Type>
struct model_traits{
  typedef typename CppAD::vector<Type> data_vector;
  typedef typename CppAD::vector<Type> parameter_vector;
};

template<typename T>
T exp(const T& x){
  return exp(x);
}

template <class T>
const T log(const T& x){return log(x);}

#endif


#endif