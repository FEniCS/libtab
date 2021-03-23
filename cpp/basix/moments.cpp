// Copyright (c) 2020 Chris Richardson & Matthew Scroggs
// FEniCS Project
// SPDX-License-Identifier:    MIT

#include "moments.h"
#include "cell.h"
#include "finite-element.h"
#include "polyset.h"
#include "quadrature.h"
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xbuilder.hpp>
#include <xtensor/xpad.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

using namespace basix;

namespace
{

//----------------------------------------------------------------------------
std::vector<int> axis_points(const cell::type celltype)
{
  switch (celltype)
  {
  case cell::type::interval:
    return {1};
  case cell::type::triangle:
    return {1, 2};
  case cell::type::quadrilateral:
    return {1, 2};
  case cell::type::tetrahedron:
    return {1, 2, 3};
  case cell::type::hexahedron:
    return {1, 2, 4};
  default:
    throw std::runtime_error(
        "Integrals of this entity type not yet implemented.");
  }
}
//----------------------------------------------------------------------------
} // namespace

//-----------------------------------------------------------------------------
xt::xtensor<double, 3> moments::create_dot_moment_dof_transformations(
    const FiniteElement& moment_space)
{
  // This function can be dramatically simplified and made
  // understandable by using tensors to give more logic to the objects

  cell::type celltype = moment_space.cell_type();
  if (celltype == cell::type::point)
    return {};

  xt::xarray<double> pts = moment_space.points();
  if (pts.shape(1) == 1)
    pts.reshape({pts.shape(0)});

  const xt::xtensor<double, 2>& P = moment_space.interpolation_matrix();
  xt::xtensor<double, 3> tpts;
  xt::xtensor<double, 3> J, K;
  switch (celltype)
  {
  case cell::type::interval:
  {
    tpts = xt::atleast_3d(1.0 - pts);
    J = {{{-1.0}}};
    K = {{{-1.0}}};
    break;
  }
  case cell::type::triangle:
  {
    std::array<std::size_t, 3> shape = {2, pts.shape(0), pts.shape(1)};
    tpts = xt::zeros<double>(shape);

    J.resize({2, 2, 2});
    K.resize({2, 2, 2});
    xt::xtensor_fixed<double, xt::xshape<2, 2>> A;

    for (std::size_t i = 0; i < pts.shape(0); ++i)
    {
      tpts(0, i, 0) = pts(i, 1);
      tpts(0, i, 1) = 1 - pts(i, 0) - pts(i, 1);
      tpts(1, i, 0) = pts(i, 1);
      tpts(1, i, 1) = pts(i, 0);
    }

    A = {{0, 1}, {-1, -1}};
    xt::view(J, 0, xt::all(), xt::all()) = A;
    A = {{-1, -1}, {1, 0}};
    xt::view(K, 0, xt::all(), xt::all()) = A;

    A = {{0, 1}, {1, 0}};
    xt::view(J, 1, xt::all(), xt::all()) = A;
    A = {{0, 1}, {1, 0}};
    xt::view(K, 1, xt::all(), xt::all()) = A;

    break;
  }
  case cell::type::quadrilateral:
  {
    std::array<std::size_t, 3> shape0 = {2, pts.shape(0), pts.shape(1)};
    tpts = xt::zeros<double>(shape0);

    J.resize({2, 2, 2});
    K.resize({2, 2, 2});
    xt::xtensor_fixed<double, xt::xshape<2, 2>> A;

    for (std::size_t i = 0; i < pts.shape(0); ++i)
    {
      tpts(0, i, 0) = pts(i, 1);
      tpts(0, i, 1) = 1.0 - pts(i, 0);
      tpts(1, i, 0) = pts(i, 1);
      tpts(1, i, 1) = pts(i, 0);
    }

    A = {{0, 1}, {-1, 0}};
    xt::view(J, 0, xt::all(), xt::all()) = A;
    A = {{0, -1}, {1, 0}};
    xt::view(K, 0, xt::all(), xt::all()) = A;

    A = {{0, 1}, {1, 0}};
    xt::view(J, 1, xt::all(), xt::all()) = A;
    A = {{0, 1}, {1, 0}};
    xt::view(K, 1, xt::all(), xt::all()) = A;

    break;
  }
  default:
  {
    throw std::runtime_error(
        "DOF transformations only implemented for tdim <= 2.");
  }
  }

  std::array<std::size_t, 3> shape
      = {tpts.shape(0), (std::size_t)moment_space.dim(),
         (std::size_t)moment_space.dim()};
  xt::xtensor<double, 3> out = xt::zeros<double>(shape);
  for (std::size_t i = 0; i < tpts.shape(0); ++i)
  {
    auto _tpoint = xt::view(tpts, i, xt::all(), xt::all());
    xt::xtensor<double, 3> moment_space_pts
        = xt::view(moment_space.tabulate_x(0, _tpoint), 0, xt::all(), xt::all(),
                   xt::all());

    // Tile the J and J^-1 for passing into the mapping function. This
    // could be avoided with some changes to calls to map functions
    // taking just one J and J^1
    auto Ji = xt::tile(xt::view(J, i, xt::newaxis(), xt::all(), xt::all()),
                       moment_space_pts.shape(0));
    auto Ki = xt::tile(xt::view(K, i, xt::newaxis(), xt::all(), xt::all()),
                       moment_space_pts.shape(0));

    std::vector<double> detJ(Ji.shape(0), 1.0);

    // Pull back basis function values to the reference cell (applied
    // map)
    xt::xtensor<double, 3> F
        = moment_space.map_pull_back(moment_space_pts, Ji, detJ, Ki);

    // Copy onto 2D array
    xt::xtensor<double, 2> _pulled({F.shape(0), F.shape(1) * F.shape(2)});
    for (std::size_t p = 0; p < F.shape(0); ++p)
    {
      {
        for (std::size_t i = 0; i < F.shape(1); ++i)
          for (std::size_t j = 0; j < F.shape(2); ++j)
            _pulled(p, j * F.shape(1) + i) = F(p, i, j);
      }
    }

    // Apply interpolation matrix to transformed basis function values
    xt::xtensor<double, 2> Pview, phi_transformed;
    for (int v = 0; v < moment_space.value_size(); ++v)
    {
      Pview = xt::view(
          P, xt::range(0, P.shape(0)),
          xt::range(v * _pulled.shape(0), (v + 1) * _pulled.shape(0)));
      phi_transformed = xt::view(
          _pulled, xt::range(0, _pulled.shape(0)),
          xt::range(moment_space.dim() * v, moment_space.dim() * (v + 1)));
      xt::view(out, i, xt::all(), xt::all())
          += xt::linalg::dot(Pview, phi_transformed);
    }
  }

  return out;
}
//----------------------------------------------------------------------------
xt::xtensor<double, 3>
moments::create_moment_dof_transformations(const FiniteElement& moment_space)
{
  const xt::xtensor<double, 3> t
      = create_dot_moment_dof_transformations(moment_space);

  xt::xtensor_fixed<double, xt::xshape<2, 2>> rot, ref;

  cell::type celltype = moment_space.cell_type();
  switch (celltype)
  {
  case cell::type::interval:
    return t;
  case cell::type::triangle:
    rot = {{-1, -1}, {1, 0}};
    ref = {{0, 1}, {1, 0}};
    break;
  case cell::type::quadrilateral:
    // TODO: check that these are correct
    rot = {{0, -1}, {1, 0}};
    ref = {{0, 1}, {1, 0}};
    break;
  default:
    throw std::runtime_error("Unexpected cell type");
  }

  const std::size_t scalar_dofs = t.shape(1);
  xt::xtensor<double, 3> M({2, 2 * scalar_dofs, 2 * scalar_dofs});
  for (std::size_t i = 0; i < scalar_dofs; ++i)
  {
    for (std::size_t j = 0; j < scalar_dofs; ++j)
    {
      xt::view(M, 0, xt::range(2 * i, 2 * i + 2), xt::range(2 * j, 2 * j + 2))
          = t(0, i, j) * rot;
    }
  }

  for (std::size_t i = 0; i < scalar_dofs; ++i)
  {
    for (std::size_t j = 0; j < scalar_dofs; ++j)
    {
      xt::view(M, 1, xt::range(2 * i, 2 * i + 2), xt::range(2 * j, 2 * j + 2))
          = t(1, i, j) * ref;
    }
  }

  return M;
}
//----------------------------------------------------------------------------
xt::xtensor<double, 3> moments::create_normal_moment_dof_transformations(
    const FiniteElement& moment_space)
{
  xt::xtensor<double, 3> t
      = create_dot_moment_dof_transformations(moment_space);
  const int tdim = cell::topological_dimension(moment_space.cell_type());
  if (tdim == 1 or tdim == 2)
    xt::view(t, tdim - 1, xt::all(), xt::all()) *= -1.0;
  return t;
}
//----------------------------------------------------------------------------
xt::xtensor<double, 3> moments::create_tangent_moment_dof_transformations(
    const FiniteElement& moment_space)
{
  const int tdim = cell::topological_dimension(moment_space.cell_type());
  if (tdim != 1)
    throw std::runtime_error("Tangent is only well-defined on an edge.");

  xt::xtensor<double, 3> t
      = create_dot_moment_dof_transformations(moment_space);
  xt::view(t, 0, xt::all(), xt::all()) *= -1.0;

  return t;
}
//----------------------------------------------------------------------------
std::pair<xt::xtensor<double, 2>, xt::xtensor<double, 2>>
moments::make_integral_moments(const FiniteElement& moment_space,
                               cell::type celltype, std::size_t value_size,
                               int q_deg)
{
  const cell::type sub_celltype = moment_space.cell_type();
  const std::size_t sub_entity_dim = cell::topological_dimension(sub_celltype);
  if (sub_entity_dim == 0)
    throw std::runtime_error("Cannot integrate over a dimension 0 entity.");
  const std::size_t sub_entity_count
      = cell::sub_entity_count(celltype, sub_entity_dim);
  const std::size_t tdim = cell::topological_dimension(celltype);

  auto [Qpts, _Qwts]
      = quadrature::make_quadrature("default", sub_celltype, q_deg);
  auto Qwts = xt::adapt(_Qwts);
  if (Qpts.dimension() == 1)
    Qpts = Qpts.reshape({Qpts.shape(0), 1});

  // Evaluate moment space at quadrature points
  xt::xtensor<double, 2> moment_space_at_Qpts
      = xt::view(moment_space.tabulate(0, Qpts), 0, xt::all(), xt::all());

  xt::xtensor<double, 2> points({sub_entity_count * Qpts.shape(0), tdim});
  const std::array<std::size_t, 2> shape
      = {moment_space_at_Qpts.shape(1) * sub_entity_count
             * (value_size == 1 ? 1 : sub_entity_dim),
         sub_entity_count * Qpts.shape(0) * value_size};
  xt::xtensor<double, 2> matrix = xt::zeros<double>(shape);

  // Iterate over sub entities
  int c = 0;
  std::vector<int> axis_pts = axis_points(celltype);
  for (std::size_t i = 0; i < sub_entity_count; ++i)
  {
    xt::xtensor<double, 2> entity
        = cell::sub_entity_geometry(celltype, sub_entity_dim, i);

    // Parametrise entity coordinates
    xt::xtensor<double, 2> axes({sub_entity_dim, tdim});
    for (std::size_t j = 0; j < sub_entity_dim; ++j)
    {
      xt::view(axes, j, xt::all()) = xt::view(entity, axis_pts[j], xt::all())
                                     - xt::view(entity, 0, xt::all());
    }

    // See
    // https://github.com/xtensor-stack/xtensor/issues/1922#issuecomment-586317746
    // for why xt::newaxis() is required
    auto points_view = xt::view(
        points, xt::range(i * Qpts.shape(0), (i + 1) * Qpts.shape(0)),
        xt::range(0, tdim));
    auto p = xt::tile(xt::view(entity, xt::newaxis(), 0), Qpts.shape(0));
    points_view = p + xt::linalg::dot(Qpts, axes);

    // Compute entity integral moments
    for (std::size_t j = 0; j < moment_space_at_Qpts.shape(1); ++j)
    {
      auto phi = xt::col(moment_space_at_Qpts, j);
      if (value_size == 1)
      {
        xt::view(matrix, c,
                 xt::range(i * Qpts.shape(0), (i + 1) * Qpts.shape(0)))
            = phi * Qwts;
        ++c;
      }
      else
      {
        // FIXME: This assumed that the moment space has a certain
        // mapping type
        for (std::size_t d = 0; d < sub_entity_dim; ++d)
        {
          auto axis = xt::row(axes, d);
          for (std::size_t k = 0; k < value_size; ++k)
          {
            std::size_t offset = (k * sub_entity_count + i) * Qpts.shape(0);
            xt::view(matrix, c, xt::range(offset, offset + Qpts.shape(0)))
                = phi * Qwts * axis[k];
          }
          ++c;
        }
      }
    }
  }

  return {points, matrix};
}
//----------------------------------------------------------------------------
std::pair<xt::xtensor<double, 2>, xt::xtensor<double, 2>>
moments::make_dot_integral_moments(const FiniteElement& moment_space,
                                   cell::type celltype, std::size_t value_size,
                                   int q_deg)
{
  const cell::type sub_celltype = moment_space.cell_type();
  const std::size_t entity_dim = cell::topological_dimension(sub_celltype);
  const std::size_t num_entities = cell::sub_entity_count(celltype, entity_dim);
  const std::size_t tdim = cell::topological_dimension(celltype);

  auto [qpts, _qwts]
      = quadrature::make_quadrature("default", sub_celltype, q_deg);
  auto qwts = xt::adapt(_qwts);
  // if (qpts.dimension() == 1)
  //   qpts = qpts.reshape({qpts.shape(0), 1});

  // If this is always true, value_size input can be removed
  assert(tdim == value_size);

  // Evaluate moment space at quadrature points
  xt::xtensor<double, 2> moment_space_at_Qpts
      = xt::view(moment_space.tabulate(0, qpts), 0, xt::all(), xt::all());

  const std::size_t moment_space_size
      = moment_space_at_Qpts.shape(1) / entity_dim;

  xt::xtensor<double, 2> points({num_entities * qpts.shape(0), tdim});
  const std::array<std::size_t, 2> shape
      = {moment_space_size * num_entities,
         num_entities * qpts.shape(0) * value_size};
  xt::xtensor<double, 2> matrix = xt::zeros<double>(shape);

  // Iterate over sub entities
  int c = 0;
  std::vector<int> axis_pts = axis_points(celltype);
  const std::size_t num_points = qpts.shape(0);
  for (std::size_t e = 0; e < num_entities; ++e)
  {
    xt::xtensor<double, 2> entity
        = cell::sub_entity_geometry(celltype, entity_dim, e);

    // Parameterise entity coordinates
    xt::xtensor<double, 2> axes({entity_dim, tdim});
    for (std::size_t j = 0; j < entity_dim; ++j)
      xt::row(axes, j) = xt::row(entity, axis_pts[j]) - xt::row(entity, 0);

    // See
    // https://github.com/xtensor-stack/xtensor/issues/1922#issuecomment-586317746
    // for why xt::newaxis() is required
    auto points_view
        = xt::view(points, xt::range(e * num_points, (e + 1) * num_points),
                   xt::range(0, tdim));
    auto p = xt::tile(xt::view(entity, xt::newaxis(), 0), num_points);
    points_view = p + xt::linalg::dot(qpts, axes);

    // Compute entity integral moments
    for (std::size_t j = 0; j < moment_space_size; ++j)
    {
      for (std::size_t k = 0; k < value_size; ++k)
      {
        auto matrix_view
            = xt::view(matrix, c,
                       xt::range((k * num_entities + e) * num_points,
                                 (k * num_entities + e + 1) * num_points));
        xt::xtensor<double, 1> q = xt::zeros<double>({num_points});
        for (std::size_t d = 0; d < entity_dim; ++d)
        {
          // FIXME: This assumed that the moment space has a certain mapping
          // type
          auto phi = xt::col(moment_space_at_Qpts, d * moment_space_size + j);
          matrix_view += phi * qwts * axes(d, k);
        }
      }
      ++c;
    }
  }

  return {points, matrix};
}
//----------------------------------------------------------------------------
std::pair<xt::xtensor<double, 2>, xt::xtensor<double, 2>>
moments::make_tangent_integral_moments(const FiniteElement& moment_space,
                                       cell::type celltype,
                                       std::size_t value_size, int q_deg)
{
  const cell::type sub_celltype = moment_space.cell_type();
  const std::size_t entity_dim = cell::topological_dimension(sub_celltype);
  const std::size_t num_entities = cell::sub_entity_count(celltype, entity_dim);
  const std::size_t tdim = cell::topological_dimension(celltype);

  if (entity_dim != 1)
    throw std::runtime_error("Tangent is only well-defined on an edge.");

  auto [pts, _wts]
      = quadrature::make_quadrature("default", cell::type::interval, q_deg);
  auto wts = xt::adapt(_wts);

  // If this is always true, value_size input can be removed
  assert(tdim == value_size);

  // Evaluate moment space at quadrature points
  xt::xtensor<double, 2> phi
      = xt::view(moment_space.tabulate(0, pts), 0, xt::all(), xt::all());

  xt::xtensor<double, 2> points({num_entities * pts.shape(0), tdim});

  const std::size_t num_points = num_entities * phi.shape(0);
  const std::size_t num_dofs = num_entities * phi.shape(1);
  const std::array shape = {num_dofs, value_size, num_points};
  xt::xtensor<double, 3> D = xt::zeros<double>(shape);

  // Iterate over cell entities
  int c = 0;
  for (std::size_t e = 0; e < num_entities; ++e)
  {
    xt::xtensor<double, 2> edge = cell::sub_entity_geometry(celltype, 1, e);
    auto X0 = xt::row(edge, 0);
    auto tangent = xt::row(edge, 1) - X0;

    // No need to normalise the tangent, as the size of this is equal to
    // the integral Jacobian

    // Map quadrature points onto triangle edge
    for (std::size_t i = 0; i < pts.shape(0); ++i)
      xt::row(points, e * pts.shape(0) + i) = X0 + pts[i] * tangent;

    // Compute edge tangent integral moments
    auto r = xt::range(e * pts.shape(0), (e + 1) * pts.shape(0));
    for (std::size_t i = 0; i < phi.shape(1); ++i)
    {
      auto phi_i = xt::col(phi, i);
      for (std::size_t j = 0; j < value_size; ++j)
        xt::view(D, c, j, r) = phi_i * wts * tangent[j];
      ++c;
    }
  }

  const std::array s = {num_dofs, num_points * value_size};
  return {points, xt::reshape_view(D, s)};
}
//----------------------------------------------------------------------------
std::pair<xt::xtensor<double, 2>, xt::xtensor<double, 2>>
moments::make_normal_integral_moments(const FiniteElement& moment_space,
                                      cell::type celltype,
                                      std::size_t value_size, int q_deg)
{
  const std::size_t tdim = cell::topological_dimension(celltype);
  assert(tdim == value_size);

  const cell::type sub_celltype = moment_space.cell_type();
  const std::size_t entity_dim = cell::topological_dimension(sub_celltype);
  const std::size_t num_entities = cell::sub_entity_count(celltype, entity_dim);

  if (static_cast<int>(entity_dim) != static_cast<int>(tdim) - 1)
    throw std::runtime_error("Normal is only well-defined on a facet.");

  // Compute quadrature points for evaluating integral
  auto [pts, _wts]
      = quadrature::make_quadrature("default", sub_celltype, q_deg);
  auto wts = xt::adapt(_wts);

  // Evaluate moment space at quadrature points
  xt::xtensor<double, 2> phi
      = xt::view(moment_space.tabulate(0, pts), 0, xt::all(), xt::all());

  // Storage for coordinates of evaluations points in the reference cell
  xt::xtensor<double, 2> points({num_entities * pts.shape(0), tdim});
  auto X = xt::reshape_view(points, {num_entities, pts.shape(0), tdim});

  // Storage for interpolation matrix
  const std::size_t num_points = num_entities * phi.shape(0);
  const std::size_t num_dofs = num_entities * phi.shape(1);
  const std::array shape = {num_dofs, value_size, num_points};
  xt::xtensor<double, 3> D = xt::zeros<double>(shape);

  // Evaluate moment space at quadrature points

  // Iterate over cell entities
  int c = 0;
  xt::xtensor<double, 1> normal;
  for (std::size_t e = 0; e < num_entities; ++e)
  {
    // Map quadrature points onto facet
    xt::xtensor<double, 2> facet_X
        = cell::sub_entity_geometry(celltype, tdim - 1, e);
    auto X0 = xt::row(facet_X, 0);
    if (tdim == 2)
    {
      // No need to normalise the normal, as the size of this is equal
      // to the integral jacobian
      auto tangent = xt::row(facet_X, 1) - X0;
      normal = {-tangent(1), tangent(0)};
      for (std::size_t p = 0; p < pts.shape(0); ++p)
        xt::view(X, e, p, xt::all()) = X0 + pts[p] * tangent;
    }
    else if (tdim == 3)
    {
      // No need to normalise the normal, as the size of this is equal
      // to the integral Jacobian
      auto t0 = xt::row(facet_X, 1) - X0;
      auto t1 = xt::row(facet_X, 2) - X0;
      normal = xt::linalg::cross(t0, t1);
      for (std::size_t p = 0; p < pts.shape(0); ++p)
        xt::view(X, e, p, xt::all()) = X0 + pts(p, 0) * t0 + pts(p, 1) * t1;
    }
    else
      throw std::runtime_error("Normal on this cell cannot be computed.");

    // Compute facet normal integral moments
    auto r = xt::range(e * pts.shape(0), (e + 1) * pts.shape(0));
    for (std::size_t i = 0; i < phi.shape(1); ++i)
    {
      auto phi_i = xt::col(phi, i);
      for (std::size_t j = 0; j < value_size; ++j)
        xt::view(D, c, j, r) = phi_i * wts * normal[j];
      ++c;
    }
  }

  const std::array s = {num_dofs, num_points * value_size};
  return {points, xt::reshape_view(D, s)};
}
//----------------------------------------------------------------------------
