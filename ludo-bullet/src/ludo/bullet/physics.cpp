/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <memory>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <ludo/algorithm.h>
#include <ludo/data.h>
#include <ludo/physics.h>

#include "debug.h"
#include "math.h"

namespace ludo
{
  btRigidBody* build_bullet_body(const instance& instance, body* body, const std::vector<uint64_t>& body_shape_ids, float mass, bool kinematic, bool ghost);
  void destroy_bullet_body(btRigidBody* bullet_body);
  void update_pointers(static_body* begin, static_body* end);
  void update_pointers(dynamic_body* begin, dynamic_body* end);
  void update_pointers(kinematic_body* begin, kinematic_body* end);
  void update_pointers(ghost_body* begin, ghost_body* end);

  struct contact_result_callback : public btCollisionWorld::ContactResultCallback
  {
    std::vector<contact> contacts;

    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
    {
      auto bullet_body_a = btRigidBody::upcast(colObj0Wrap->getCollisionObject());
      auto bullet_body_b = btRigidBody::upcast(colObj1Wrap->getCollisionObject());

      if (!bullet_body_a || !bullet_body_b)
      {
        return 0;
      }

      contacts.emplace_back(contact
      {
        .body_a = static_cast<body*>(bullet_body_a->getUserPointer()),
        .local_position_a = to_vec3(cp.m_localPointA),

        .body_b = static_cast<body*>(bullet_body_b->getUserPointer()),
        .local_position_b = to_vec3(cp.m_localPointB),
        .world_position_b = to_vec3(cp.m_positionWorldOnB),
        .normal_b = to_vec3(cp.m_normalWorldOnB),

        .distance = cp.m_distance1
      });

      return 0;
    }
  };

  struct motion_state : public btMotionState
  {
    ludo::body* body;

    explicit motion_state(ludo::body* body) : body(body)
    {
    }

    void getWorldTransform(btTransform& world_transform) const override
    {
      world_transform = to_btTransform(body->transform);
    }

    void setWorldTransform(const btTransform& world_transform) override
    {
      body->transform = to_transform(world_transform);
    }
  };

  void simulate_physics(instance& instance, float time_step, float speed)
  {
    auto physics_context = first<ludo::physics_context>(instance);

    assert(physics_context && "physics context not found");
    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);

    bullet_world.stepSimulation(instance.delta_time * speed, 1, time_step);

    if (!bullet_world.getDebugDrawer())
    {
      return;
    }

    auto& meshes = data<ludo::mesh>(instance, "ludo-bullet::visualizations");
    if (meshes.array_size == 0)
    {
      return;
    }

