// Copyright (c) 2020 Chris Richardson
// FEniCS Project
// SPDX-License-Identifier:    MIT

#include "cell.h"

Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
Cell::geometry(Cell::Type celltype)
{
  Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> geom;

  switch (celltype)
  {
  case Cell::Type::interval:
    geom.resize(2, 1);
    geom << 0.0, 1.0;
    break;
  case Cell::Type::triangle:
    geom.resize(3, 2);
    geom << 0.0, 0.0, 1.0, 0.0, 0.0, 1.0;
    break;
  case Cell::Type::quadrilateral:
    geom.resize(4, 2);
    geom << 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0;
    break;
  case Cell::Type::tetrahedron:
    geom.resize(4, 3);
    geom << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0;
    break;
  case Cell::Type::prism:
    geom.resize(6, 3);
    geom << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0,
        0.0, 1.0, 0.0, 1.0, 1.0;
    break;
  case Cell::Type::pyramid:
    geom.resize(5, 3);
    geom << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0,
        0.0, 1.0;
    break;
  case Cell::Type::hexahedron:
    geom.resize(8, 3);
    geom << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0;
    break;
  default:
    throw std::runtime_error("Unsupported cell type");
  }
  return geom;
}
//-----------------------------------------------------------------------------
std::vector<std::vector<std::vector<int>>> Cell::topology(Cell::Type celltype)
{
  std::vector<std::vector<std::vector<int>>> topo;

  switch (celltype)
  {
  case Cell::Type::interval:
    topo.resize(2);
    // Vertices
    topo[0] = {{0}, {1}};
    // Cell
    topo[1] = {{0, 1}};
    break;
  case Cell::Type::triangle:
    topo.resize(3);
    // Vertices
    topo[0] = {{0}, {1}, {2}};
    // Edges
    topo[1] = {{1, 2}, {2, 0}, {0, 1}};
    // Cell
    topo[2] = {{0, 1, 2}};
    break;
  case Cell::Type::quadrilateral:
    topo.resize(3);
    // FIXME - check all these
    // Vertices
    topo[0] = {{0}, {1}, {2}, {3}};
    // Edges
    topo[1] = {{1, 2}, {2, 3}, {3, 0}, {0, 1}};
    // Cell
    topo[2] = {{0, 1, 2, 3}};
    break;
  case Cell::Type::tetrahedron:
    topo.resize(4);
    // Vertices
    topo[0] = {{0}, {1}, {2}, {3}};
    // Edges
    topo[1] = {{2, 3}, {1, 3}, {1, 2}, {0, 3}, {0, 2}, {0, 1}};
    // Faces
    topo[2] = {{1, 2, 3}, {2, 3, 0}, {3, 0, 1}, {0, 1, 2}};
    // Cell
    topo[3] = {{0, 1, 2, 3}};
    break;
  case Cell::Type::prism:
    // FIXME: check
    topo.resize(4);
    // Vertices
    topo[0] = {{0}, {1}, {2}, {3}, {4}, {5}, {6}};
    // Edges
    topo[1] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {1, 4},
               {2, 5}, {3, 4}, {4, 5}, {5, 3}};
    // Faces
    topo[2] = {{0, 1, 2}, {0, 1, 3, 4}, {1, 2, 4, 5}, {2, 0, 5, 3}, {3, 4, 5}};
    // Cell
    topo[3] = {{0, 1, 2, 3, 4, 5}};
    break;
  case Cell::Type::pyramid:
    // FIXME: check all these
    topo.resize(4);
    // Vertices
    topo[0] = {{0}, {1}, {2}, {3}, {4}};
    // Edges
    topo[1] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 4}, {1, 4}, {2, 4}, {3, 4}};
    // Faces
    topo[2] = {{0, 1, 2, 3}, {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}};
    // Cell
    topo[3] = {{0, 1, 2, 3, 4}};
    break;
  case Cell::Type::hexahedron:
    topo.resize(4);
    // FIXME: check over
    // Vertices
    topo[0] = {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}};
    // Edges
    topo[1] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 4}, {1, 5},
               {2, 6}, {3, 7}, {4, 5}, {5, 6}, {6, 7}, {7, 4}};
    // Faces
    topo[2] = {{0, 1, 2, 3}, {0, 1, 4, 5}, {1, 2, 5, 6},
               {2, 3, 6, 7}, {3, 0, 7, 4}, {4, 5, 6, 7}};
    // Cell
    topo[3] = {{0, 1, 2, 3, 4, 5, 6, 7}};
    break;
  default:
    throw std::runtime_error("Unsupported cell type");
  }
  return topo;
}
//-----------------------------------------------------------------------------
int Cell::topological_dimension(Cell::Type cell_type)
{
  switch (cell_type)
  {
  case Cell::Type::interval:
    return 1;
  case Cell::Type::triangle:
    return 2;
  case Cell::Type::quadrilateral:
    return 2;
  case Cell::Type::tetrahedron:
    return 3;
  case Cell::Type::hexahedron:
    return 3;
  case Cell::Type::prism:
    return 3;
  case Cell::Type::pyramid:
    return 3;
  default:
    throw std::runtime_error("Unsupported cell type");
  }
  return 0;
}
//-----------------------------------------------------------------------------
Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
Cell::sub_entity_geometry(Cell::Type celltype, int dim, int index)
{
  std::vector<std::vector<std::vector<int>>> cell_topology
      = Cell::topology(celltype);
  Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
      cell_geometry = Cell::geometry(celltype);

  if (dim < 0 or dim >= (int)cell_topology.size())
    throw std::runtime_error("Invalid dimension for sub-entity");

  const std::vector<std::vector<int>>& t = cell_topology[dim];

  if (index < 0 or index >= (int)t.size())
    throw std::runtime_error("Invalid entity index");

  Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
      sub_entity(t[index].size(), cell_geometry.cols());

  for (int i = 0; i < sub_entity.rows(); ++i)
    sub_entity.row(i) = cell_geometry.row(t[index][i]);

  return sub_entity;
}
//----------------------------------------------------------------------------
Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
Cell::create_lattice(Cell::Type celltype, int n, bool exterior)
{
  Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> points;

  if (celltype == Cell::Type::quadrilateral)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(2,
                                                                             2);
    hs.row(0) << 0.0, 1.0;
    hs.row(1) << 1.0, 0.0;
    hs /= static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 2 * b + 1);
    points.resize(m * m, 2);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      for (int j = b; j < n + 1 - b; ++j)
        points.row(c++) = hs.row(0) * i + hs.row(1) * j;
  }
  else if (celltype == Cell::Type::hexahedron)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(3,
                                                                             3);
    hs.row(0) << 0.0, 0.0, 1.0;
    hs.row(1) << 0.0, 1.0, 0.0;
    hs.row(2) << 1.0, 0.0, 0.0;
    hs /= static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 2 * b + 1);
    points.resize(m * m * m, 3);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      for (int j = b; j < n + 1 - b; ++j)
        for (int k = b; k < n + 1 - b; ++k)
          points.row(c++) = hs.row(0) * i + hs.row(1) * j + hs.row(2) * k;
  }
  else if (celltype == Cell::Type::point)
  {
    points.resize(1, 1);
    points(0, 0) = 0.0;
  }
  else if (celltype == Cell::Type::interval)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(1,
                                                                             1);
    hs.row(0) << 1.0 / static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 2 * b + 1);

    points.resize(m, 1);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      points.row(c++) = hs.row(0) * i;
  }
  else if (celltype == Cell::Type::triangle)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(2,
                                                                             2);
    hs.row(0) << 0.0, 1.0;
    hs.row(1) << 1.0, 0.0;
    hs /= static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 3 * b + 1) * (n - 3 * b + 2) / 2;
    points.resize(m, 2);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      for (int j = b; j < n + 1 - i - b; ++j)
        points.row(c++) = hs.row(1) * j + hs.row(0) * i;
  }
  else if (celltype == Cell::Type::tetrahedron)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(3,
                                                                             3);
    hs.row(0) << 0.0, 0.0, 1.0;
    hs.row(1) << 0.0, 1.0, 0.0;
    hs.row(2) << 1.0, 0.0, 0.0;
    hs /= static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 4 * b + 1) * (n - 4 * b + 2) * (n - 4 * b + 3) / 6;
    points.resize(m, 3);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      for (int j = b; j < n + 1 - i - b; ++j)
        for (int k = b; k < n + 1 - i - j - b; ++k)
          points.row(c++) = hs.row(2) * k + hs.row(1) * j + hs.row(0) * i;
  }
  else if (celltype == Cell::Type::prism)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(3,
                                                                             3);

    hs.row(0) << 0.0, 0.0, 1.0;
    hs.row(1) << 0.0, 1.0, 0.0;
    hs.row(2) << 1.0, 0.0, 0.0;
    hs /= static_cast<double>(n);

    int b = (exterior == false) ? 1 : 0;
    int m = (n - 2 * b + 1) * (n - 3 * b + 1) * (n - 3 * b + 2) / 2;
    points.resize(m, 3);
    int c = 0;
    for (int i = b; i < n + 1 - b; ++i)
      for (int j = b; j < n + 1 - i - b; ++j)
        for (int k = b; k < n + 1 - b; ++k)
          points.row(c++) = hs.row(0) * k + hs.row(1) * j + hs.row(2) * i;
  }
  else if (celltype == Cell::Type::pyramid)
  {
    Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> hs(3,
                                                                             3);

    hs.row(0) << 0.0, 0.0, 1.0;
    hs.row(1) << 0.0, 1.0, 0.0;
    hs.row(2) << 1.0, 0.0, 0.0;
    hs /= static_cast<double>(n);

    if (exterior == false)
      throw std::runtime_error("not implemented in pyramid");

    int m = (n + 1) * (n + 2) * (2 * n + 3) / 6;
    points.resize(m, 3);
    int c = 0;
    for (int k = 0; k < n + 1; ++k)
      for (int i = 0; i < n + 1 - k; ++i)
        for (int j = 0; j < n + 1 - k; ++j)
          points.row(c++) = hs.row(0) * k + hs.row(1) * j + hs.row(2) * i;
  }
  else
    throw std::runtime_error("Unsupported cell for lattice");

  return points;
}
//-----------------------------------------------------------------------------
Cell::Type Cell::simplex_type(int dim)
{
  switch (dim)
  {
  case 0:
    return Cell::Type::point;
  case 1:
    return Cell::Type::interval;
  case 2:
    return Cell::Type::triangle;
  case 3:
    return Cell::Type::tetrahedron;
  default:
    throw std::runtime_error("Unsupported dimension");
  }
}