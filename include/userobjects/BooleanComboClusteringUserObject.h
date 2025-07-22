#pragma once

#include "ClusteringHeuristicUserObjectBase.h"
#include <unordered_map>

class BooleanComboClusteringUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  BooleanComboClusteringUserObject(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {};
  virtual void finalize() override {};

  ///getter function for extra element integer
  int getExtraIntegerScore(libMesh::Elem * elem) const;

private:
  /*function for converting the expression input to rpn
   * it follows the  shunting yard algorithm
   */
  void reversePolishNotation(const std::vector<std::string>& expression);

protected:

  /// method for evaluting if element should be clusterd or not
  bool belongsToCluster(libMesh::Elem * base_element, libMesh::Elem * neighbor_elem);

  /// this method implements the mesh walking process
  void findCluster();

  /// sets the extra element integer to -1 for every active elemnet
  void resetExtraInteger();

  /// sets the ref to heuristic based user objects from the expression
  void initializeUserObjects();

  /// eeiid name
  const ExtraElementIDName _id_name;

  /// mesh ref
  libMesh::MeshBase & _mesh;

  /// element integer idex
  unsigned int _extra_integer_index;

  /// hash map for clustering user object.
  std::unordered_map< std::string, const ClusteringHeuristicUserObjectBase* > _clustering_user_objects;

  /// hold the final rpn expression
  std::vector<std::string> _output_stack;

  /// operator predecence
  static const std::unordered_map<std::string, int> _precedence { {"not", 3}, {"!", 3}, {"and", 2}, {"&&", 2}, {"or", 1}, {"||", 1} };

  static constexpr int NOT_VISITED = -1;
  static constexpr std::string _left_parenthesis = "(";
  static constexpr std::string _right_parenthesis = ")";
};