/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <btBulletDynamicsCommon.h>

#include <ludo/physics.h>

#include "debug.h"
#include "math.h"

namespace ludo
{
  struct contact_result_callback : public btCollisionWorld::ContactResultCallback
  {
    std::vector<contact> contacts;

    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override
    {
      if (!colObj0Wrap->getCollisionObject() || !colObj1Wrap->getCollisionObject())
      {
        return 0;
      }

      contacts.emplace_back(contact
      {
        .body_a_id = reinterpret_cast<uint64_t>(colObj0Wrap->getCollisionObject()),
        .local_position_a = to_vec3(cp.m_localPointA),

        .body_b_id = reinterpret_cast<uint64_t>(colObj1Wrap->getCollisionObject()),
        .local_position_b = to_vec3(cp.m_localPointB),
        .world_position_b = to_vec3(cp.m_positionWorldOnB),
        .normal_b = to_vec3(cp.m_normalWorldOnB),

        .distance = cp.m_distance1
      });

      return 0;
    }
  };

  void init(physics_context& physics_context)
  {
    auto config = new btDefaultCollisionConfiguration();
    // config->setConvexConvexMultipointIterations();

    // We are just using the default collision dispatcher and constraint solver. For parallel processing see Extras/BulletMultiThreaded.
    auto dispatcher = new btCollisionDispatcher(config);
    auto broadphase = new btDbvtBroadphase();
    auto bullet_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, nullptr, config);

