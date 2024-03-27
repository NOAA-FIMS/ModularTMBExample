#ifndef VONBERTALANFFY_HPP
#define  VONBERTALANFFY_HPP

#include <vector>
#include "../common/def.hpp"
#include "../common/model_object.hpp"

//#include "../interface/interface.hpp"

/**
 * Portable von Bertalanffy model. Can be used in multiple
 * modeling platforms.
 */
template<typename Type>
class VonBertalanffy : public ModelObject<Type> {
  static uint32_t id_g; /**< The ID of the instance of the Population class */
  public:
  Type k;
  Type l_inf;
  Type a_min;
  Type alpha;
  Type beta;
  
  VonBertalanffy(){
    this->id = VonBertalanffy::id_g++;
  }
  
  /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(const Type& age){
    Type ret = this->l_inf * (1.0 - exp(-this->k * (age - this->a_min)));
    return ret;
  }
  
  
};

template <class Type>
uint32_t VonBertalanffy<Type>::id_g = 0;




#endif
