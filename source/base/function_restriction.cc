// ---------------------------------------------------------------------
//
// Copyright (C) 2010 - 2021 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------

#include <deal.II/base/bounding_box.h>
#include <deal.II/base/function_restriction.h>

DEAL_II_NAMESPACE_OPEN
namespace internal
{
  template <int dim>
  Point<dim + 1>
  create_higher_dim_point(const Point<dim> & point,
                          const unsigned int component_in_dim_plus_1,
                          const double       coordinate_value)
  {
    AssertIndexRange(component_in_dim_plus_1, dim + 1);

    Point<dim + 1> output;
    output(component_in_dim_plus_1) = coordinate_value;
    for (int d = 0; d < dim; ++d)
      {
        const unsigned int component_to_write_to =
          dealii::internal::coordinate_to_one_dim_higher<dim>(
            component_in_dim_plus_1, d);
        output(component_to_write_to) = point(d);
      }

    return output;
  }
} // namespace internal



namespace Functions
{
  template <int dim>
  CoordinateRestriction<dim>::CoordinateRestriction(
    const Function<dim + 1> &function,
    const unsigned int       direction,
    const double             coordinate_value)
    : function(&function)
    , restricted_direction(direction)
    , coordinate_value(coordinate_value)
  {
    AssertIndexRange(restricted_direction, dim + 1);
  }



  template <int dim>
  double
  CoordinateRestriction<dim>::value(const Point<dim> & point,
                                    const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point,
                                        restricted_direction,
                                        coordinate_value);

    return function->value(full_point, component);
  }



  template <int dim>
  Tensor<1, dim>
  CoordinateRestriction<dim>::gradient(const Point<dim> & point,
                                       const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point,
                                        restricted_direction,
                                        coordinate_value);

    const Tensor<1, dim + 1> full_gradient =
      function->gradient(full_point, component);

    // CoordinateRestriction is constant in restricted direction. Through away
    // the derivatives with respect to this direction and copy the other
    // values.
    Tensor<1, dim> grad;
    for (int d = 0; d < dim; ++d)
      {
        const unsigned int index_to_write_from =
          internal::coordinate_to_one_dim_higher<dim>(restricted_direction, d);
        grad[d] = full_gradient[index_to_write_from];
      }
    return grad;
  }



  template <int dim>
  SymmetricTensor<2, dim>
  CoordinateRestriction<dim>::hessian(const Point<dim> & point,
                                      const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point,
                                        restricted_direction,
                                        coordinate_value);

    const Tensor<2, dim + 1> full_hessian =
      function->hessian(full_point, component);

    // CoordinateRestriction is constant in restricted direction. Through away
    // the derivatives with respect to this direction and copy the other
    // values.
    SymmetricTensor<2, dim> hess;
    for (int i = 0; i < dim; ++i)
      {
        const unsigned int i_to_write_from =
          internal::coordinate_to_one_dim_higher<dim>(restricted_direction, i);
        for (int j = 0; j < dim; ++j)
          {
            const unsigned int j_to_write_from =
              internal::coordinate_to_one_dim_higher<dim>(restricted_direction,
                                                          j);
            hess[i][j] = full_hessian[i_to_write_from][j_to_write_from];
          }
      }
    return hess;
  }



  template <int dim>
  PointRestriction<dim>::PointRestriction(const Function<dim + 1> &function,
                                          const unsigned int open_direction,
                                          const Point<dim> & point)
    : function(&function)
    , open_direction(open_direction)
    , point(point)
  {
    AssertIndexRange(open_direction, dim + 1);
  }



  template <int dim>
  double
  PointRestriction<dim>::value(const Point<1> &   point_1D,
                               const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point, open_direction, point_1D(0));
    return function->value(full_point, component);
  }



  template <int dim>
  Tensor<1, 1>
  PointRestriction<dim>::gradient(const Point<1> &   point_1D,
                                  const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point, open_direction, point_1D(0));
    const Tensor<1, dim + 1> full_gradient =
      function->gradient(full_point, component);

    // The PointRestrictions is constant in all but the open direction. Throw
    // away the derivatives in all but this direction.
    Tensor<1, 1> grad;
    grad[0] = full_gradient[open_direction];
    return grad;
  }



  template <int dim>
  SymmetricTensor<2, 1>
  PointRestriction<dim>::hessian(const Point<1> &   point_1D,
                                 const unsigned int component) const
  {
    const Point<dim + 1> full_point =
      internal::create_higher_dim_point(point, open_direction, point_1D(0));
    const Tensor<2, dim + 1> full_hessian =
      function->hessian(full_point, component);

    // The PointRestrictions is constant in all but the open direction. Throw
    // away the derivatives in all but this direction.
    SymmetricTensor<2, 1> hess;
    hess[0][0] = full_hessian[open_direction][open_direction];
    return hess;
  }



} // namespace Functions
#include "function_restriction.inst"
DEAL_II_NAMESPACE_CLOSE
