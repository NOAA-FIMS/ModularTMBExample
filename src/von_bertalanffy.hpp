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
  typename model_traits<Type>::data_vector obs;
  std::vector<Type> predicted;
  std::vector<double> ages;
  std::vector<int> fish;
  std::vector<Type*> parameters;
  int nfish;
  Type log_k_mean;
  Type log_l_inf_mean;
  Type log_k_sigma;
  Type log_l_inf_sigma;
  Type a_min;
  vector<Type> log_k;
  vector<Type> log_l_inf;
  
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
    for(int i =0; i < obs.size(); i++){
      Type linf=exp(this->log_l_inf_mean + this->log_l_inf[this->fish[i]]);
      Type k=exp(this->log_k_mean + this->log_k[this->fish[i]]);
      Type temp = linf*(1.0-exp(-k*(ages[i]-this->a_min)));
      this->predicted[i] = temp;
      norm2+=(temp-obs[i])*(temp-obs[i])/(2*.1*.1);
    }
    // probability of the random effects
    for(int i=0;i<nfish;i++){
      norm2+= log_l_inf[i]/(2*pow(exp(log_l_inf_sigma),2));
      norm2+= log_k[i]/(2*pow(exp(log_k_sigma),2));
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
