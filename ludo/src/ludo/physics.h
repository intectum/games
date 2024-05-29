/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_PHYSICS_H
#define LUDO_PHYSICS_H

#include "core.h"
#include "meshes.h"
#include "math/transform.h"
#include "math/vec.h"
#include "rendering.h"
#include "tasks.h"

namespace ludo
{
  ///
  /// A physics context.
  struct LUDO_API physics_context
  {
    uint64_t id = 0; ///< A unique identifier.

    vec3 gravity = { 0.0f, -9.8f, 0.0f }; ///< The gravitational force to apply to all dynamic bodies.
  };

  ///
  /// A static body.
  /// The transform of this body should not be changed.
  /// Not affected by physics.
  /// Can affect dynamic bodies.
  struct LUDO_API static_body
  {
    uint64_t id = 0; ///< A unique identifier.

    ludo::transform transform; ///< The transform.
  };

  ///
  /// A dynamic body.
  /// The transform of this body is updated during the physics frame function.
  /// Affected by physics.
  /// Can affect dynamic bodies.
  struct LUDO_API dynamic_body
  {
    uint64_t id = 0; ///< A unique identifier.

    ludo::transform transform; ///< The transform.

    float mass = 0.0f; ///< The mass.
    vec3 linear_velocity = vec3_zero; ///< The linear velocity.
    vec3 angular_velocity = vec3_zero; ///< The angular velocity.
  };

  ///
  /// A kinematic body.
  /// The transform and velocities of this body needs to be updated manually.
  /// Not affected by physics.
  /// Can affect dynamic bodies.
  struct LUDO_API kinematic_body
  {
    uint64_t id = 0; ///< A unique identifier.

    ludo::transform transform; ///< The transform.

    vec3 linear_velocity = vec3_zero; ///< The linear velocity.
    vec3 angular_velocity = vec3_zero; ///< The angular velocity.
  };

  ///
  /// A ghost body.
  /// The transform of this body needs to be updated manually.
  /// Not affected by physics.
  /// Cannot affect other bodies.
  struct LUDO_API ghost_body
  {
    uint64_t id = 0; ///< A unique identifier.

    ludo::transform transform; ///< The transform.
  };

  ///
  /// A dynamic body shape.
  /// The shape is a collection of convex hulls that are applicable to dynamic bodies.
  struct LUDO_API dynamic_body_shape
  {
    uint64_t id = 0; ///< A unique identifier.

    std::vector<std::vector<vec3>> convex_hulls; ///< The convex hulls that make up the shape.
  };

  ///
  /// A contact between two bodies.
  struct LUDO_API contact
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
  struct LUDO_API constraint
  {
    uint64_t id = 0; ///< A unique identifier.

    dynamic_body* body_a = nullptr; ///< The first body being constrained.
    dynamic_body* body_b = nullptr; ///< The second body being constrained (optional).

    transform frame_a; ///< The frame of reference for the first body.
    transform frame_b; ///< The frame of reference for the second body.

    vec3 linear_lower_limit = vec3_one; ///< The linear lower limits to constrain movement to.
    vec3 linear_upper_limit = vec3_zero; ///< The linear upper limits to constrain movement to.
    vec3 angular_lower_limit = vec3_one; ///< The angular lower limits to constrain rotation to.
    vec3 angular_upper_limit = vec3_zero; ///< The angular upper limits to constrain rotation to.
  };

  ///
  /// Initializes a physics context.
  /// \param physics_context The physics context.
  LUDO_API void init(physics_context& physics_context);

  ///
  /// De-initializes a physics context.
  /// \param physics_context The physics context.
  LUDO_API void de_init(physics_context& physics_context);

  ///
  /// Commits the state of a physics context to the physics engine.
  /// \param context The physics context.
  LUDO_API void commit(const physics_context& physics_context);

  ///
  /// Simulates physics.
  /// \param physics_context The physics context.
  /// \param delta_time The time since the last simulation.
  LUDO_API void simulate(physics_context& physics_context, float delta_time);

  ///
  /// Visualizes physics.
  /// \param physics_context The physics context.
  /// \param mesh The mesh to write physics visualization data to.
  LUDO_API void visualize(const physics_context& physics_context, mesh& mesh);