    // TODO if we implement 'refs' it could be used instead of this?
    dynamic_cast<debug_drawer*>(bullet_world.getDebugDrawer())->mesh = &meshes[0];
    bullet_world.debugDrawWorld();
  }

  template<>
  physics_context* add(instance& instance, const physics_context& init, const std::string& partition)
  {
    auto physics_context = add(data<ludo::physics_context>(instance), init, partition);
    physics_context->id = next_id++;

    auto config = new btDefaultCollisionConfiguration();
    // config->setConvexConvexMultipointIterations();

    // use the default collision dispatcher and constraint solver. For parallel processing see Extras/BulletMultiThreaded
    auto dispatcher = new btCollisionDispatcher(config);
    auto broadphase = new btDbvtBroadphase;
    auto bullet_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, nullptr, config);

    if (physics_context->visualize)
    {
      bullet_world->setDebugDrawer(new debug_drawer(btIDebugDraw::DBG_DrawWireframe));
    }

    physics_context->id = reinterpret_cast<uint64_t>(bullet_world);
    push(*physics_context);

    return physics_context;
  }

  template<>
  void remove<physics_context>(instance& instance, physics_context* element, const std::string& partition)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(element->id);

    if (bullet_world->getDebugDrawer())
    {
      delete bullet_world->getDebugDrawer();
    }
    delete bullet_world->getBroadphase();
    delete dynamic_cast<btCollisionDispatcher*>(bullet_world->getDispatcher())->getCollisionConfiguration();
    delete bullet_world->getDispatcher();
    delete bullet_world;

    remove(data<physics_context>(instance), element, partition);
  }

  void push(const physics_context& physics_context)
  {
    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);

    bullet_world.setGravity(to_btVector3(physics_context.gravity));
  }

  std::vector<contact> contacts(const instance& instance, const body& body_a)
  {
    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body_a.id);

    auto contact_result_callback = std::make_unique<ludo::contact_result_callback>();
    bullet_world.contactTest(&bullet_body, *contact_result_callback);

    return contact_result_callback->contacts;
  }

  std::vector<contact> contacts(const instance& instance, const body& body_a, const body& body_b)
  {
    auto body_a_contacts = ludo::contacts(instance, body_a);

    auto contacts = std::vector<contact>();
    for (auto& contact : contacts)
    {
      if (contact.body_b == &body_b)
      {
        contacts.emplace_back(contact);
      }
    }

    return contacts;
  }

  template<>
  void deallocate<static_body>(instance& instance)
  {
    auto& static_bodies = data<static_body>(instance);
    for (auto partition_iter = static_bodies.partitions.rbegin(); partition_iter != static_bodies.partitions.rend(); partition_iter++)
    {
      while (partition_iter->second.array_size)
      {
        // Call remove() to clean up Bullet resources
        auto static_body_iter = partition_iter->second.end() - 1;
        remove(instance, static_body_iter, partition_iter->first);
      }
    }

    deallocate(static_bodies);

    instance.data.erase(partitioned_buffer_key<static_body>());
    delete &static_bodies;
  }

  template<>
  static_body* add(instance& instance, const static_body& init, const std::string& partition)
  {
    auto& static_bodies = data<static_body>(instance);
    auto body = add(static_bodies, init, partition);

    // This addition shifts the elements after the one just added.
    // As a result any pointers to them are invalidated.
    update_pointers(body + 1, static_bodies.end());

    auto bullet_body = new btRigidBody(0.0f, nullptr, nullptr);

    body->id = reinterpret_cast<uint64_t>(bullet_body);
    bullet_body->setUserPointer(body);
    push(*body);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.addRigidBody(bullet_body);

    return body;
  }

  template<>
  void remove<static_body>(instance& instance, static_body* element, const std::string& partition)
  {
    auto& static_bodies = data<static_body>(instance, partition);

    auto bullet_body = reinterpret_cast<btRigidBody*>(element->id);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.removeRigidBody(bullet_body);

    if (bullet_body->getCollisionShape())
    {
      auto bullet_triangle_mesh_shape = dynamic_cast<btBvhTriangleMeshShape*>(bullet_body->getCollisionShape());
      delete bullet_triangle_mesh_shape->getMeshInterface();
      delete bullet_triangle_mesh_shape;
    }
    delete bullet_body;

    remove(data<static_body>(instance), element, partition);

    // This removal shifts the elements after the one just removed.
    // As a result any pointers to them are invalidated.
    update_pointers(element, static_bodies.end());
  }

  void push(const static_body& body)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.setWorldTransform(to_btTransform(body.transform));
  }

  task build_shape(instance& instance, static_body& body, const mesh& mesh, const vertex_format& format, uint32_t index_start, uint32_t index_count)
  {
    return [&instance, body, mesh, format, index_start, index_count]()
    {
      assert((index_start + index_count) * sizeof(uint32_t) <= mesh.index_buffer.size && "indexes out of range");

      auto bullet_mesh_interface = new btTriangleIndexVertexArray();

      auto bullet_mesh = btIndexedMesh();
      bullet_mesh.m_vertexBase = reinterpret_cast<u_char*>(mesh.vertex_buffer.data);
      bullet_mesh.m_vertexStride = format.size;
      bullet_mesh.m_numVertices = static_cast<int32_t>(index_count);
      bullet_mesh.m_triangleIndexBase = reinterpret_cast<u_char*>(mesh.index_buffer.data + index_start * sizeof(uint32_t));
      bullet_mesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
      bullet_mesh.m_numTriangles = static_cast<int32_t>(index_count / 3);
      bullet_mesh_interface->addIndexedMesh(bullet_mesh);

      auto bullet_shape = new btBvhTriangleMeshShape(bullet_mesh_interface, true);

      return [&instance, body, bullet_shape]()
      {
        auto physics_context = first<ludo::physics_context>(instance);
        assert(physics_context && "physics context not found");

        auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
        auto bullet_body = reinterpret_cast<btRigidBody*>(body.id);

        bullet_world.removeRigidBody(bullet_body);
        bullet_body->setCollisionShape(bullet_shape);
        bullet_world.addRigidBody(bullet_body);
      };
    };
  }

  template<>
  void deallocate<dynamic_body>(instance& instance)
  {
    auto& dynamic_bodies = data<dynamic_body>(instance);
    for (auto partition_iter = dynamic_bodies.partitions.rbegin(); partition_iter != dynamic_bodies.partitions.rend(); partition_iter++)
    {
      while (partition_iter->second.array_size)
      {
        // Call remove() to clean up Bullet resources
        auto dynamic_body_iter = partition_iter->second.end() - 1;
        remove(instance, dynamic_body_iter, partition_iter->first);
      }
    }

    deallocate(dynamic_bodies);

    instance.data.erase(partitioned_buffer_key<dynamic_body>());
    delete &dynamic_bodies;
  }

  template<>
  dynamic_body* add(instance& instance, const dynamic_body& init, const std::string& partition)
  {
    auto& dynamic_bodies = data<dynamic_body>(instance);
    auto body = add(dynamic_bodies, init, partition);

    // This addition shifts the elements after the one just added.
    // As a result any pointers to them are invalidated.
    update_pointers(body + 1, dynamic_bodies.end());

    auto bullet_body = build_bullet_body(instance, body, body->body_shape_ids, body->mass, false, false);
    push(*body);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.addRigidBody(bullet_body);

    return body;
  }

  template<>
  void remove<dynamic_body>(instance& instance, dynamic_body* element, const std::string& partition)
  {
    auto& dynamic_bodies = data<dynamic_body>(instance, partition);

    auto bullet_body = reinterpret_cast<btRigidBody*>(element->id);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.removeRigidBody(bullet_body);

    destroy_bullet_body(bullet_body);

    remove(data<dynamic_body>(instance), element, partition);

    // This removal shifts the elements after the one just removed.
    // As a result any pointers to them are invalidated.
    update_pointers(element, dynamic_bodies.end());
  }

  void push(const dynamic_body& body)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.setWorldTransform(to_btTransform(body.transform));

    if (bullet_body.getLocalInertia() == btVector3(0.0f, 0.0f, 0.0f) || (bullet_body.getCollisionShape() && bullet_body.getMass() != body.mass))
    {
      auto local_inertia = btVector3(0.0f, 0.0f, 0.0f);
      if (body.mass != 0.0f)
      {
        bullet_body.getCollisionShape()->calculateLocalInertia(body.mass, local_inertia);
      }

      bullet_body.setMassProps(body.mass, local_inertia);
    }

    bullet_body.setLinearVelocity(to_btVector3(body.linear_velocity));
    bullet_body.setAngularVelocity(to_btVector3(body.angular_velocity));
  }

  void pull(dynamic_body& body)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    body.linear_velocity = to_vec3(bullet_body.getLinearVelocity());
    body.angular_velocity = to_vec3(bullet_body.getAngularVelocity());
  }

  void apply_force(dynamic_body& body, const vec3& force, const vec3& position)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.applyForce(to_btVector3(force), to_btVector3(position));
  }

  void apply_impulse(dynamic_body& body, const vec3& impulse, const vec3& position)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.applyImpulse(to_btVector3(impulse), to_btVector3(position));
  }

  void apply_torque(dynamic_body& body, const vec3& torque)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.applyTorque(to_btVector3(torque));
  }

  void apply_torque_impulse(dynamic_body& body, const vec3& torque_impulse)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.applyTorqueImpulse(to_btVector3(torque_impulse));
  }

  template<>
  void deallocate<kinematic_body>(instance& instance)
  {
    auto& kinematic_bodies = data<kinematic_body>(instance);
    for (auto partition_iter = kinematic_bodies.partitions.rbegin(); partition_iter != kinematic_bodies.partitions.rend(); partition_iter++)
    {
      while (partition_iter->second.array_size)
      {
        // Call remove() to clean up Bullet resources
        auto kinematic_body_iter = partition_iter->second.end() - 1;
        remove(instance, kinematic_body_iter, partition_iter->first);
      }
    }

    deallocate(kinematic_bodies);

    instance.data.erase(partitioned_buffer_key<kinematic_body>());
    delete &kinematic_bodies;
  }

  template<>
  kinematic_body* add(instance& instance, const kinematic_body& init, const std::string& partition)
  {
    auto& kinematic_bodies = data<kinematic_body>(instance);
    auto body = add(kinematic_bodies, init, partition);

    // This addition shifts the elements after the one just added.
    // As a result any pointers to them are invalidated.
    update_pointers(body + 1, kinematic_bodies.end());

    auto bullet_body = build_bullet_body(instance, body, body->body_shape_ids, 0.0f, true, false);
    push(*body);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.addRigidBody(bullet_body);

    return body;
  }

  template<>
  void remove<kinematic_body>(instance& instance, kinematic_body* element, const std::string& partition)
  {
    auto& kinematic_bodies = data<kinematic_body>(instance, partition);

    auto bullet_body = reinterpret_cast<btRigidBody*>(element->id);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.removeRigidBody(bullet_body);

    destroy_bullet_body(bullet_body);

    remove(data<kinematic_body>(instance), element, partition);

    // This removal shifts the elements after the one just removed.
    // As a result any pointers to them are invalidated.
    update_pointers(element, kinematic_bodies.end());
  }

  void push(const kinematic_body& body)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.setWorldTransform(to_btTransform(body.transform));

    bullet_body.setLinearVelocity(to_btVector3(body.linear_velocity));
    bullet_body.setAngularVelocity(to_btVector3(body.angular_velocity));
  }

  template<>
  void deallocate<ghost_body>(instance& instance)
  {
    auto& ghost_bodies = data<ghost_body>(instance);
    for (auto partition_iter = ghost_bodies.partitions.rbegin(); partition_iter != ghost_bodies.partitions.rend(); partition_iter++)
    {
      while (partition_iter->second.array_size)
      {
        // Call remove() to clean up Bullet resources
        auto ghost_body_iter = partition_iter->second.end() - 1;
        remove(instance, ghost_body_iter, partition_iter->first);
      }
    }

    deallocate(ghost_bodies);

    instance.data.erase(partitioned_buffer_key<ghost_body>());
    delete &ghost_bodies;
  }

  template<>
  ghost_body* add(instance& instance, const ghost_body& init, const std::string& partition)
  {
    auto& ghost_bodies = data<ghost_body>(instance);
    auto body = add(ghost_bodies, init, partition);

    // This addition shifts the elements after the one just added.
    // As a result any pointers to them are invalidated.
    update_pointers(body + 1, ghost_bodies.end());

    auto bullet_body = build_bullet_body(instance, body, body->body_shape_ids, 0.0f, false, true);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.addRigidBody(bullet_body);

    return body;
  }

  template<>
  void remove<ghost_body>(instance& instance, ghost_body* element, const std::string& partition)
  {
    auto& ghost_bodies = data<ghost_body>(instance, partition);

    auto bullet_body = reinterpret_cast<btRigidBody*>(element->id);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.removeRigidBody(bullet_body);

    destroy_bullet_body(bullet_body);

    remove(data<ghost_body>(instance), element, partition);

    // This removal shifts the elements after the one just removed.
    // As a result any pointers to them are invalidated.
    update_pointers(element, ghost_bodies.end());
  }

  void push(const ghost_body& body)
  {
    auto& bullet_body = *reinterpret_cast<btRigidBody*>(body.id);

    bullet_body.setWorldTransform(to_btTransform(body.transform));
  }

  template<>
  void deallocate<constraint>(instance& instance)
  {
    auto& constraints = data<constraint>(instance);
    for (auto partition_iter = constraints.partitions.rbegin(); partition_iter != constraints.partitions.rend(); partition_iter++)
    {
      while (partition_iter->second.array_size)
      {
        // Call remove() to clean up Bullet resources
        auto constraint_iter = partition_iter->second.end() - 1;
        remove(instance, constraint_iter, partition_iter->first);
      }
    }

    deallocate(constraints);

    instance.data.erase(partitioned_buffer_key<constraint>());
    delete &constraints;
  }

  template<>
  constraint* add(instance& instance, const constraint& init, const std::string& partition)
  {
    auto constraint = add<ludo::constraint>(data<ludo::constraint>(instance), init, partition);

    auto bullet_constraint = static_cast<btGeneric6DofSpring2Constraint*>(nullptr);
    if (init.body_b)
    {
      auto& bullet_body_a = *reinterpret_cast<btRigidBody*>(init.body_a->id);
      auto& bullet_body_b = *reinterpret_cast<btRigidBody*>(init.body_b->id);
      bullet_constraint = new btGeneric6DofSpring2Constraint(bullet_body_a, bullet_body_b, to_btTransform(init.frame_a), to_btTransform(init.frame_b));
    }
    else
    {
      auto& bullet_body = *reinterpret_cast<btRigidBody*>(init.body_a->id);
      bullet_constraint = new btGeneric6DofSpring2Constraint(bullet_body, to_btTransform(init.frame_a));
    }

    constraint->id = reinterpret_cast<uint64_t>(bullet_constraint);
    push(*constraint);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context->id);
    bullet_world.addConstraint(bullet_constraint);

    return constraint;
  }

  template<>
  void remove<constraint>(instance& instance, constraint* element, const std::string& partition)
  {
    auto bullet_constraint = reinterpret_cast<btTypedConstraint*>(element->id);

    auto physics_context = first<ludo::physics_context>(instance);
    assert(physics_context && "physics context not found");

    auto& bullet_world = *reinterpret_cast<btDiscreteDynamicsWorld *>(physics_context->id);
    bullet_world.removeConstraint(bullet_constraint);

    delete bullet_constraint;

    remove(data<constraint>(instance), element, partition);
  }

  void push(const constraint& constraint)
  {
    auto bullet_constraint = reinterpret_cast<btGeneric6DofSpring2Constraint*>(constraint.id);

    bullet_constraint->setFrames(to_btTransform(constraint.frame_a), to_btTransform(constraint.frame_b));

    bullet_constraint->setLinearLowerLimit(to_btVector3(constraint.linear_lower_limit));
    bullet_constraint->setLinearUpperLimit(to_btVector3(constraint.linear_upper_limit));
    bullet_constraint->setAngularLowerLimit(to_btVector3(constraint.angular_lower_limit));
    bullet_constraint->setAngularUpperLimit(to_btVector3(constraint.angular_upper_limit));
  }

  btRigidBody* build_bullet_body(const instance& instance, body* body, const std::vector<uint64_t>& body_shape_ids, float mass, bool kinematic, bool ghost)
  {
    auto local_transform = btTransform();
    local_transform.setIdentity();

    auto bullet_compound_shape = new btCompoundShape(true, static_cast<int>(body_shape_ids.size()));
    for (auto& body_shape_id : body_shape_ids)
    {
      auto* body_shape = get<ludo::body_shape>(instance, body_shape_id);
      bullet_compound_shape->addChildShape(
        local_transform,
        new btConvexHullShape(body_shape->positions.data()->begin(), static_cast<int>(body_shape->positions.size()), sizeof(vec3))
      );
    }

    auto bullet_body = new btRigidBody(mass, new motion_state(body), bullet_compound_shape);

    if (kinematic || ghost)
    {
      bullet_body->setCollisionFlags(bullet_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
      bullet_body->setActivationState(DISABLE_DEACTIVATION);
    }

    if (ghost)
    {
      bullet_body->setCollisionFlags(bullet_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    body->id = reinterpret_cast<uint64_t>(bullet_body);
    bullet_body->setUserPointer(body);

    return bullet_body;
  }

  void destroy_bullet_body(btRigidBody* bullet_body)
  {
    auto bullet_compound_shape = dynamic_cast<btCompoundShape*>(bullet_body->getCollisionShape());

    for (auto shape_index = 0; shape_index < bullet_compound_shape->getNumChildShapes(); shape_index++)
    {
      delete bullet_compound_shape->getChildShape(shape_index);
    }
    delete bullet_compound_shape;
    delete bullet_body->getMotionState();
    delete bullet_body;
  }

  void update_pointers(static_body* begin, static_body* end)
  {
    for (auto current = begin; current < end; current++)
    {
      auto bullet_body = reinterpret_cast<btRigidBody*>(current->id);
      bullet_body->setUserPointer(current);
    }
  }

  void update_pointers(dynamic_body* begin, dynamic_body* end)
  {
    for (auto current = begin; current < end; current++)
    {
      auto bullet_body = reinterpret_cast<btRigidBody*>(current->id);
      bullet_body->setUserPointer(current);

      dynamic_cast<motion_state*>(bullet_body->getMotionState())->body = current;
    }
  }

  void update_pointers(kinematic_body* begin, kinematic_body* end)
  {
    for (auto current = begin; current < end; current++)
    {
      auto bullet_body = reinterpret_cast<btRigidBody*>(current->id);
      bullet_body->setUserPointer(current);
    }
  }

  void update_pointers(ghost_body* begin, ghost_body* end)
  {
    for (auto current = begin; current < end; current++)
    {
      auto bullet_ghost = reinterpret_cast<btCollisionObject*>(current->id);
      bullet_ghost->setUserPointer(current);
    }
  }
}
