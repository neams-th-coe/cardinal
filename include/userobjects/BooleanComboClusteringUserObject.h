#pragma once

#include "GeneralUserObject.h"
#include <unordered_map>

class ClusteringHeuristicUserObjectBase;

/**
 * A clustering user object which implements the mesh walking
 * and clustering process. This is performed by evaluating a user-defined
 * boolean expression involving other heuristic based user objects
 */
class BooleanComboClusteringUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  BooleanComboClusteringUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {};
  virtual void finalize() override {};

private:
  /**
   * getter function for extra element integer score of an element
   * @param[in] a libmesh element
   * @return extra element integer score of that element
   */
  int getExtraIntegerScore(libMesh::Elem * elem) const;

  /**
   * function for converting the expression input to reverse polish notation
   * using the shunting yard algorithm.
   * @param[in] expression a boolean logic expression
   */
  void reversePolishNotation(const std::vector<std::string> & expression);

  /**
   * Method for evaluating if element should be clustered or not
   * @param[in] base_element
   * @param[in] neighbor_elem neighboring element of the base element
   * @return if these two element belong to a cluster or not
   */
  bool belongsToCluster(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem);

  /// this method implements the mesh walking process
  void findCluster();

  /// sets the extra element integer to NOT_VISITED for every active element
  void resetExtraInteger();

  /**
   * Initializes the local cache of clustering user objects from the boolean
   * logic operation expression from the input file
   */
  void initializeUserObjects();

  /// extra element integer id name
  const ExtraElementIDName & _id_name;

  /// mesh ref
  libMesh::MeshBase & _mesh;

  /// element integer index
  unsigned int _extra_integer_index;

  /// Hash map for clustering user object. The key is the name of the UserObject
  std::unordered_map<std::string, const ClusteringHeuristicUserObjectBase *>
      _clustering_user_objects;

  /// hold the final rpn expression
  std::vector<std::string> _output_stack;

  /// operator precedence
  static std::unordered_map<std::string, int> _precedence;

  static constexpr int NOT_VISITED = -1;
  static constexpr std::string_view _left_parenthesis = "(";
  static constexpr std::string_view _right_parenthesis = ")";
};
