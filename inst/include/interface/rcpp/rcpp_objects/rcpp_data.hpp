#ifndef RCPP_DATA
#define RCPP_DATA


#include "rcpp_interface_base.hpp"

class ObsDataInterface  : public RcppInterfaceBase {
public:
    Rcpp::NumericVector data;
    Rcpp::NumericVector ages;
    Rcpp::NumericVector predicted;

    template<typename Type>
    bool prepare_local() {

        std::shared_ptr<Model<Type> > model = Model<Type>::getInstance();
        std::shared_ptr< ObsData<Type> > obsdata;
        obsdata = std::make_shared<ObsData<Type> >();
        
        obsdata->ages.resize(this->ages.size());
        obsdata->data.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            obsdata->ages[i] = this->ages[i];
            obsdata->data[i] = this->data[i];
        }
        model->predicted.resize(this->data.size());
        
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
    void finalize(Rcpp::NumericVector v) {
        std::shared_ptr< Model<double> > model = Model<double>::getInstance();

        if (this->data.size() != this->ages.size()) {
            Rcpp::stop("finalize: ages vector length not equal to data vector length");
        }

        
        for (int i = 0; i < this->predicted.size(); i++) {
            this->predicted[i] = model->predicted[i];
        }
    }

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