#ifndef RCPP_POPULATION
#define RCPP_POPULATION


#include "rcpp_interface_base.hpp"

/**
 * @brief Rcpp interface for Data as an S4 object. To instantiate
 * from R:
 *
 */
class PopulationInterfaceBase : public RcppInterfaceBase {
public:
static uint32_t id_g; /**< static id of the DataInterface object */
uint32_t id;          /**< local id of the DataInterface object */
uint32_t module_id;          /**< local id of the RcppInterfaceBase object */


/** @brief constructor
 */
PopulationInterfaceBase() {
  this->id = PopulationInterfaceBase::id_g++;
  RcppInterfaceBase::interface_objects.push_back(this);
  this->module_id = RcppInterfaceBase::module_id_g;
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
    
    PopulationInterface() : PopulationInterfaceBase() {}
    
    virtual ~PopulationInterface() {}
    
    virtual uint32_t get_id() { return this->id; }

    /** @brief returns the id for the logistic selectivity interface */
    virtual uint32_t get_module_id() { return this->module_id; }


    template<typename Type>
    bool prepare_local() {

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< Population<Type> > pop = 
            std::make_shared<Population<Type> >();
        
        pop->ages.resize(this->ages.size());
        
        for(int i =0; i < this->ages.size(); i++){
            pop->ages[i] = this->ages[i];
        }

        model->pop = pop;
     
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