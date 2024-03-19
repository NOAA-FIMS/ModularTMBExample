#ifndef RCPP_DATA
#define RCPP_DATA


#include "rcpp_interface_base.hpp"
#include "../../../common/model.hpp"

/**
 * @brief Rcpp interface for Data as an S4 object. To instantiate
 * from R:
 * fleet <- new(fims$Data)
 *
 */
class DataInterfaceBase : public RcppInterfaceBase {
public:
static uint32_t id_g; /**< static id of the DataInterface object */
uint32_t id;          /**< local id of the DataInterface object */
static std::map<uint32_t, DataInterfaceBase*>
  data_objects; /**< map associating the ids of DataInterface to
 the objects */

/** @brief constructor
 */
DataInterfaceBase() {
  this->id = DataInterfaceBase::id_g++;
  DataInterfaceBase::data_objects[this->id] = this;
  RcppInterfaceBase::interface_objects.push_back(this);
}

/** @brief destructor
 */
virtual ~DataInterfaceBase() {}

};
uint32_t DataInterfaceBase::id_g = 1;
std::map<uint32_t, DataInterfaceBase*> DataInterfaceBase::data_objects;


class ObsDataInterface  : public DataInterfaceBase {
public:
    Rcpp::NumericVector data;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;
    
    ObsDataInterface() : DataInterfaceBase() {}
    
    virtual ~ObsDataInterface() {}
    
   
    
    template<typename Type>
    bool prepare_local() {
        Model<Type>* model = Model<Type>::getInstance();
        ObsData<Type>* obsdata = ObsData<Type>::obsdata_new();
        //Rcpp::XPtr< Model<Type> > model_ptr(model, true);
        //Rcpp::XPtr< ObsData<Type> > obsdata_ptr(obsdata, true);
        
        obsdata->ages.resize(this->ages.size());
        obsdata->data.resize(this->data.size());
        model->predicted.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            obsdata->ages[i] = this->ages[i];
            obsdata->data[i] = this->data[i];
        }

        model->obsdata = obsdata;
     
        return true;
    }

     /**
     * Prepares the model to work with TMB.
     */
    virtual bool prepare() {


        if (this->data.size() != this->ages.size()) {
            Rcpp::stop("ages vector length not equal to data vector length");
        }

        this->predicted = Rcpp::NumericVector(this->ages.size());

#ifdef TMB_MODEL

        this->prepare_local<TMB_FIMS_REAL_TYPE>();
        this->prepare_local<TMB_FIMS_FIRST_ORDER>();
        this->prepare_local<TMB_FIMS_SECOND_ORDER>();
        this->prepare_local<TMB_FIMS_THIRD_ORDER>();
     
#endif
   
        return true;
    }

     /**
     * Update the model parameter values and finalize. Sets the parameter values and evaluates the
     * portable model once and transfers values back to the Rcpp interface.
     */
    /*
    void finalize(Rcpp::NumericVector v) {
        Model<double>* model = Model<double>::getInstance();
        Rcpp::XPtr< Model<double> > model_ptr(model, true);

        if (this->data.size() != this->ages.size()) {
            Rcpp::stop("finalize: ages vector length not equal to data vector length");
        }

        
        for (int i = 0; i < this->predicted.size(); i++) {
            this->predicted[i] = model_ptr->predicted[i];
        }
    }
*/
     /**
     * Print model values.
     */
    void show_() {
        Rcpp::Rcout << std::setw(15) << "observed  " << std::setw(15) << "predicted\n";
        //Rcpp::Rcout << "Predicted size: " << this->predicted.size() << std::endl;
        for (int i = 0; i < this->predicted.size(); i++) {
            Rcpp::Rcout << std::left << std::setw(15) << this->data[i] << std::setw(15) << this->predicted[i] << "\n";
        }
    }
    
};
#endif