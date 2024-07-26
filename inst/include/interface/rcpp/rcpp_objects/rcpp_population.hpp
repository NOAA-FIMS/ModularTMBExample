#ifndef RCPP_POPULATION
#define RCPP_POPULATION


#include "../../../common/model.hpp"
#include "rcpp_interface_base.hpp"
#include "../../../pop_dy/population.hpp"

/**
 * @brief Rcpp interface for Data as an S4 object. To instantiate
 * from R:
 *
 */
class PopulationInterfaceBase : public RcppInterfaceBase {
public:
static uint32_t id_g; /**< static id of the DataInterface object */
uint32_t id;          /**< local id of the DataInterface object */
uint32_t module_id;

/** @brief constructor
 */
PopulationInterfaceBase() {
  this->id = PopulationInterfaceBase::id_g++;
  RcppInterfaceBase::interface_objects.push_back(this);
}

/** @brief destructor
 */
virtual ~PopulationInterfaceBase() {}


/** @brief get_id method for child classes to inherit */
virtual uint32_t get_id() = 0;


};
uint32_t PopulationInterfaceBase::id_g = 1;


class PopulationInterface  : public PopulationInterfaceBase {
public:
    Rcpp::NumericVector ages;
    VariableVector u;
    uint32_t growth_id;        /**< id of the growth function*/

    
    PopulationInterface() : PopulationInterfaceBase() {}
    
    virtual ~PopulationInterface() {}
    
    virtual uint32_t get_id() { return this->id; }

    void SetGrowth(uint32_t growth_id) { this->growth_id = growth_id; }


    virtual std::string get_module_name() {
        return "Population";
    }

    template<typename Type>
    bool prepare_local() {
        std::shared_ptr<Information<Type> > info =
            Information<Type>::getInstance();

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< Population<Type> > pop = 
            std::make_shared<Population<Type> >();
        
        pop->id = this->id;
        pop->growth_id = this->growth_id;
        
        pop->ages.resize(this->ages.size());
        
        for(int i =0; i < this->ages.size(); i++){
            pop->ages[i] = this->ages[i];
        }
        //Set Population length
        pop->length.resize(ages.size());
        std::stringstream ss;
        ss << this->get_module_name() << "_" << this->id;
        std::string key = ss.str();
        ss.str("");
        ss << key << "_length";
        info->variable_map[ss.str()] = &(pop)->length; 
        ss.str("");
        
        
        if(this->u.size() == 0){
          pop->u.resize(ages.size());
          for(int i=0; i<ages.size(); i++){
            pop->u[i] = 0.0;
          }
        } else {
          pop->u.resize(this->u.size());
          for(int i =0; i < this->u.size(); i++){
            pop->u[i] = this->u[i].value;
            if(this->u[i].is_random_effect){
              model->random_effects.push_back(&(pop)->u[i]);
            }
          }
        }
        
        ss << this->get_module_name() << "_" << this->id;
        ss.str("");
        ss << key << "_u";
        info->variable_map[ss.str()] = &(pop)->u; 
        ss.str("");
        

        info->pop_models[pop->id] = pop;
        model->pop_models[pop->id] = pop;
     
        return true;
    }

     /**
     * Prepares the model to work with TMB.
     */
    virtual bool prepare() {

#ifdef TMB_MODEL

        this->prepare_local<TMB_FIMS_REAL_TYPE>();
        this->prepare_local<TMB_FIMS_FIRST_ORDER>();
        this->prepare_local<TMB_FIMS_SECOND_ORDER>();
        this->prepare_local<TMB_FIMS_THIRD_ORDER>();
     
#endif
   
        return true;
    }
    
};
#endif