    physics_context.id = reinterpret_cast<uint64_t>(bullet_world);
    commit(physics_context);
  }

  void de_init(physics_context& physics_context)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    physics_context.id = 0;

    if (bullet_world->getDebugDrawer())
    {
      delete bullet_world->getDebugDrawer();
    }
    delete bullet_world->getBroadphase();
    delete dynamic_cast<btCollisionDispatcher*>(bullet_world->getDispatcher())->getCollisionConfiguration();
    delete bullet_world->getDispatcher();
    delete bullet_world;
  }

  void commit(const physics_context& physics_context)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);

    bullet_world->setGravity(to_btVector3(physics_context.gravity));
  }

  void simulate(physics_context& physics_context, float delta_time)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);

    bullet_world->stepSimulation(delta_time);
  }

  void visualize(const physics_context& physics_context, mesh& mesh)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);

    if (!bullet_world->getDebugDrawer())
    {
      bullet_world->setDebugDrawer(new debug_drawer());
    }

    std::memset(mesh.index_buffer.data, 0, mesh.index_buffer.size);
    std::memset(mesh.vertex_buffer.data, 0, mesh.vertex_buffer.size);

    dynamic_cast<debug_drawer*>(bullet_world->getDebugDrawer())->mesh = &mesh;
    bullet_world->debugDrawWorld();
  }

  std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id)
  {
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_body = reinterpret_cast<btRigidBody*>(body_a_id);

    auto contact_result_callback = ludo::contact_result_callback();
    bullet_world->contactTest(bullet_body, contact_result_callback);

    return contact_result_callback.contacts;
  }

  std::vector<contact> contacts(const physics_context& physics_context, uint64_t body_a_id, uint64_t body_b_id)
  {
    auto body_a_contacts = ludo::contacts(physics_context, body_a_id);

    auto contacts = std::vector<contact>();
    for (auto& contact : contacts)
    {
      if (contact.body_b_id == body_b_id)
      {
        contacts.emplace_back(contact);
      }
    }

    return contacts;
  }

  void init(static_body& static_body, physics_context& physics_context)
  {
    auto bullet_body = new btRigidBody(0.0f, nullptr, nullptr);
    static_body.id = reinterpret_cast<uint64_t>(bullet_body);
    commit(static_body);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->addRigidBody(bullet_body);
  }

  void de_init(static_body& static_body, physics_context& physics_context)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(static_body.id);
    static_body.id = 0;

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->removeRigidBody(bullet_body);

    if (bullet_body->getCollisionShape())
    {
      auto bullet_shape = dynamic_cast<btBvhTriangleMeshShape*>(bullet_body->getCollisionShape());
      delete bullet_shape->getMeshInterface();
      delete bullet_shape;
    }
    delete bullet_body;
  }

  void connect(static_body& static_body, physics_context& physics_context, const mesh& mesh, const vertex_format& format)
  {
    // TODO disconnect from previous

    auto bullet_mesh_interface = new btTriangleIndexVertexArray();

    auto bullet_mesh = btIndexedMesh();
    bullet_mesh.m_vertexBase = reinterpret_cast<const unsigned char*>(mesh.vertex_buffer.data);
    bullet_mesh.m_vertexStride = static_cast<int>(format.size);
    bullet_mesh.m_numVertices = static_cast<int>(mesh.index_buffer.size / sizeof(uint32_t));
    bullet_mesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(mesh.index_buffer.data);
    bullet_mesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
    bullet_mesh.m_numTriangles = static_cast<int>(mesh.index_buffer.size / (3 * sizeof(uint32_t)));
    bullet_mesh_interface->addIndexedMesh(bullet_mesh);

    auto bullet_shape = new btBvhTriangleMeshShape(bullet_mesh_interface, true);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_body = reinterpret_cast<btRigidBody*>(static_body.id);
    bullet_world->removeRigidBody(bullet_body);
    bullet_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(bullet_body);
  }

  void commit(const static_body& static_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(static_body.id);

    bullet_body->setWorldTransform(to_btTransform(static_body.transform));
  }

  void init(dynamic_body& dynamic_body, physics_context& physics_context)
  {
    auto bullet_body = new btRigidBody(dynamic_body.mass, new btDefaultMotionState(to_btTransform(dynamic_body.transform)), nullptr);
    dynamic_body.id = reinterpret_cast<uint64_t>(bullet_body);
    commit(dynamic_body);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->addRigidBody(bullet_body);
  }

  void de_init(dynamic_body& dynamic_body, physics_context& physics_context)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);
    dynamic_body.id = 0;

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->removeRigidBody(bullet_body);

    delete bullet_body->getMotionState();
    delete bullet_body;
  }

  void connect(dynamic_body& dynamic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(bullet_body);
    bullet_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(bullet_body);

    // Clear out the local inertia to trigger allow recalculation during commit.
    auto local_inertia = btVector3(0.0f, 0.0f, 0.0f);
    bullet_body->setMassProps(dynamic_body.mass, local_inertia);
    commit(dynamic_body);
  }

  void commit(const dynamic_body& dynamic_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->getMotionState()->setWorldTransform(to_btTransform(dynamic_body.transform));

    if (bullet_body->getCollisionShape() && (bullet_body->getLocalInertia() == btVector3(0.0f, 0.0f, 0.0f) || bullet_body->getMass() != dynamic_body.mass))
    {
      auto local_inertia = btVector3(0.0f, 0.0f, 0.0f);
      if (dynamic_body.mass != 0.0f)
      {
        bullet_body->getCollisionShape()->calculateLocalInertia(dynamic_body.mass, local_inertia);
      }

      bullet_body->setMassProps(dynamic_body.mass, local_inertia);
    }

    bullet_body->setLinearVelocity(to_btVector3(dynamic_body.linear_velocity));
    bullet_body->setAngularVelocity(to_btVector3(dynamic_body.angular_velocity));
  }

  void fetch(dynamic_body& dynamic_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    auto transform = btTransform();
    bullet_body->getMotionState()->getWorldTransform(transform);
    dynamic_body.transform = to_transform(transform);

    dynamic_body.linear_velocity = to_vec3(bullet_body->getLinearVelocity());
    dynamic_body.angular_velocity = to_vec3(bullet_body->getAngularVelocity());
  }

  void apply_force(dynamic_body& dynamic_body, const vec3& force, const vec3& position)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->applyForce(to_btVector3(force), to_btVector3(position));
  }

  void apply_impulse(dynamic_body& dynamic_body, const vec3& impulse, const vec3& position)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->applyImpulse(to_btVector3(impulse), to_btVector3(position));
  }

  void apply_torque(dynamic_body& dynamic_body, const vec3& torque)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->applyTorque(to_btVector3(torque));
  }

  void apply_torque_impulse(dynamic_body& dynamic_body, const vec3& torque_impulse)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->applyTorqueImpulse(to_btVector3(torque_impulse));
  }

  void clear_forces(dynamic_body& dynamic_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(dynamic_body.id);

    bullet_body->clearForces();
  }

  void init(kinematic_body& kinematic_body, physics_context& physics_context)
  {
    auto bullet_body = new btRigidBody(0.0f, nullptr, nullptr);
    bullet_body->setCollisionFlags(bullet_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    bullet_body->setActivationState(DISABLE_DEACTIVATION);
    kinematic_body.id = reinterpret_cast<uint64_t>(bullet_body);
    commit(kinematic_body);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->addRigidBody(bullet_body);
  }

  void de_init(kinematic_body& kinematic_body, physics_context& physics_context)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(kinematic_body.id);
    kinematic_body.id = 0;

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->removeRigidBody(bullet_body);

    delete bullet_body;
  }

  void connect(kinematic_body& kinematic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_body = reinterpret_cast<btRigidBody*>(kinematic_body.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(bullet_body);
    bullet_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(bullet_body);
  }

  void commit(const kinematic_body& kinematic_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(kinematic_body.id);

    bullet_body->setWorldTransform(to_btTransform(kinematic_body.transform));

    bullet_body->setLinearVelocity(to_btVector3(kinematic_body.linear_velocity));
    bullet_body->setAngularVelocity(to_btVector3(kinematic_body.angular_velocity));
  }

  void fetch(kinematic_body& kinematic_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(kinematic_body.id);

    auto transform = btTransform();
    bullet_body->getMotionState()->getWorldTransform(transform);
    kinematic_body.transform = to_transform(transform);

    kinematic_body.linear_velocity = to_vec3(bullet_body->getLinearVelocity());
    kinematic_body.angular_velocity = to_vec3(bullet_body->getAngularVelocity());
  }

  void init(ghost_body& ghost_body, physics_context& physics_context)
  {
    auto bullet_body = new btRigidBody(0.0f, nullptr, nullptr);
    bullet_body->setCollisionFlags(bullet_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    bullet_body->setActivationState(DISABLE_DEACTIVATION);
    bullet_body->setCollisionFlags(bullet_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    ghost_body.id = reinterpret_cast<uint64_t>(bullet_body);
    commit(ghost_body);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->addRigidBody(bullet_body);
  }

  void de_init(ghost_body& ghost_body, physics_context& physics_context)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(ghost_body.id);
    ghost_body.id = 0;

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->removeRigidBody(bullet_body);

    delete bullet_body;
  }

  void connect(ghost_body& ghost_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_body = reinterpret_cast<btRigidBody*>(ghost_body.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(bullet_body);
    bullet_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(bullet_body);
  }

  void commit(const ghost_body& ghost_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(ghost_body.id);

    bullet_body->setWorldTransform(to_btTransform(ghost_body.transform));
  }

  void fetch(ghost_body& ghost_body)
  {
    auto bullet_body = reinterpret_cast<btRigidBody*>(ghost_body.id);

    auto transform = btTransform();
    bullet_body->getMotionState()->getWorldTransform(transform);
    ghost_body.transform = to_transform(transform);
  }

  void init(dynamic_body_shape& dynamic_body_shape)
  {
    auto bullet_shape = new btCompoundShape(true, static_cast<int>(dynamic_body_shape.convex_hulls.size()));
    dynamic_body_shape.id = reinterpret_cast<uint64_t>(bullet_shape);

    auto transform = btTransform();
    transform.setIdentity();

    for (auto& convex_hull : dynamic_body_shape.convex_hulls)
    {
      bullet_shape->addChildShape(transform, new btConvexHullShape(convex_hull.data()->begin(), static_cast<int>(convex_hull.size()), sizeof(vec3)));
    }
  }

  void de_init(dynamic_body_shape& dynamic_body_shape)
  {
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);
    dynamic_body_shape.id = 0;

    for (auto index = 0; index < bullet_shape->getNumChildShapes(); index++)
    {
      delete bullet_shape->getChildShape(index);
    }
    delete bullet_shape;
  }

  void init(constraint& constraint, physics_context& physics_context)
  {
    auto bullet_constraint = static_cast<btGeneric6DofSpring2Constraint*>(nullptr);
    if (constraint.body_b)
    {
      auto& bullet_body_a = *reinterpret_cast<btRigidBody*>(constraint.body_a->id);
      auto& bullet_body_b = *reinterpret_cast<btRigidBody*>(constraint.body_b->id);
      bullet_constraint = new btGeneric6DofSpring2Constraint(bullet_body_a, bullet_body_b, to_btTransform(constraint.frame_a), to_btTransform(constraint.frame_b));
    }
    else
    {
      auto& bullet_body = *reinterpret_cast<btRigidBody*>(constraint.body_a->id);
      bullet_constraint = new btGeneric6DofSpring2Constraint(bullet_body, to_btTransform(constraint.frame_a));
    }

    constraint.id = reinterpret_cast<uint64_t>(bullet_constraint);
    commit(constraint);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->addConstraint(bullet_constraint);
  }

  void de_init(constraint& constraint, physics_context& physics_context)
  {
    auto bullet_constraint = reinterpret_cast<btTypedConstraint*>(constraint.id);

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld *>(physics_context.id);
    bullet_world->removeConstraint(bullet_constraint);

    delete bullet_constraint;
  }

  void commit(const constraint& constraint)
  {
    auto bullet_constraint = reinterpret_cast<btGeneric6DofSpring2Constraint*>(constraint.id);

    bullet_constraint->setFrames(to_btTransform(constraint.frame_a), to_btTransform(constraint.frame_b));

    bullet_constraint->setLinearLowerLimit(to_btVector3(constraint.linear_lower_limit));
    bullet_constraint->setLinearUpperLimit(to_btVector3(constraint.linear_upper_limit));
    bullet_constraint->setAngularLowerLimit(to_btVector3(constraint.angular_lower_limit));
    bullet_constraint->setAngularUpperLimit(to_btVector3(constraint.angular_upper_limit));
  }
}
