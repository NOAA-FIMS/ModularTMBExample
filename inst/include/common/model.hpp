
#include "../pop_dy/von_bertalanffy.hpp"

template<typename Type>
class Model{
    public:
    std::vector<double> data; //TODO: make sure data not empty
    std::vector<double> ages;
    VonBertalanffy<Type>* vb;
    std::vector<Type*> parameters;



    //singleton instance based on Type
  static Model<Type>* instance;
  
  /**
   * Returns the sigleton instance of VonBertalanffyModel
   * of type Type.
   */
  static Model<Type>* getInstance(){
    return Model<Type>::instance; //TODO: make sure not null and create if null
  }

     /**
   * Objective function to compute least squares
   * of observed and predicted length.
   */
  Type evaluate(){
    Type norm2 = 0.0;
    for(int i =0; i < ages.size(); i++){
        Type pred = vb -> evaluate(ages[i]);
        norm2+=(pred-data[i])*(pred-data[i]);
    }
    return norm2;
  }
  
  /**
   * clears the estimated parameter list
   */
  void clear(){
    this->parameters.clear();
  }
  
  };

template<typename Type>
Model<Type>* Model<Type>::instance = new Model<Type>();

