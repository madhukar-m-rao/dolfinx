// Copyright (C) 2003-2012 Anders Logg
//
// This file is part of DOLFIN (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#pragma once

#include "FunctionSpace.h"
#include <Eigen/Dense>
#include <dolfin/common/types.h>
#include <dolfin/fem/FiniteElement.h>
#include <dolfin/la/PETScVector.h>
#include <memory>
#include <petscsys.h>
#include <petscvec.h>
#include <vector>

namespace dolfin
{

namespace geometry
{
class BoundingBoxTree;
}
namespace mesh
{
class Mesh;
} // namespace mesh

namespace function
{
/// This class represents a function \f$ u_h \f$ in a finite
/// element function space \f$ V_h \f$, given by
///
/// \f[     u_h = \sum_{i=1}^{n} U_i \phi_i \f]
/// where \f$ \{\phi_i\}_{i=1}^{n} \f$ is a basis for \f$ V_h \f$,
/// and \f$ U \f$ is a vector of expansion coefficients for \f$ u_h \f$.

class Function
{
public:
  /// Create function on given function space
  /// @param[in] V The function space
  explicit Function(std::shared_ptr<const FunctionSpace> V);

  /// Create function on given function space with a given vector
  ///
  /// *Warning: This constructor is intended for internal library use only*
  ///
  /// @param[in] V The function space
  /// @param[in] x The vector
  Function(std::shared_ptr<const FunctionSpace> V, Vec x);

  // Copy constructor
  Function(const Function& v) = delete;

  /// Move constructor
  Function(Function&& v) = default;

  /// Destructor
  virtual ~Function() = default;

  /// Move assignment
  Function& operator=(Function&& v) = default;

  // Assignment
  Function& operator=(const Function& v) = delete;

  /// Extract subfunction (view into the Function)
  /// @param[in] i Index of subfunction
  /// @return The subfunction
  Function sub(int i) const;

  /// Collapse a subfunction (view into the Function) to a stand-alone
  /// Function
  Function collapse() const;

  /// Return shared pointer to function space
  /// @return The function space
  std::shared_ptr<const FunctionSpace> function_space() const;

  /// Return vector of expansion coefficients (non-const version)
  /// @return The vector of expansion coefficients
  la::PETScVector& vector();

  /// Return vector of expansion coefficients (const version)
  /// @return The vector of expansion coefficients
  const la::PETScVector& vector() const;

  /// Interpolate a Function (on possibly non-matching meshes)
  /// @param[in] v The function to be interpolated.
  void interpolate(const Function& v);

  /// Interpolate expression
  /// @param[in] f The expression to be interpolated.
  void interpolate(const FunctionSpace::interpolation_function& f);

  /// Return value rank
  int value_rank() const;

  /// Return value size
  int value_size() const;

  /// Return value dimension for given axis
  /// @param[in] i The index of the axis
  /// @returns The value dimension.
  int value_dimension(int i) const;

  /// Return value shape
  std::vector<int> value_shape() const;

  /// Evaluate the Function at physical points
  /// @param[in] x The physical coordinates of the points. It has shape
  ///              (num_points, 3).
  /// @param[in] cells An array of cell indices. cells[i] is the index
  ///                  of the cell that contains the point x(i).
  ///                  Negative cell indices can be passed, and the
  ///                  corresponding point will be ignored.
  /// @param[in,out] u The values at the points. Values are not computed
  ///                  for points with a negative cell index. This
  ///                  argument must be passed with the corrext size.
  void
  eval(const Eigen::Ref<
           const Eigen::Array<double, Eigen::Dynamic, 3, Eigen::RowMajor>>& x,
       const Eigen::Ref<const Eigen::Array<int, Eigen::Dynamic, 1>>& cells,
       Eigen::Ref<Eigen::Array<PetscScalar, Eigen::Dynamic, Eigen::Dynamic,
                               Eigen::RowMajor>>
           u) const;

  /// Evaluate the Function at points in reference coordinates on all
  /// cells
  /// @warning This function only supports non-manifold Lagrange
  ///          elements
  /// @param[in] X The reference element coordinates of the points. It
  ///              has shape (num_points, 3).
  /// @param[in,out] u The values at the points for each cell.
  //                   The values are ordered by cell. This argument
  //                   must be passed with the corrext size.
  void eval_reference(
      const Eigen::Ref<
          const Eigen::Array<double, Eigen::Dynamic, 3, Eigen::RowMajor>>& X,
      Eigen::Ref<Eigen::Array<PetscScalar, Eigen::Dynamic, Eigen::Dynamic,
                              Eigen::RowMajor>>
          u) const;

  /// Compute values at all mesh points
  /// @return The values at all geometric points
  Eigen::Array<PetscScalar, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
  compute_point_values() const;

  /// Name
  std::string name = "u";

  /// ID
  std::size_t id() const;

private:
  // ID
  std::size_t _id;

  // The function space
  std::shared_ptr<const FunctionSpace> _function_space;

  // The vector of expansion coefficients (local)
  la::PETScVector _vector;
};
} // namespace function
} // namespace dolfin
