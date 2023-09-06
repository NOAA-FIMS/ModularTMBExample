#include "../common/data.hpp"
#include "../pop_dy/von_bertalanffy.hpp"

template<typename Type>
class Model{
    public:
    VonBertalanffy<Type>* vb;
    ObsData<Type>* obsdata;
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
    for(int i =0; i < obsdata->ages.size(); i++){
        Type pred = vb -> evaluate(obsdata->ages[i]);
        norm2+=(pred-obsdata->data[i])*(pred-obsdata->data[i]);
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

