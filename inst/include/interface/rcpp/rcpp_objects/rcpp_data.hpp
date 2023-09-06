#ifndef RCPP_DATA
#define RCPP_DATA


#include "rcpp_interface_base.hpp"

class ObsDataInterface  : public RcppInterfaceBase {
public:
    Rcpp::NumericVector data;
    Rcpp::NumericVector ages;

    virtual bool prepare(){
        
        if(this->data.size() != this->ages.size()){
            Rcpp::stop("ages vector length not equal to data vector length");
        }
        
        ObsData<double>* obsdata;
        
        obsdata->ages.resize(this->ages.size());
        obsdata->data.resize(this->data.size());
        
        for(int i =0; i < this->data.size(); i++){
            obsdata->ages[i] = this->ages[i];
            obsdata->data[i] = this->data[i];
        }
        
    }
    
};
#endif