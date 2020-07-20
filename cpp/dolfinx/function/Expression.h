// Copyright (C) 2020 Jack S. Hale
//
// This file is part of DOLFINX (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#pragma once

#include "Constant.h"
#include "evaluate.h"
#include <dolfinx/fem/FormCoefficients.h>
#include <functional>
#include <utility>
#include <vector>

namespace dolfinx
{
namespace fem
{
template <typename T>
class FormCoefficients;
}

namespace mesh
{
class Mesh;
}

namespace function
{
template <typename T>
class Constant;

/// Represents a mathematical expression evaluated at a pre-defined
/// set of points on the reference cell. Holds fem::FormCoefficients. 
/// This class closely follows the concept of a UFC Expression.

template <typename T>
class Expression
{
public:
  /// Create Expression. UFC Expression callable and mesh should be set later by caller.
  Expression(
      const fem::FormCoefficients<T>& coefficients,
      const std::vector<
          std::pair<std::string, std::shared_ptr<const function::Constant<T>>>>&
          constants)
      : _coefficients(coefficients), _constants(constants), _fn(nullptr), _mesh(nullptr)
  {
    // Do nothing
  }

  /// Create Expression. Members should be set later by caller.
  explicit Expression() : Expression(fem::FormCoefficients<T>({}), {})
  {
    // Do nothing
  }

  /// Move constructor
  Expression(Expression&& form) = default;

  /// Destructor
  virtual ~Expression() = default;

  /// Access coefficients
  fem::FormCoefficients<T>& coefficients() { return _coefficients; }

  /// Access coefficients (const version)
  const fem::FormCoefficients<T>& coefficients() const { return _coefficients; }

  /// Evaluate the expression on cells
  /// @param[in] active_cells Cells on which to evaluate the Expression
  /// @param[in,out] values To store the result. Caller responsible for correct sizing.
  void eval(const Eigen::Ref<const Eigen::Array<std::int32_t, Eigen::Dynamic, 1>>& active_cells,
                const Eigen::Ref<Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>& values)
  {
    function::eval(values, *this, active_cells);
  }

  /// Register the function for tabulate_expression.
  /// @param[in] fn Function to tabulate expression.
  void set_tabulate_expression(
      const std::function<void(T*, const T*, const T*, const double*)> fn)
  {
    _fn = fn;
  }

  /// Set coefficient with given number (shared pointer version)
  /// @param[in] coefficients Map from coefficient index to the
  ///   coefficient
  void set_coefficients(
      const std::map<int, std::shared_ptr<const function::Function<T>>>&
          coefficients)
  {
    for (const auto& c : coefficients)
      _coefficients.set(c.first, c.second);
  }

  /// Set coefficient with given name (shared pointer version)
  /// @param[in] coefficients Map from coefficient name to the
  ///   coefficient
  void set_coefficients(
      const std::map<std::string, std::shared_ptr<const function::Function<T>>>&
          coefficients)
  {
    for (const auto& c : coefficients)
      _coefficients.set(c.first, c.second);
  }

  /// Set constants based on their names
  ///
  /// This method is used in command-line workflow, when users set
  /// constants to the form in cpp file.
  ///
  /// Names of the constants must agree with their names in UFL file.
  void set_constants(
      const std::map<std::string, std::shared_ptr<const function::Constant<T>>>&
          constants)
  {
    for (auto const& constant : constants)
    {
      // Find matching string in existing constants
      const std::string name = constant.first;
      const auto it = std::find_if(
          _constants.begin(), _constants.end(),
          [&](const std::pair<
              std::string, std::shared_ptr<const function::Constant<T>>>& q) {
            return (q.first == name);
          });
      if (it != _constants.end())
        it->second = constant.second;
      else
        throw std::runtime_error("Constant '" + name + "' not found in form");
    }
  }

  /// Set constants based on their order (without names)
  ///
  /// This method is used in Python workflow, when constants are
  /// automatically attached to the expression based on their order in the
  /// original expression.
  ///
  /// The order of constants must match their order in original ufl
  /// expression.
  void
  set_constants(const std::vector<std::shared_ptr<const function::Constant<T>>>&
                    constants)
  {
    // TODO: Why this check? Should resize as necessary.
    // if (constants.size() != _constants.size())
    // throw std::runtime_error("Incorrect number of constants.");
    _constants.resize(constants.size());

    // Loop over each constant that user wants to attach
    for (std::size_t i = 0; i < constants.size(); ++i)
    {
      // In this case, the constants don't have names
      _constants[i] = std::pair("", constants[i]);
    }
  }

  /// Check if all constants associated with the expression have been set
  /// @return True if all Form constants have been set
  bool all_constants_set() const
  {
    for (const auto& constant : _constants)
      if (!constant.second)
        return false;
    return true;
  }

  /// Return names of any constants that have not been set
  /// @return Names of unset constants
  std::set<std::string> get_unset_constants() const
  {
    std::set<std::string> unset;
    for (const auto& constant : _constants)
      if (!constant.second)
        unset.insert(constant.first);
    return unset;
  }

  /// Set mesh
  /// @param[in] mesh The mesh
  void set_mesh(const std::shared_ptr<const mesh::Mesh>& mesh) { _mesh = mesh; }

  /// Get mesh
  /// @return The mesh
  std::shared_ptr<const mesh::Mesh> mesh() const { return _mesh; }

private:
  // Coefficients associated with the Expression
  fem::FormCoefficients<T> _coefficients;

  // Constants associated with the Expression
  std::vector<
      std::pair<std::string, std::shared_ptr<const function::Constant<T>>>>
      _constants;

  // Function to evaluate the Expression
  std::function<void(T*, const T*, const T*, const double*)> _fn;

  // The mesh. Not necessary if the Expression has no Coefficients.
  std::shared_ptr<const mesh::Mesh> _mesh;
};
} // namespace function
} // namespace dolfinx