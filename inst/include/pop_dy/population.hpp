#ifndef POPULATION_HPP
#define POPULATION_HPP

#include "von_bertalanffy.hpp"
#include "../common/def.hpp"
#include "../common/model_object.hpp"

template <typename Type>
struct Population : public ModelObject<Type> {
  static uint32_t id_g; /**< The ID of the instance of the Population class */

  typename model_traits<Type>::data_vector length;
  typename model_traits<Type>::data_vector ages;

  std::shared_ptr< VonBertalanffy<Type> > vb;

  Population( ) {
    //in FIMS, this is done in information because could be multiples of any module
    // information loops over all modules and links correct child module with the correct parent modules
    //in this example, there is only one VB module, so the intiation can happen here
    this->vb = std::make_shared<VonBertalanffy<Type> >(); 
    this->id = Population::id_g++;
  }

  inline void CalculateLength(){
    this->length.resize(ages.size());
    for(int i =0; i < ages.size(); i++){
        this->length[i] = vb -> evaluate(ages[i]);
    }
  }

  void evaluate() {
    CalculateLength();
    for(int i=0; i<length.size(); i++){
      Rcout << "length, " << i << " is: " << length[i] << std::endl;
      Rcout << "vb output for age 3 is: " << vb->evaluate(ages[2]) << std::endl;
    }
    Rcout << "local vb k is: " << vb->k << std::endl;
  }

};

template <class Type>
uint32_t Population<Type>::id_g = 0;


#endif