/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   def.hpp
 * Author: mattadmin
 *
 * Created on September 6, 2023, 8:47 AM
 */

#ifndef DEF_HPP
#define DEF_HPP

#define TMB_MODEL


#ifdef TMB_MODEL

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

#define TMB_FIMS_REAL_TYPE double 
#define TMB_FIMS_FIRST_ORDER AD<TMB_FIMS_REAL_TYPE> 
#define TMB_FIMS_SECOND_ORDER AD<TMB_FIMS_FIRST_ORDER >
#define TMB_FIMS_THIRD_ORDER AD<TMB_FIMS_SECOND_ORDER> 

#endif



#endif /* DEF_HPP */

