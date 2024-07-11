#ifndef POPULATION_HPP
#define POPULATION_HPP

#include "von_bertalanffy.hpp"
#include "../common/fims_vector.hpp"
#include "../common/model_object.hpp"

template <typename Type>
struct Population : public ModelObject<Type> {
  static uint32_t id_g; /**< The ID of the instance of the Population class */
  // growth
  int growth_id = -999; /*!< id of growth model object*/

  fims::Vector<Type> length;
  fims::Vector<Type> ages;
  fims::Vector<Type> u;

  std::shared_ptr< VonBertalanffy<Type> > vb;

  Population() { 
    this->id = Population::id_g++;
  }
  

  inline void CalculateLength(){ 
    
    for(int i =0; i < ages.size(); i++){
      Rcout << "u, " << i << " is: " << u[i] << std::endl;
        length[i] = vb -> evaluate(ages[i]) + u[i];
    }
  }

  void evaluate() {
    CalculateLength();
  }

};


template <class Type>
uint32_t Population<Type>::id_g = 0;

#endif