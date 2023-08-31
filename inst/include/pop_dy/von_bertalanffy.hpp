#ifndef VONBERTALANFFY_HPP
#define  VONBERTALANFFY_HPP

#include <vector>
#include "interface.hpp"

/**
 * Portable von Bertalanffy model. Can be used in multiple
 * modeling platforms.
 */
template<typename Type>
class VonBertalanffyModel{
public:
  //using traits for modeling platform specific structures
  typename model_traits<Type>::data_vector data;
  std::vector<Type> predicted;
  std::vector<double> ages;
  std::vector<Type*> parameters;
  Type k;
  Type l_inf;
  Type a_min;
  Type alpha;
  Type beta;
  
  
  /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(const Type& age){
    Type ret = this->l_inf * (1.0 - exp(-k * (age- this->a_min)));
    return ret;
  }
  
  /**
   * clears the estimated parameter list
   */
  void clear(){
    this->parameters.clear();
  }
};



#endif
