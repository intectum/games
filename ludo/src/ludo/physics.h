/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#pragma once

#include "math/quat.h"
#include "math/mat.h"
#include "meshes.h"

namespace ludo
{
  // TODO
  /*///
  /// A dynamic body shape.
  /// The shape is a collection of convex hulls that are applicable to dynamic bodies.
  struct dynamic_body_shape
  {
    uint64_t id = 0; ///< A unique identifier.

    std::vector<std::vector<vec3>> convex_hulls; ///< The convex hulls that make up the shape.
  };

  ///
  /// A contact between two bodies.
  struct contact
  {
    uint64_t body_a_id = 0; ///< The first body.
    vec3 local_position_a = vec3_zero; ///< The position of the contact on the first body in it's local space.

    uint64_t body_b_id = 0; ///< The second body.
    vec3 local_position_b = vec3_zero; ///< The position of the contact on the second body in it's local space.
    vec3 world_position_b = vec3_zero; ///< The position of the contact on the second body in world space.
    vec3 normal_b = vec3_zero; ///< The normal of the contact relative to the second body in world space.

    float distance = 0.0f; ///< The distance between the two bodies (negative if they are intersecting).
  };

  ///
  /// A constraint.
  struct constraint
  {
    uint64_t id = 0; ///< A unique identifier.

    dynamic_body* body_a = nullptr; ///< The first body being constrained.
    dynamic_body* body_b = nullptr; ///< The second body being constrained (optional).

    mat4 frame_a; ///< The frame of reference for the first body.
    mat4 frame_b; ///< The frame of reference for the second body.

    vec3 linear_lower_limit = vec3_one; ///< The linear lower limits to constrain movement to.
    vec3 linear_upper_limit = vec3_zero; ///< The linear upper limits to constrain movement to.
    vec3 angular_lower_limit = vec3_one; ///< The angular lower limits to constrain rotation to.
    vec3 angular_upper_limit = vec3_zero; ///< The angular upper limits to constrain rotation to.
  };

  ///
  /// Visualizes physics.
  /// \param physics_context The physics context.
  /// \param mesh The mesh to write physics visualization data to.
  void visualize(const physics_context& physics_context, mesh& mesh);

  ///
  /// Determines the contacts between the given body and other bodies.
  /// \param physics_context The physics context.
  /// \param body_a_id The body to determine contacts for.
  /// \return The contacts between the given body and other bodies.
  std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id);

  ///
  /// Determines the contacts between the given bodies.
  /// \param physics_context The physics context.
  /// \param body_a_id The first body to determine contacts for.
  /// \param body_b_id The second body to determine contacts for.
  /// \return The contacts between the given bodies.
  std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id, uint64_t body_b_id);

  ///
  /// Connects a static body to a mesh.
  /// \param static_body The static body.
  /// \param physics_context The physics context.
  /// \param mesh The mesh.
  /// \param vertex_format The vertex format of the mesh.
  void connect(static_body& static_body, physics_context& physics_context, const mesh& mesh, const vertex_format& format);

  ///
  /// Connects a dynamic body to a dynamic body shape.
  /// \param dynamic_body The dynamic body.
  /// \param physics_context The physics context.
  /// \param dynamic_body_shape The dynamic body shape.
  void connect(dynamic_body& dynamic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);

  ///
  /// Connects a kinematic body to body shapes.
  /// \param kinematic_body The kinematic body.
  /// \param physics_context The physics context.
  /// \param dynamic_body_shape The dynamic body shape.
  void connect(kinematic_body& kinematic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);

  ///
  /// Connects a ghost body to body shapes.
  /// \param ghost_body The ghost body.
  /// \param physics_context The physics context.
  /// \param body_shapes The body shapes.
  /// \param dynamic_body_shape The dynamic body shape.
  void connect(ghost_body& ghost_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);*/
}
