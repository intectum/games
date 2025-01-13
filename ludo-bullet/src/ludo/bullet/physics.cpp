/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <btBulletDynamicsCommon.h>

#include <ludo/physics.h>

#include "math.h"
#include "physics.h"

namespace ludo
{
  // TODO
  /*struct contact_result_callback : public btCollisionWorld::ContactResultCallback
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

    std::memset(mesh.indices.start, 0, mesh.indices.end - mesh.indices.start);
    std::memset(mesh.vertices.start, 0, mesh.vertices.end - mesh.vertices.start);

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

  void connect(static_body& static_body, physics_context& physics_context, const mesh& mesh, const vertex_format& format)
  {
    // TODO disconnect from previous

    auto bullet_mesh_interface = new btTriangleIndexVertexArray();

    auto bullet_mesh = btIndexedMesh();
    bullet_mesh.m_vertexBase = reinterpret_cast<const unsigned char*>(mesh.vertices.start);
    bullet_mesh.m_vertexStride = static_cast<int>(format.size);
    bullet_mesh.m_numVertices = static_cast<int>((mesh.indices.end - mesh.indices.start) / sizeof(uint32_t));
    bullet_mesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(mesh.indices.start);
    bullet_mesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
    bullet_mesh.m_numTriangles = static_cast<int>((mesh.indices.end - mesh.indices.start) / (3 * sizeof(uint32_t)));
    bullet_mesh_interface->addIndexedMesh(bullet_mesh);

    auto bullet_shape = new btBvhTriangleMeshShape(bullet_mesh_interface, true);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    bullet_world->removeRigidBody(static_body);
    static_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(static_body);
  }

  void connect(dynamic_body& dynamic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(dynamic_body);
    dynamic_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(dynamic_body);

    // Clear out the local inertia to trigger recalculation during commit.
    auto local_inertia = btVector3(0.0f, 0.0f, 0.0f);
    dynamic_body->setMassProps(dynamic_body.mass, local_inertia);
    commit(dynamic_body);
  }*/

