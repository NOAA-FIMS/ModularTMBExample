#ifndef DATA_HPP
#define  DATA_HPP

#include <vector>
#include "def.hpp"

/**
 * Input Data Types
 */
template<typename Type>
class ObsData{
public:
    std::vector<Type> ages;
    typename model_traits<Type>::data_vector data;
    static ObsData<Type>* obsdata_ptr;
    
    ObsData(){}
    
    
    static ObsData<Type>* obsdata_new() {
      return ObsData<Type>::obsdata_ptr;
    }

};

template<typename Type>
ObsData<Type>* ObsData<Type>::obsdata_ptr = //new ObsData<Type>;
  Rcpp::XPtr<ObsData<Type>>(new ObsData<Type>);

#endif