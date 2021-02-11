// Copyright (c) 2020 Chris Richardson
// FEniCS Project
// SPDX-License-Identifier:    MIT

#include <Eigen/Core>
#include <utility>
#include <vector>

namespace basix
{

/// Create element in global registry and return handle
/// @return handle
int register_element(const char* family_name, const char* cell_type,
                     int degree);

/// Delete from global registry
/// @param [in] handle Identifier
void release_element(int handle);

/// Tabulate basis values into the memory at "basis_values" with nd derivatives
/// for the points x.
/// The memory for "basis_values" must be allocated by the user, and is a four
/// dimensional (row-major) ndarray with dimensions
/// [(nd+tdim)!/nd!tdim!, value_size, dim, npoints]
/// where "npoints" and "nd" are defined below, "tdim" is the topological dimension of the cell for this
/// element, "dim" is the dimension of the finite element (See `dim()`), and
/// "value_size" is the value size of the finite element (the product of the
/// values in `value_shape()`).
/// "x" points to the memory for a two dimensional (row-major) ndarray with
/// dimensions [npoints, tdim] where "npoints" is defined below, and "tdim" is
/// the topological dimension of the reference element.
///
/// @param [in] handle The handle for the basix element
/// @param [out] basis_values Block of memory to be filled with basis data
/// @param [in] nd Number of derivatives
/// @param [in] x Points at which to evaluate (of size [npoints * tdim])
/// @param [in] npoints Number of points
///
void tabulate(int handle, double* basis_values, int nd, const double* x,
              int npoints);

/// Map a function value from the reference to a physical cell.
/// The memory for "physical_data" must be allocated by the user, and is a two
/// dimensional (row-major) ndarray with dimensions
/// [physical_value_size, dim]
/// where "physical_value_size" is defined below, and "dim" is the dimension of
/// the finite element (See `dim()`). "reference_data" points to the memory for
/// a two dimensional (row-major) ndarray with dimensions [value_size, dim]
/// where "dim" is the dimension of the finite element (See `dim()`), and
/// "value_size" is the value size of the finite element (the product of the
/// values in `value_shape()`).
/// "J" points to the memory for a two dimensional (row-major) ndarray with
/// dimensions [tdim, physical_dim], where "physical_dim" is defined below, and
/// "tdim" is the topological dimension of the cell for this element. "K" points
/// to the memory for a two dimensional (row-major) ndarray with dimensions
/// [physical_dim, tdim].
///
/// @param[in] handle The handle of the basix element
/// @param[out] physical_data The data on the physical cell at the corresponding
/// point
/// @param[in] reference_data The reference data at a single point
/// @param[in] J The Jacobian of the map to the cell (evaluated at the point)
/// @param[in] detJ The determinant of the Jacobian of the map to the cell
/// (evaluated at the point)
/// @param[in] K The inverse of the Jacobian of the map to the cell (evaluated
/// at the point)
/// @param[in] physical_dim The geometric dimension of the physical domain
/// @param[in] physical_value_size The value size of the physical element
void map_push_forward(int handle, double* physical_data,
                      const double* reference_data, const double* J,
                      const double detJ, const double* K,
                      const int physical_dim, const int physical_value_size);

/// Map a function value from a physical cell to the reference
/// @param[in] handle The handle of the basix element
/// @param[in] physical_data The physical data at a single point
/// @param[in] J The Jacobian of the map to the cell (evaluated at the point)
/// @param[in] detJ The determinant of the Jacobian of the map to the cell
/// (evaluated at the point)
/// @param[in] K The inverse of the Jacobian of the map to the cell (evaluated
/// at the point)
/// @return The data on the reference element at the corresponding point
// TODO: remove Eigen from public interface
Eigen::ArrayXXd map_pull_back(int handle, const Eigen::ArrayXd& physical_data,
                              const Eigen::MatrixXd& J, double detJ,
                              const Eigen::MatrixXd& K);

/// String representation of the cell type of the finite element
/// @param handle
/// @return cell type string
const char* cell_type(int handle);

/// Degree
int degree(int handle);

/// Value rank
/// @param handle Identifier
/// @return The number of dimensions of the value shape
int value_rank(int handle);

/// Value shape
/// @param[in] handle Identifier
/// @param[in,out] dimensions Array of value_rank size
void value_shape(int handle, int* dimensions);

/// Finite Element dimension
/// @param [in] handle Identifier
int dim(int handle);

/// Family name
/// @param [in] handle Identifier
const char* family_name(int handle);

/// Mapping name (identity, piola etc.)
/// @param [in] handle Identifier
const char* mapping_name(int handle);

/// Number of dofs per entity of given dimension
/// @param handle Identifier
/// @param dim Entity dimension
/// @param[in,out] num_dofs Number of dofs on each entity
void entity_dofs(int handle, int dim, int* num_dofs);

/// Number of interpolation points
/// @param [in] handle Identifier
int interpolation_num_points(int handle);

/// Interpolation points
/// @param [in] handle Identifier
/// @param [in,out] points The interpolation points
void interpolation_points(int handle, double* points);

/// Interpolation matrix
/// @param [in] handle Identifier
/// @param [in,out] matrix The interpolation matrix
void interpolation_matrix(int handle, double* matrix);

/// Cell geometry number of points (npoints)
/// @param [in] cell_type
/// @return npoints
int cell_geometry_num_points(const char* cell_type);

/// Cell geometric dimension (gdim)
/// @param [in] cell_type
/// @returns gdim
int cell_geometry_dimension(const char* cell_type);

/// Cell points
/// The memory for "x" must be allocated by the user, and is a two
/// dimensional (row-major) ndarray with dimensions [gdim, npoints]
/// where "npoints" is the number of vertices of the cell, and "gdim"
/// is the geometric dimension of the cell.
///
/// @param [in] cell_type
/// @param [out] points Array of size [npoints x gdim]
void cell_geometry(const char* cell_type, double* points);

/// Cell topology
std::vector<std::vector<std::vector<int>>> topology(const char* cell_type);

} // namespace basix
