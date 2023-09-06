#ifndef DATA_HPP
#define  DATA_HPP

#include <vector>
#include "../interface/interface.hpp"

/**
 * Input Data Types
 */
template<typename Type>
class ObsData{
public:
    std::vector<Type> ages;
    typename model_traits<Type>::data_vector data;

};

#endif