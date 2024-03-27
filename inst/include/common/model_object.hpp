#ifndef COMMON_MODEL_OBJECT_HPP
#define COMMON_MODEL_OBJECT_HPP

#include <stdint.h>

#include <vector>

#include "def.hpp"


/**
 * @brief ModelObject struct that defines member types and returns the unique id
 */
template <typename Type>
struct ModelObject {
  uint32_t id; /**< unique identifier assigned for all fims objects */
  std::vector<Type*> parameters; /**< list of estimable parameters */
 
  virtual ~ModelObject() {}
  /**
   * @brief Getter that returns the unique id for parameters in the model
   */
  uint32_t GetId() const { return id; }
};


#endif /* COMMON_MODEL_OBJECT_HPP */