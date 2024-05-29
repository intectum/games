/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#ifndef LUDO_BULLET_DEBUG_H
#define LUDO_BULLET_DEBUG_H

#include <btBulletDynamicsCommon.h>

#include <ludo/meshes.h>

namespace ludo
{
  struct debug_drawer : public btIDebugDraw
  {
    debug_drawer();

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

    void clearLines() override;

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

    void draw3dText(const btVector3& location, const char* text_string) override;

    void reportErrorWarning(const char* warning_string) override;

    [[nodiscard]] int getDebugMode() const override;

    void setDebugMode(int debug_mode) override;

    ludo::mesh* mesh;
    int32_t debug_mode;
    uint32_t next_index;
  };
}

#endif // LUDO_BULLET_DEBUG_H
