
#ifndef MODEL_HPP
#define MODEL_HPP

#include "def.hpp"
#include "information.hpp"
#include "../pop_dy/population.hpp"
#include "../pop_dy/von_bertalanffy.hpp"
#include "../distributions/normal_lpdf.hpp"

template<typename Type>
class Model{
    public:

    fims::Vector<Type> predicted;

    std::map<uint32_t, std::shared_ptr<DensityComponentBase<Type> > >
      density_components;
    typedef typename std::map<
      uint32_t, std::shared_ptr<DensityComponentBase<Type> > >::iterator
      density_components_iterator;

    std::map<uint32_t, std::shared_ptr<Population<Type> > >
      pop_models;
    typedef typename std::map<
      uint32_t, std::shared_ptr<Population<Type> > >::iterator
      pop_iterator;
      

    std::vector<Type*> parameters;
    std::vector<Type*> random_effects; 
    std::vector<Type*> data;
    std::vector<std::string> pnames;
    static std::shared_ptr<Model<Type> > model;
    std::shared_ptr<Information<Type> > info;
    #ifdef TMB_MODEL
      ::objective_function<Type> *of;
      using DataIndicator = typename model_traits<Type>::data_indicator_;
      DataIndicator keep;
    #endif
    

    Model(){ }
  
  /**
   * Returns the sigleton instance of VonBertalanffyModel
   * of type Type.
   */
  static std::shared_ptr<Model<Type> > getInstance() {
    if (Model<Type>::model == nullptr) {
      Model<Type>::model = std::make_shared<Model<Type> >();
      Model<Type>::model->info = Information<Type>::getInstance();
    }
    return Model<Type>::model;
  }

  

     /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(){
    Type jnll = 0.0;

    this->info->setup_population();
    
    //maybe here, setup functions can take a simulate flag and simulation can be controlled from model
    //setup pointers for priors
    this->info->setup_priors();
     for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
      #ifdef TMB_MODEL
        n->of = this->of;
      #endif
      if(n->input_type == "prior"){
        jnll -= n->evaluate();
      }
    }

    
    //setup pointers for random effects
    info->setup_random_effects();
    //evaluate nlls for priors and random effects
     for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
      #ifdef TMB_MODEL
        n->of = this->of;
      #endif
      if(n->input_type == "random_effects"){
        jnll -= n->evaluate();
      }
    }
    
    for (pop_iterator it = this->pop_models.begin();
         it != this->pop_models.end(); ++it) {
      std::shared_ptr<Population<Type> > pop = (*it).second;
      pop->evaluate(); 
    }
    this->info->setup_data();
    for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
      #ifdef TMB_MODEL
        n->of = this->of;
        n->keep = this->keep;
      #endif
      if(n->input_type == "data"){
        jnll -= n->evaluate();
      }
    }
    return jnll;

  }
  
  /**
   * clears the estimated parameter list
   */
  void clear(){
    this->parameters.clear();
  }
  
  };

template <typename Type>
std::shared_ptr<Model<Type> > Model<Type>::model = nullptr;  // singleton instance

#endif