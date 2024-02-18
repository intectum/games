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
    uint64_t id = 0; ///< The ID of the physics context.

    vec3 gravity = { 0.0f, -9.8f, 0.0f }; ///< The gravitational force to apply to all dynamic bodies.

    bool visualize = false; ///< Determines if the physics simulation should be visualized. Required data: 1 mesh in the "ludo-bullet::visualizations" partition with line list primitives and the vertex format p3c4.
  };

  ///
  /// A body.
  struct LUDO_API body
  {
    uint64_t id = 0; ///< The ID of the body.
    ludo::transform transform; ///< The transform of the body.
  };

  ///
  /// A static body. The transform of this body should not be changed. Not affected by physics. Can affect dynamic bodies.
  struct LUDO_API static_body : public body
  {
  };

  ///
  /// A dynamic body. The transform of this body is updated during the physics frame function. Affected by physics. Can affect dynamic bodies.
  struct LUDO_API dynamic_body : public body
  {
    float mass = 0.0f; ///< The mass of the body.
    std::vector<std::vector<vec3>> shapes; ///< The shapes of the body (convex hulls).

    vec3 linear_velocity = vec3_zero; ///< The linear velocity of the body.
    vec3 angular_velocity = vec3_zero; ///< The angular velocity of the body.
  };

  ///
  /// A kinematic body. The transform and velocities of this body needs to be updated manually. Not affected by physics. Can affect dynamic bodies.
  struct LUDO_API kinematic_body : public body
  {
    std::vector<std::vector<vec3>> shapes; ///< The shapes of the body (convex hulls).

    vec3 linear_velocity = vec3_zero; ///< The linear velocity of the body (used only for affecting dynamic bodies).
    vec3 angular_velocity = vec3_zero; ///< The angular velocity of the body (used only for affecting dynamic bodies).
  };

  ///
  /// A ghost body. The transform of this body needs to be updated manually. Not affected by physics. Cannot affect other bodies.
  struct LUDO_API ghost_body : public body
  {
    std::vector<std::vector<vec3>> shapes; ///< The shapes of the body (convex hulls).
  };

  ///
  /// A contact between two bodies.
  struct LUDO_API contact
  {
    body* body_a = nullptr; ///< The first body involved in the contact.
    vec3 local_position_a = vec3_zero; ///< The position of the contact on the first body in it's local space.

    body* body_b = nullptr; ///< The second body involved in the contact.
    vec3 local_position_b = vec3_zero; ///< The position of the contact on the second body in it's local space.
    vec3 world_position_b = vec3_zero; ///< The position of the contact on the second body in world space.
    vec3 normal_b = vec3_zero; ///< The normal of the contact relative to the second body in world space.

    float distance = 0.0f; ///< The distance between the two bodies (negative if they are intersecting).
  };

  ///
  /// A constraint.
  struct LUDO_API constraint
  {
    uint64_t id = 0; ///< The ID of the constraint.

    dynamic_body* body_a = nullptr; ///< The first body being constrained.
    dynamic_body* body_b = nullptr; ///< The second body being constrained (optional).

    transform frame_a; ///< The frame of reference fot the first body.
    transform frame_b; ///< The frame of reference fot the second body.

    vec3 linear_lower_limit = vec3_one; ///< The linear lower limits to constrain movement to.
    vec3 linear_upper_limit = vec3_zero; ///< The linear upper limits to constrain movement to.
    vec3 angular_lower_limit = vec3_one; ///< The angular lower limits to constrain rotation to.
    vec3 angular_upper_limit = vec3_zero; ///< The angular upper limits to constrain rotation to.
  };

  ///
  /// Simulates physics for the bodies within the given instance.
  /// \param instance The instance to simulate physics for. Must contain a physics_context.
  /// \param time_step The fixed time step to use in the simulation (0 = not fixed).
  /// \param speed Speed multiplier.
  LUDO_API void simulate_physics(instance& instance, float time_step = 1.0f / 60.0f, float speed = 1.0f);

  template<>
  LUDO_API physics_context* add(instance& instance, const physics_context& init, const std::string& partition);

  template<>
  LUDO_API void remove<physics_context>(instance& instance, physics_context* element, const std::string& partition);

  ///
  /// Pushes the state of a physics context to the physics engine.
  /// \param context The physics context to push.
  LUDO_API void push(const physics_context& physics_context);

  ///
  /// Determines the contacts between the given body and other bodies.
  /// \param instance The instance containing the physical simulation.
  /// \param body The body to determine contacts for.
  /// \return The contacts between the given body and other bodies.
  LUDO_API std::vector<contact> contacts(const instance& instance, const body& body_a);

  ///
  /// Determines the contacts between the given bodies.
  /// \param instance The instance containing the physical simulation.
  /// \param body_a The first body to determine contacts for.
  /// \param body_b The second body to determine contacts for.
  /// \return The contacts between the given bodies.
  LUDO_API std::vector<contact> contacts(const instance& instance, const body& body_a, const body& body_b);

  template<>
  LUDO_API void deallocate<static_body>(instance& instance);

  template<>
  LUDO_API static_body* add(instance& instance, const static_body& init, const std::string& partition);

  template<>
  LUDO_API void remove<static_body>(instance& instance, static_body* element, const std::string& partition);

  ///
  /// Pushes the state of a body to the physics engine.
  /// \param body The body to push.
  LUDO_API void push(const static_body& body);

  ///
  /// Builds the shape of a static body.
  /// \param instance The instance containing the physical simulation.
  /// \param body The body to build the shape for.
  /// \param mesh_id The ID of the mesh used to represent the shape of the body.
  /// \param format The format of the mesh used to represent the shape of the body.
  /// \param index_start The first index within the mesh used to represent the shape of the body.
  /// \param index_count The number of indices within the mesh used to represent the shape of the body.
  /// \return
  LUDO_API task build_shape(instance& instance, static_body& body, uint64_t mesh_id, const vertex_format& format, uint32_t index_start, uint32_t index_count);

  template<>
  LUDO_API void deallocate<dynamic_body>(instance& instance);

  template<>
  LUDO_API dynamic_body* add(instance& instance, const dynamic_body& init, const std::string& partition);

  template<>
  LUDO_API void remove<dynamic_body>(instance& instance, dynamic_body* element, const std::string& partition);

  ///
  /// Pushes the state of a body to the physics engine.
  /// \param body The body to push.
  LUDO_API void push(const dynamic_body& body);

  ///
  /// Pulls the state of a body from the physics engine.
  /// \param body The body to pull.
  LUDO_API void pull(dynamic_body& body);

  ///
  /// Applies a force to a position on the body.
  /// \param body The body.
  /// \param force The force to apply.
  /// \param position The position at which to apply the force.
  LUDO_API void apply_force(dynamic_body& body, const vec3& force, const vec3& position = vec3_zero);

  ///
  /// Applies an impulse to a position on the body.
  /// \param body The body.
  /// \param impulse The impulse to apply.
  /// \param position The position at which to apply the impulse.
  LUDO_API void apply_impulse(dynamic_body& body, const vec3& impulse, const vec3& position = vec3_zero);

  ///
  /// Applies torque to the body.
  /// \param body The body.
  /// \param torque The torque to apply.
  LUDO_API void apply_torque(dynamic_body& body, const vec3& torque);

  ///
  /// Applies a torque impulse to the body.
  /// \param body The body.
  /// \param torque_impulse The torque impulse to apply.
  LUDO_API void apply_torque_impulse(dynamic_body& body, const vec3& torque_impulse);

  ///
  /// Clears any forces acting on a body.
  /// \param body The body.
  LUDO_API void clear_forces(dynamic_body& body);

  template<>
  LUDO_API void deallocate<kinematic_body>(instance& instance);

  template<>
  LUDO_API kinematic_body* add(instance& instance, const kinematic_body& init, const std::string& partition);

  template<>
  LUDO_API void remove<kinematic_body>(instance& instance, kinematic_body* element, const std::string& partition);

  ///
  /// Pushes the state of a body to the physics engine.
  /// \param body The body to push.
  LUDO_API void push(const kinematic_body& body);

  template<>
  LUDO_API void deallocate<ghost_body>(instance& instance);

  template<>
  LUDO_API ghost_body* add(instance& instance, const ghost_body& init, const std::string& partition);

  template<>
  LUDO_API void remove<ghost_body>(instance& instance, ghost_body* element, const std::string& partition);

  ///
  /// Pushes the state of a body to the physics engine.
  /// \param body The body to push.
  LUDO_API void push(const ghost_body& body);

  template<>
  LUDO_API void deallocate<constraint>(instance& instance);

  template<>
  LUDO_API constraint* add(instance& instance, const constraint& init, const std::string& partition);

  template<>
  LUDO_API void remove<constraint>(instance& instance, constraint* element, const std::string& partition);

  ///
  /// Pushes the state of a constraint to the physics engine.
  /// \param constraint The constraint to push.
  LUDO_API void push(const constraint& constraint);
}

#endif // LUDO_PHYSICS_H