  auto update_bullet_dynamic_bodies = job
  {
    .read_component_names = { "position", "rotation", "mass", "linear_velocity", "angular_velocity" },
    .write_component_names = { "dynamic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto positions = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto rotations = reinterpret_cast<const quat*>(read_component_data[1].data);
      auto masses = reinterpret_cast<const float*>(read_component_data[2].data);
      auto linear_velocities = reinterpret_cast<const vec3*>(read_component_data[3].data);
      auto angular_velocities = reinterpret_cast<const vec3*>(read_component_data[4].data);
      auto dynamic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto mass = masses[index];
        auto dynamic_body = dynamic_bodies[index];

        dynamic_body->getMotionState()->setWorldTransform(to_btTransform(positions[index], rotations[index]));

        if (dynamic_body->getCollisionShape() && (dynamic_body->getLocalInertia() == btVector3(0.0f, 0.0f, 0.0f) || dynamic_body->getMass() != mass))
        {
          auto local_inertia = btVector3(0.0f, 0.0f, 0.0f);
          if (mass != 0.0f)
          {
            dynamic_body->getCollisionShape()->calculateLocalInertia(mass, local_inertia);
          }

          dynamic_body->setMassProps(mass, local_inertia);
        }

        dynamic_body->setLinearVelocity(to_btVector3(linear_velocities[index]));
        dynamic_body->setAngularVelocity(to_btVector3(angular_velocities[index]));
      }
    }
  };

  auto apply_bullet_forces = job
  {
    .read_component_names = { "force", "force_position" },
    .write_component_names = { "dynamic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto forces = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto force_positions = reinterpret_cast<const vec3*>(read_component_data[1].data);
      auto dynamic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto dynamic_body = dynamic_bodies[index];

        dynamic_body->applyForce(to_btVector3(forces[index]), to_btVector3(force_positions[index]));
      }
    }
  };

  auto apply_bullet_impulses = job
  {
    .read_component_names = { "impulse", "impulse_position" },
    .write_component_names = { "dynamic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto impulses = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto impulse_positions = reinterpret_cast<const vec3*>(read_component_data[1].data);
      auto dynamic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto dynamic_body = dynamic_bodies[index];

        dynamic_body->applyImpulse(to_btVector3(impulses[index]), to_btVector3(impulse_positions[index]));
      }
    }
  };

  auto apply_bullet_torques = job
  {
    .read_component_names = { "torque" },
    .write_component_names = { "dynamic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto torques = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto dynamic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto dynamic_body = dynamic_bodies[index];

        dynamic_body->applyTorque(to_btVector3(torques[index]));
      }
    }
  };

  auto apply_bullet_torque_impulses = job
  {
    .read_component_names = { "torque_impulse" },
    .write_component_names = { "dynamic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto torque_impulses = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto dynamic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto dynamic_body = dynamic_bodies[index];

        dynamic_body->applyTorqueImpulse(to_btVector3(torque_impulses[index]));
      }
    }
  };

  auto update_from_bullet_dynamic_bodies = job
  {
    .read_component_names = { "dynamic_body" },
    .write_component_names = { "position", "rotation", "linear_velocity", "angular_velocity" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto dynamic_bodies = reinterpret_cast<const btRigidBody**>(read_component_data[0].data);
      auto positions = reinterpret_cast<vec3*>(write_component_data[0].data);
      auto rotations = reinterpret_cast<quat*>(write_component_data[1].data);
      auto linear_velocities = reinterpret_cast<vec3*>(write_component_data[1].data);
      auto angular_velocities = reinterpret_cast<vec3*>(write_component_data[1].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto dynamic_body = dynamic_bodies[index];

        auto bullet_transform = btTransform();
        dynamic_body->getMotionState()->getWorldTransform(bullet_transform);

        auto transform = to_mat4(bullet_transform);
        positions[index] = position(transform);
        rotations[index] = quat(mat3(transform));

        linear_velocities[index] = to_vec3(dynamic_body->getLinearVelocity());
        angular_velocities[index] = to_vec3(dynamic_body->getAngularVelocity());
      }
    }
  };

  // TODO
  /*void connect(kinematic_body& kinematic_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(kinematic_body);
    kinematic_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(kinematic_body);
  }*/

  job update_bullet_kinematic_bodies =
  {
    .read_component_names = { "position", "rotation", "linear_velocity", "angular_velocity" },
    .write_component_names = { "kinematic_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto positions = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto rotations = reinterpret_cast<const quat*>(read_component_data[1].data);
      auto linear_velocities = reinterpret_cast<const vec3*>(read_component_data[2].data);
      auto angular_velocities = reinterpret_cast<const vec3*>(read_component_data[3].data);
      auto kinematic_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto kinematic_body = kinematic_bodies[index];

        kinematic_body->setWorldTransform(to_btTransform(positions[index], rotations[index]));

        kinematic_body->setLinearVelocity(to_btVector3(linear_velocities[index]));
        kinematic_body->setAngularVelocity(to_btVector3(angular_velocities[index]));
      }
    }
  };

  // TODO
  /*void connect(ghost_body& ghost_body, physics_context& physics_context, const dynamic_body_shape& dynamic_body_shape)
  {
    // TODO disconnect from previous

    auto bullet_world = reinterpret_cast<btDiscreteDynamicsWorld*>(physics_context.id);
    auto bullet_shape = reinterpret_cast<btCompoundShape*>(dynamic_body_shape.id);

    // It seems that bullet physics only registers a change to the collision shape when the rigid body is added to the world.
    bullet_world->removeRigidBody(ghost_body);
    ghost_body->setCollisionShape(bullet_shape);
    bullet_world->addRigidBody(ghost_body);
  }*/

  job update_bullet_ghost_bodies =
  {
    .read_component_names = { "position", "rotation" },
    .write_component_names = { "ghost_body" },
    .kernel = [](uint32_t entity_start, uint32_t entity_count, const std::vector<arena>& read_component_data, std::vector<arena>& write_component_data)
    {
      auto positions = reinterpret_cast<const vec3*>(read_component_data[0].data);
      auto rotations = reinterpret_cast<const quat*>(read_component_data[1].data);
      auto ghost_bodies = reinterpret_cast<btRigidBody**>(write_component_data[0].data);

      for (auto index = 0; index < entity_count; index++)
      {
        auto ghost_body = ghost_bodies[index];

        ghost_body->setWorldTransform(to_btTransform(positions[index], rotations[index]));
      }
    }
  };
}
