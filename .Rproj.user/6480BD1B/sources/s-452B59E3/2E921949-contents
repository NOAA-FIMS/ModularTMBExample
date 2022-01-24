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
  //singleton instance based on Type
  static VonBertalanffyModel<Type>* instance;
  
  /**
   * Returns the sigleton instance of VonBertalanffyModel
   * of type Type.
   */
  static VonBertalanffyModel<Type>* getInstance(){
    return VonBertalanffyModel<Type>::instance;
  }
  
  /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(){
    if(this->predicted.size()==0){
      this->predicted.resize(ages.size());
    }
    Type norm2 = 0.0;
    for(int i =0; i < ages.size(); i++){
        Type temp = this->l_inf * (1.0 - exp(-k * (ages[i] - this->a_min)));
        this->predicted[i] = temp;
        norm2+=(temp-data[i])*(temp-data[i]);
    }
    return norm2;
  }
  
  /**
   * clears the estimated parameter list
   */
  void clear(){
    this->parameters.clear();
  }
};

template<class Type>
VonBertalanffyModel<Type>* VonBertalanffyModel<Type>::instance = new VonBertalanffyModel<Type>();


#endif
