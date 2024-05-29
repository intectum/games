/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_MESHES_SHAPES_H
#define LUDO_MESHES_SHAPES_H

#include "../meshes.h"

namespace ludo
{
  ///
  /// A set of options for building a shape.
  struct LUDO_API shape_options
  {
    vec3 center = vec3_zero; ///< The center position of the shape.
    vec3 dimensions = vec3_one; ///< The dimensions of the shape. Of the form [width/diameter,height,depth].
    uint32_t divisions = 1; ///< The number of divisions in the shape.

    bool outward_faces = true; ///< Determines if the outward faces should be included. If this is true, the shape will be visible from the outside.
    bool inward_faces = false; ///< Determines if the inward faces should be included. If this is true, the shape will be visible from the inside.

    bool smooth = false; ///< Determines if the shape should use smooth shading (only applicable to the cylinder and spheres).

    vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f); ///< The color of the shape.
  };

  ///
  /// Builds a box within a mesh.
  /// \param mesh The mesh to build the box in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the box at.
  /// \param start_vertex The vertex to start building the box at.
  /// \param options The options used to build the box.
  LUDO_API void box(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a box.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the box.
  /// \return The total and unique vertex counts of a box. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> box_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a circle within a mesh.
  /// \param mesh The mesh to build the circle in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the circle at.
  /// \param start_vertex The vertex to start building the circle at.
  /// \param options The options used to build the circle.
  LUDO_API void circle(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a circle.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the circle.
  /// \return The total and unique vertex counts of a circle. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> circle_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a cylinder within a mesh.
  /// \param mesh The mesh to build the cylinder in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the cylinder at.
  /// \param start_vertex The vertex to start building the cylinder at.
  /// \param options The options used to build the cylinder.
  LUDO_API void cylinder(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a cylinder.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the cylinder.
  /// \return The total and unique vertex counts of a cylinder. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> cylinder_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a rectangle within a mesh.
  /// \param mesh The mesh to build the rectangle in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the rectangle at.
  /// \param start_vertex The vertex to start building the rectangle at.
  /// \param options The options used to build the rectangle.
  LUDO_API void rectangle(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a rectangle.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the rectangle.
  /// \return The total and unique vertex counts of a rectangle. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> rectangle_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a cube-based sphere within a mesh.
  /// \param mesh The mesh to build the sphere in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the sphere at.
  /// \param start_vertex The vertex to start building the sphere at.
  /// \param options The options used to build the sphere.
  /// \param spherified Determines if the points of the cube should be 'spherified' instead of just normalized.
  LUDO_API void sphere_cube(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {}, bool spherified = true);

  ///
  /// Determines the total and unique vertex counts in a cube-based sphere.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the sphere.
  /// \return The total and unique vertex counts of a cube-based sphere. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> sphere_cube_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a icosahedron-based sphere within a mesh.
  /// \param mesh The mesh to build the sphere in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the sphere at.
  /// \param start_vertex The vertex to start building the sphere at.
  /// \param options The options used to build the sphere.
  LUDO_API void sphere_ico(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in an icosahedron-based sphere.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the sphere.
  /// \return The total and unique vertex counts of an icosahedron-based sphere. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> sphere_ico_counts(const vertex_format& format, const shape_options& options = {});

  ///
  /// Builds a UV sphere within a mesh.
  /// \param mesh The mesh to build the sphere in.
  /// \param format The vertex format of the mesh.
  /// \param start_index The index to start building the sphere at.
  /// \param start_vertex The vertex to start building the sphere at.
  /// \param options The options used to build the sphere.
  LUDO_API void sphere_uv(mesh& mesh, const vertex_format& format, uint32_t start_index, uint32_t start_vertex, const shape_options& options = {});

  ///
  /// Determines the total and unique vertex counts in a UV sphere.
  /// \param format The vertex format of the mesh.
  /// \param options The options used to build the sphere.
  /// \return The total and unique vertex counts of a UV sphere. Of the form { total, unique }.
  LUDO_API std::pair<uint32_t, uint32_t> sphere_uv_counts(const vertex_format& format, const shape_options& options = {});
}

#endif // LUDO_MESHES_SHAPES_H
