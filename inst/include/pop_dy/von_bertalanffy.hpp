#ifndef VONBERTALANFFY_HPP
#define  VONBERTALANFFY_HPP

/**
 * Portable von Bertalanffy model. Can be used in multiple
 * modeling platforms.
 */
template<typename Type>
class VonBertalanffy{
public:
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
    Type ret = this->l_inf * (1.0 - exp(-k * (age - this->a_min)));
    return ret;
  }
  
  
};



#endif