  ///
  /// Determines the contacts between the given body and other bodies.
  /// \param physics_context The physics context.
  /// \param body_a_id The body to determine contacts for.
  /// \return The contacts between the given body and other bodies.
  LUDO_API std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id);

  ///
  /// Determines the contacts between the given bodies.
  /// \param physics_context The physics context.
  /// \param body_a_id The first body to determine contacts for.
  /// \param body_b_id The second body to determine contacts for.
  /// \return The contacts between the given bodies.
  LUDO_API std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id, uint64_t body_b_id);

  ///
  /// Initializes a static body.
  /// \param static_body The static body.
  /// \param physics_context The physics context.
  LUDO_API void init(static_body& static_body, physics_context& physics_context);

  ///
  /// De-initializes a static body.
  /// \param static_body The static body.
  /// \param physics_context The physics context.
  LUDO_API void de_init(static_body& static_body, physics_context& physics_context);

  ///
  /// Connects a static body to a mesh.
  /// \param static_body The static body.
  /// \param physics_context The physics context.
  /// \param mesh The mesh.
  /// \param vertex_format The vertex format of the mesh.
  LUDO_API void connect(static_body& static_body, physics_context& physics_context, const mesh& mesh, const vertex_format& format);

  ///
  /// Commits the state of a static body to the physics engine.
  /// \param static_body The static body.
  LUDO_API void commit(const static_body& static_body);

  ///
  /// Initializes a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param physics_context The physics context.
  LUDO_API void init(dynamic_body& dynamic_body, physics_context& physics_context);

  ///
  /// De-initializes a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param physics_context The physics context.
  LUDO_API void de_init(dynamic_body& dynamic_body, physics_context& physics_context);

  ///
  /// Connects a dynamic body to a dynamic body shape.
  /// \param dynamic_body The dynamic body.
  /// \param physics_context The physics context.
  /// \param dynamic_body_shape The dynamic body shape.
  LUDO_API void connect(dynamic_body& dynamic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);

  ///
  /// Commits the state of a dynamic body to the physics engine.
  /// \param dynamic_body The dynamic body.
  LUDO_API void commit(const dynamic_body& dynamic_body);

  ///
  /// Fetches the state of a dynamic body from the physics engine.
  /// \param dynamic_body The dynamic body.
  LUDO_API void fetch(dynamic_body& dynamic_body);

  ///
  /// Applies a force to a position on a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param force The force to apply.
  /// \param position The position at which to apply the force.
  LUDO_API void apply_force(dynamic_body& dynamic_body, const vec3& force, const vec3& position = vec3_zero);

  ///
  /// Applies an impulse to a position on a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param impulse The impulse to apply.
  /// \param position The position at which to apply the impulse.
  LUDO_API void apply_impulse(dynamic_body& dynamic_body, const vec3& impulse, const vec3& position = vec3_zero);

  ///
  /// Applies torque to a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param torque The torque to apply.
  LUDO_API void apply_torque(dynamic_body& dynamic_body, const vec3& torque);

  ///
  /// Applies a torque impulse to a dynamic body.
  /// \param dynamic_body The dynamic body.
  /// \param torque_impulse The torque impulse to apply.
  LUDO_API void apply_torque_impulse(dynamic_body& body, const vec3& torque_impulse);

  ///
  /// Clears any forces acting on a dynamic body.
  /// \param dynamic_body The dynamic body.
  LUDO_API void clear_forces(dynamic_body& dynamic_body);

  ///
  /// Initializes a kinematic body.
  /// \param kinematic_body The kinematic body.
  /// \param physics_context The physics context.
  LUDO_API void init(kinematic_body& kinematic_body, physics_context& physics_context);

  ///
  /// De-initializes a kinematic body.
  /// \param kinematic_body The kinematic body.
  /// \param physics_context The physics context.
  LUDO_API void de_init(kinematic_body& kinematic_body, physics_context& physics_context);

  ///
  /// Connects a kinematic body to body shapes.
  /// \param kinematic_body The kinematic body.
  /// \param physics_context The physics context.
  /// \param dynamic_body_shape The dynamic body shape.
  LUDO_API void connect(kinematic_body& kinematic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);

  ///
  /// Commits the state of a kinematic body to the physics engine.
  /// \param kinematic_body The kinematic body.
  LUDO_API void commit(const kinematic_body& kinematic_body);

  ///
  /// Fetches the state of a kinematic body from the physics engine.
  /// \param kinematic_body The kinematic body.
  LUDO_API void fetch(kinematic_body& kinematic_body);

  ///
  /// Initializes a ghost body.
  /// \param ghost_body The ghost body.
  /// \param physics_context The physics context.
  LUDO_API void init(ghost_body& ghost_body, physics_context& physics_context);

  ///
  /// De-initializes a ghost body.
  /// \param ghost_body The ghost body.
  /// \param physics_context The physics context.
  LUDO_API void de_init(ghost_body& ghost_body, physics_context& physics_context);

  ///
  /// Connects a ghost body to body shapes.
  /// \param ghost_body The ghost body.
  /// \param physics_context The physics context.
  /// \param body_shapes The body shapes.
  /// \param dynamic_body_shape The dynamic body shape.
  LUDO_API void connect(ghost_body& ghost_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape);

  ///
  /// Commits the state of a ghost body to the physics engine.
  /// \param ghost_body The ghost body.
  LUDO_API void commit(const ghost_body& ghost_body);

  ///
  /// Fetches the state of a ghost body from the physics engine.
  /// \param ghost_body The ghost body.
  LUDO_API void fetch(ghost_body& ghost_body);

  ///
  /// Initializes a dynamic body shape.
  /// \param dynamic_body_shape The dynamic body shape.
  LUDO_API void init(dynamic_body_shape& dynamic_body_shape);

  ///
  /// De-initializes a dynamic body shape.
  /// \param dynamic_body_shape The dynamic body shape.
  LUDO_API void de_init(dynamic_body_shape& dynamic_body_shape);

  ///
  /// Initializes a constraint.
  /// \param constraint The constraint.
  LUDO_API void init(constraint& constraint, physics_context& physics_context);

  ///
  /// De-initializes a constraint.
  /// \param constraint The constraint.
  LUDO_API void de_init(constraint& constraint, physics_context& physics_context);

  ///
  /// Commits the state of a constraint to the physics engine.
  /// \param constraint The constraint.
  LUDO_API void commit(const constraint& constraint);
}

#endif // LUDO_PHYSICS_H
