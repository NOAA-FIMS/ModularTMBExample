#ifndef FIMS_COMMON_INFORMATION_HPP
#define FIMS_COMMON_INFORMATION_HPP

#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../distributions/density_components_base.hpp"
#include "../distributions/normal_lpdf.hpp"
#include "../distributions/ar1_lpdf.hpp"
#include "../pop_dy/population.hpp"
#include "../pop_dy/von_bertalanffy.hpp"

template <typename Type>
class Information {
  public:
    static std::shared_ptr<Information<Type> >
        information;          /**< singleton instance >*/
    std::vector<Type*> parameters;
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

    std::map<uint32_t, std::shared_ptr<VonBertalanffy<Type> > >
        vb_models;
    typedef typename std::map<
        uint32_t, std::shared_ptr<VonBertalanffy<Type> > >::iterator
        vb_iterator;

    std::unordered_map<uint32_t, fims::Vector<Type>* > variable_map;
    typedef typename std::unordered_map<uint32_t, fims::Vector<Type>* >::iterator 
      variable_map_iterator;

    
    Information() {}

    virtual ~Information() {}



    //singleton instance based on Type
    static std::shared_ptr<Information<Type> > instance;
  
    /**
     * Returns a single Information object for type T.
     *
     * @return singleton for type T
     */
    static std::shared_ptr<Information<Type> > getInstance() {
        return Information<Type>::instance; //TODO: make sure not null and create if null
    }


    void setup_population(){
      for (pop_iterator it = this->pop_models.begin();
         it != this->pop_models.end(); ++it) {
        std::shared_ptr<Population<Type> > pop = (*it).second;
        uint32_t growth_uint = static_cast<uint32_t>(pop->growth_id);
        vb_iterator vbit = this->vb_models.find(growth_uint);  
        pop->vb = (*vbit).second; 
      }
    }
    
    void setup_priors(){
      for(density_components_iterator it = density_components.begin(); it!= density_components.end(); ++it){
      std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
      if(n->input_type == "prior"){
        variable_map_iterator vmit;
        vmit = this->variable_map.find(n->key[0]); 
        n->observed_value = *(*vmit).second;
        for(size_t i=1; i<n->key.size(); i++){
          vmit = this->variable_map.find(n->key[i]); 
          n->observed_value.insert(std::end(n->observed_value), 
            std::begin(*(*vmit).second), std::end(*(*vmit).second));
        } 
      }
    }
    }
    void setup_random_effects(){
      for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
        std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
        if(n->input_type == "re"){
          variable_map_iterator vmit;
          vmit = this->variable_map.find(n->key[0]); 
          n->observed_value = *(*vmit).second;
          for(size_t i=1; i<n->key.size(); i++){
            vmit = this->variable_map.find(n->key[i]); 
            n->observed_value.insert(std::end(n->observed_value), 
                                     std::begin(*(*vmit).second), std::end(*(*vmit).second));
          } 
          std::shared_ptr<DensityComponentBase<Type> > density_components_base = density_components[n->id];
          AR1LPDF<Type>* ar1 = (AR1LPDF<Type>*) density_components_base.get();
          Rcout << "inside setup_re, obs.val size is: " << n->observed_value.size() << std::endl;
          Rcout << "inside setup_re, rho is: " << ar1->rho[0] << std::endl;
          for (pop_iterator it = this->pop_models.begin();
               it != this->pop_models.end(); ++it) {
               std::shared_ptr<Population<Type> > pop = (*it).second;
            Rcout << "pop u size is: " << pop->u.size() << std::endl;
            Rcout << "first pop u is: " << pop->u[0] << std::endl;
          }
        }
      }
    }
    void setup_data(){
      for(density_components_iterator it = this->density_components.begin(); it!= this->density_components.end(); ++it){
      std::shared_ptr<DensityComponentBase<Type> > n = (*it).second;
      if(n->input_type == "data"){
        variable_map_iterator vmit;
        vmit = this->variable_map.find(n->key[0]); 
        n->expected_value = *(*vmit).second;
        
        for(size_t i=1; i<n->key.size(); i++){
          vmit = this->variable_map.find(n->key[i]); 
          n->expected_value.insert(std::end(n->expected_value), 
            std::begin(*(*vmit).second), std::end(*(*vmit).second));
        } 
      }
    }
  }
    
    
    
};

template<typename Type>
std::shared_ptr<Information<Type> > Information<Type>::instance = std::make_shared<Information<Type> >();


#endif