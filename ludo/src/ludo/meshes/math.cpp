/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include "../math/util.h"
#include "edit.h"
#include "math.h"

namespace ludo
{
  /*struct mesh_intersection
  {
    std::tuple<Vector3, Vector3> intersection;
    Vector3 lhs_normal;
    std::tuple<Vector3, Vector3, Vector3> lhs_triangle;
    uint32_t lhs_triangle_index;
    Vector3 rhs_normal;
    std::tuple<Vector3, Vector3, Vector3> rhs_triangle;
    uint32_t rhs_triangle_index;

    bool is_point_intersection()
    {
      return near(std::get<0>(intersection), std::get<1>(intersection));
    }
  };







  void remove_intersection(const mesh& lhs,
                           const mesh& rhs,
                           std::vector<Vertex>& new_vertices,
                           const std::tuple<Vector3, Vector3, Vector3>& lhsTriangle,
                           uint32_t lhsTriangleIndex,
                           uint32_t nextLhsTriangleIndex,
                           const std::vector<mesh_intersection>* intersections,
                           bool* currentIntersectionsAtEdge,
                           const Matrix44& inverseRelativeTransform)
  {
    auto template_vertex = lhs.vertex_data[lhsTriangleIndex * 3];

    if (!intersections[0].empty())
    {
      auto lhsEdge0(lhsTriangle.getPointA(), lhsTriangle.getPointB());
      remove_intersection(new_vertices, new_indices, intersections[0], lhsEdge0, templateVertex,
                         currentIntersectionsAtEdge[0]);
    }
    if (!intersections[1].empty())
    {
      auto lhsEdge1(lhsTriangle.getPointB(), lhsTriangle.getPointC());
      remove_intersection(new_vertices, new_indices, intersections[1], lhsEdge1, templateVertex,
                         currentIntersectionsAtEdge[1]);
    }
    if (!intersections[2].empty())
    {
      auto lhsEdge2(lhsTriangle.getPointC(), lhsTriangle.getPointA());
      remove_intersection(new_vertices, new_indices, intersections[2], lhsEdge2, templateVertex,
                         currentIntersectionsAtEdge[2]);
    }

    *//*
     * If the current lhs triangle was intersected, move to the next lhs triangle. This implicitly
     * handles the case where the first lhs triangle was not intersected (by not moving to the next lhs
     * triangle).
     *//*
    if (!intersections[0].empty() || !intersections[1].empty() || !intersections[2].empty())
    {
      lhsTriangleIndex++;
    }

    // Preserve any lhs triangles NOT within the rhs mesh that did not have any intersections.
    while(lhsTriangleIndex < nextLhsTriangleIndex)
    {
      uint32_t vertexIndex = lhsTriangleIndex * 3;

      Triangle lhsTriangle(lhs[vertexIndex].position, lhs[vertexIndex + 1].position,
                           lhs[vertexIndex + 2].position);

      if (Intersection::contains(rhs, lhsTriangle, inverseRelativeTransform))
      {
        lhsTriangleIndex++;
        continue;
      }

      new_vertices.push_back(lhs[vertexIndex]);
      new_vertices.push_back(lhs[vertexIndex + 1]);
      new_vertices.push_back(lhs[vertexIndex + 2]);

      new_indices.push_back(new_vertices.size() - 3);
      new_indices.push_back(new_vertices.size() - 2);
      new_indices.push_back(new_vertices.size() - 1);

      lhsTriangleIndex++;
    }
  }

  void remove_intersection(mesh& mesh,
                           const std::vector<mesh_intersection>& intersections,
                           const std::tuple<Vector3, Vector3>& lhs_edge,
                           const Vector3& template_vertex,
                           bool intersection_at_edge)
  {
    auto sorted_edge_points = std::unordered_map<float, Vector3>();
    getSortedEdgePoints(intersections, lhs_edge, sorted_edge_points);
    auto sorted_intersection_points = std::unordered_map<float, Vector3>();
    getSortedIntersectionPoints(intersections, lhs_edge, sorted_intersection_points);

    // Connect the dots!
    auto gap = false;
    auto intersection_point_iter = sorted_intersection_points.begin();
    auto edge_point_iter = sorted_edge_points.begin();
    while (intersection_point_iter != sorted_intersection_points.end())
    {
      if (gap)
      {
        gap = false;
      }
      else
      {
        // Create a triangle between two edge points and an intersection point.
        Vertex vertexAA = templateVertex;
        vertexAA.position = intersection_point_iter->second;
        vertices.push_back(vertexAA);

        Vertex vertexAB = templateVertex;
        vertexAB.position = edge_point_iter->second;
        vertices.push_back(vertexAB);

        edge_point_iter++;
        if (edge_point_iter == sorted_edge_points.end())
        {
          // Oops... we've reached the end of the edge points which means this triangle isn't needed.
          // This happens if a gap was created in the lhs edge by the intersections.
          vertices.pop_back();
          vertices.pop_back();
        }
        else
        {
          Vertex vertexAC = templateVertex;
          vertexAC.position = edge_point_iter->second;
          vertices.push_back(vertexAC);
        }
      }

      Vertex templateVertex2 = templateVertex;
      templateVertex2.color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

      // Create a triangle between two intersection points and an edge point.
      Vertex vertexBA = templateVertex2;
      vertexBA.position = intersection_point_iter->second;
      vertices.push_back(vertexBA);

      intersection_point_iter++;
      if (intersection_point_iter == sorted_intersection_points.end())
      {
        // Oops... we've reached the end of the intersection points which means this triangle isn't needed.
        vertices.pop_back();
      }
      else
      {
        Vertex vertexBB = templateVertex2;
        vertexBB.position = edge_point_iter->second;
        vertices.push_back(vertexBB);

        Vertex vertexBC = templateVertex2;
        vertexBC.position = intersection_point_iter->second;
        vertices.push_back(vertexBC);
      }

      if (intersection_point_iter != sorted_intersection_points.end())
      {
        indices.push_back(vertices.size() - 6);
        indices.push_back(vertices.size() - 5);
        indices.push_back(vertices.size() - 4);

        // If an intersection point is on the line then a gap in the edge has been created!
        if (near(Distance::distanceBetween(lhsEdge, Point(intersection_point_iter->second)),  0.0f))
        {
          edge_point_iter++;

          *//*
           * Although... just maybe... the intersection ended exactly on the edge in which case there is
           * no gap... If that's true we should stick with the same intersection point.
           *//*
          if (!intersectionAtEdge)
          {
            intersection_point_iter++;
          }

          edge_point_iter++;
          gap = true;
        }
      }

      indices.push_back(vertices.size() - 3);
      indices.push_back(vertices.size() - 2);
      indices.push_back(vertices.size() - 1);
    }
  }

  void retain_intersection(const mesh& lhs,
                           const mesh& rhs,
                           std::vector<Vertex>& new_vertices,
                           std::vector<uint32_t>& new_indices,
                           const Matrix44& relative_transform)
  {
    auto inverse_relative_transform = relative_transform;
    inverse_relative_transform.invert();

    auto intersections = get_intersections(lhs, rhs, relative_transform);
    Triangle lhsTriangle = intersections[0].lhsTriangle;
    uint32_t lhsTriangleIndex = 0;

    // Populate the lhs triangle data.
    Vector3 lhsEdge0Vector = lhsTriangle.getPointB() - lhsTriangle.getPointA();
    Vector3 lhsEdge1Vector = lhsTriangle.getPointC() - lhsTriangle.getPointB();
    Vector3 lhsEdge2Vector = lhsTriangle.getPointA() - lhsTriangle.getPointC();
    *//*
     * These 'out' vectors are vectors on the lhs triangle's plane perpendicular to the edges of the triangle
     * and in the direction that is 'outward' from the triangle.
     *//*
    Vector3 lhsEdge0Out = crossProduct(lhsEdge0Vector, intersections[0].lhsNormal);
    lhsEdge0Out.normalize();
    Vector3 lhsEdge1Out = crossProduct(lhsEdge1Vector, intersections[0].lhsNormal);
    lhsEdge1Out.normalize();
    Vector3 lhsEdge2Out = crossProduct(lhsEdge2Vector, intersections[0].lhsNormal);
    lhsEdge2Out.normalize();

    // The intersections with the current lhs triangle, separated according to which edge they are most aligned.
    vector<MeshIntersection> currentIntersections[3];

    for (MeshIntersection& intersection : intersections)
    {
      // Disregard point intersections.
      if (intersection.isPointIntersection())
      {
        continue;
      }

      // If we've finished processing an lhs triangle.
      if (intersection.lhsTriangleIndex != lhsTriangleIndex)
      {
        retain_intersection(lhs, rhs, new_vertices, new_indices, lhsTriangle, lhsTriangleIndex,
                           intersection.lhsTriangleIndex, currentIntersections, inverseRelativeTransform);

        lhsTriangle = intersection.lhsTriangle;
        lhsTriangleIndex = intersection.lhsTriangleIndex;

        // Re-populate the lhs triangle data.
        Vector3 lhsEdge0Vector = lhsTriangle.getPointB() - lhsTriangle.getPointA();
        Vector3 lhsEdge1Vector = lhsTriangle.getPointC() - lhsTriangle.getPointB();
        Vector3 lhsEdge2Vector = lhsTriangle.getPointA() - lhsTriangle.getPointC();
        lhsEdge0Out = crossProduct(lhsEdge0Vector, intersection.lhsNormal);
        lhsEdge0Out.normalize();
        lhsEdge1Out = crossProduct(lhsEdge1Vector, intersection.lhsNormal);
        lhsEdge1Out.normalize();
        lhsEdge2Out = crossProduct(lhsEdge2Vector, intersection.lhsNormal);
        lhsEdge2Out.normalize();

        // Reset the intersection data.
        currentIntersections[0].clear();
        currentIntersections[1].clear();
        currentIntersections[2].clear();
      }

      float dotEdge0Out = dotProduct(intersection.rhsNormal, lhsEdge0Out);
      float dotEdge1Out = dotProduct(intersection.rhsNormal, lhsEdge1Out);
      float dotEdge2Out = dotProduct(intersection.rhsNormal, lhsEdge2Out);

      *//*
       * Add the intersection to the vector for the edge of the lhs triangle to which it is most
       * closely aligned. Since we're talking about convex shapes the intersection should be on that
       * edge's side of the rhs mesh.
       *//*
      if (dotEdge0Out > dotEdge1Out && dotEdge0Out > dotEdge2Out)
      {
        currentIntersections[0].push_back(intersection);
      }
      else if (dotEdge1Out > dotEdge0Out && dotEdge1Out > dotEdge2Out)
      {
        currentIntersections[1].push_back(intersection);
      }
      else
      {
        currentIntersections[2].push_back(intersection);
      }
    }

    retain_intersection(lhs, rhs, new_vertices, new_indices, lhsTriangle, lhsTriangleIndex,
                       lhs.indexCount / 3, currentIntersections, inverseRelativeTransform);
  }

  void retain_intersection(const mesh<vertex_type, index_type>& lhs, const mesh<vertex_type, index_type>& rhs, vector<Vertex>& new_vertices,
                          vector<uint32_t>& new_indices, const Triangle& lhsTriangle, uint32_t lhsTriangleIndex,
                          uint32_t nextLhsTriangleIndex, const vector<MeshIntersection>* intersections,
                          const Matrix44& inverseRelativeTransform)
  {
    vector<Vector3> sortedIntersectionPoints;
    Line lhsEdge0(lhsTriangle.getPointA(), lhsTriangle.getPointB());
    Line lhsEdge1(lhsTriangle.getPointB(), lhsTriangle.getPointC());
    Line lhsEdge2(lhsTriangle.getPointC(), lhsTriangle.getPointA());

    if (!intersections[0].empty())
    {
      map<float, Vector3> sortedEdgeIntersectionPoints;
      getSortedIntersectionPoints(intersections[0], lhsEdge0, sortedEdgeIntersectionPoints);

      for (map<float, Vector3>::iterator sortedPointIter = sortedEdgeIntersectionPoints.begin();
           sortedPointIter != sortedEdgeIntersectionPoints.end(); sortedPointIter++)
      {
        sortedIntersectionPoints.push_back(sortedPointIter->second);
      }
    }

    if (!intersections[1].empty())
    {
      map<float, Vector3> sortedEdgeIntersectionPoints;
      getSortedIntersectionPoints(intersections[1], lhsEdge1, sortedEdgeIntersectionPoints);

      for (map<float, Vector3>::iterator sortedPointIter = sortedEdgeIntersectionPoints.begin();
           sortedPointIter != sortedEdgeIntersectionPoints.end(); sortedPointIter++)
      {
        sortedIntersectionPoints.push_back(sortedPointIter->second);
      }
    }

    if (!intersections[2].empty())
    {
      map<float, Vector3> sortedEdgeIntersectionPoints;
      getSortedIntersectionPoints(intersections[2], lhsEdge2, sortedEdgeIntersectionPoints);

      for (map<float, Vector3>::iterator sortedPointIter = sortedEdgeIntersectionPoints.begin();
           sortedPointIter != sortedEdgeIntersectionPoints.end(); sortedPointIter++)
      {
        sortedIntersectionPoints.push_back(sortedPointIter->second);
      }
    }

    Vector3 center(0.0f, 0.0f, 0.0f);
    for (Vector3& intersectionPoint : sortedIntersectionPoints)
    {
      center += intersectionPoint;
    }
    center /= static_cast<float>(sortedIntersectionPoints.size());

    Vertex templateVertex = lhs[lhsTriangleIndex * 3];
    for (uint32_t index = 0; index < sortedIntersectionPoints.size(); index++)
    {
      Vertex vertexA = templateVertex;
      vertexA.position = sortedIntersectionPoints[index];
      new_vertices.push_back(vertexA);

      Vertex vertexB = templateVertex;
      if (index < sortedIntersectionPoints.size() - 1)
      {
        vertexB.position = sortedIntersectionPoints[index + 1];
      }
      else
      {
        vertexB.position = sortedIntersectionPoints[0];
      }
      new_vertices.push_back(vertexB);

      Vertex vertexC = templateVertex;
      vertexC.position = center;
      new_vertices.push_back(vertexC);

      new_indices.push_back(new_vertices.size() - 3);
      new_indices.push_back(new_vertices.size() - 2);
      new_indices.push_back(new_vertices.size() - 1);
    }

    *//*
     * If the current lhs triangle was intersected, move to the next lhs triangle. This implicitly
     * handles the case where the first lhs triangle was not intersected (by not moving to the next lhs
     * triangle).
     *//*
    if (!intersections[0].empty() || !intersections[1].empty() || !intersections[2].empty())
    {
      lhsTriangleIndex++;
    }

    // Preserve any lhs triangles within the rhs mesh that did not have any intersections.
    while(lhsTriangleIndex < nextLhsTriangleIndex)
    {
      uint32_t vertexIndex = lhsTriangleIndex * 3;

      Triangle lhsTriangle(lhs[vertexIndex].position, lhs[vertexIndex + 1].position,
                           lhs[vertexIndex + 2].position);

      if (!Intersection::contains(rhs, lhsTriangle, inverseRelativeTransform))
      {
        lhsTriangleIndex++;
        continue;
      }

      new_vertices.push_back(lhs[vertexIndex]);
      new_vertices.push_back(lhs[vertexIndex + 1]);
      new_vertices.push_back(lhs[vertexIndex + 2]);

      new_indices.push_back(new_vertices.size() - 3);
      new_indices.push_back(new_vertices.size() - 2);
      new_indices.push_back(new_vertices.size() - 1);

      lhsTriangleIndex++;
    }
  }







  std::vector<mesh_intersection> get_intersections(const vertex_format& format,
                                                   const mesh& lhs,
                                                   const mesh& rhs,
                                                   const Matrix44& relative_transform)
  {
    auto intersections = std::vector<mesh_intersection>();

    auto position_offset = get_mesh_offset(format, 'p');
    auto inverse_relative_transform = relative_transform;
    inverse_relative_transform.invert();

    auto lhs_float_index = position_offset;
    for (auto lhs_index = 0; lhs_index < lhs.vertex_count; lhs_index += 3)
    {
      auto lhs_point0 = *reinterpret_cast<Vector3*>(&lhs.vertex_data[lhs_float_index]);
      lhs_float_index += format.size;
      auto lhs_point1 = *reinterpret_cast<Vector3*>(&lhs.vertex_data[lhs_float_index]);
      lhs_float_index += format.size;
      auto lhs_point2 = *reinterpret_cast<Vector3*>(&lhs.vertex_data[lhs_float_index]);
      lhs_float_index += format.size;

      auto lhs_triangle = std::tuple<Vector3, Vector3, Vector3>(lhs_point0, lhs_point1, lhs_point2);

      auto lhs_edge0 = lhs_point1 - lhs_point0;
      auto lhs_edge1 = lhs_point2 - lhs_point0;
      auto lhs_normal = crossProduct(lhs_edge0, lhs_edge1);
      lhs_normal.normalize();

      auto rhs_float_index = position_offset;
      for (auto rhs_index = 0; rhs_index < rhs.vertex_count; rhs_index += 3)
      {
        auto rhs_point0 = *reinterpret_cast<Vector3*>(&rhs.vertex_data[rhs_float_index]);
        rhs_float_index += format.size;
        auto rhs_point1 = *reinterpret_cast<Vector3*>(&rhs.vertex_data[rhs_float_index]);
        rhs_float_index += format.size;
        auto rhs_point2 = *reinterpret_cast<Vector3*>(&rhs.vertex_data[rhs_float_index]);
        rhs_float_index += format.size;

        rhs_point0 = Vector3((relative_transform * Vector4(rhs_point0, 1.0f)).getData());
        rhs_point1 = Vector3((relative_transform * Vector4(rhs_point1, 1.0f)).getData());
        rhs_point2 = Vector3((relative_transform * Vector4(rhs_point2, 1.0f)).getData());

        auto rhs_triangle = std::tuple<Vector3, Vector3, Vector3>(rhs_point0, rhs_point1, rhs_point2);

        auto rhs_edge0 = rhs_point1 - rhs_point0;
        auto rhs_edge1 = rhs_point2 - rhs_point0;
        auto rhs_normal = crossProduct(rhs_edge0, rhs_edge1);
        rhs_normal.normalize();

        if (Intersection::intersect(lhs_triangle, rhs_triangle, lhs_normal, rhs_normal))
        {
          intersections.push_back(MeshIntersection(
            Intersection::getIntersection(lhs_triangle, rhs_triangle, lhs_normal, rhs_normal),
            lhs_normal,
            lhs_triangle,
            lhs_index / 3,
            rhs_normal,
            rhs_triangle,
            rhs_index / 3));
        }
      }
    }

    return intersections;
  }

  void remove_intersection(const vertex_format& format,
                           const mesh& lhs,
                           const mesh& rhs,
                           const mesh& difference,
                           const Matrix44& relative_transform)
  {
    auto inverse_relative_transform = relative_transform;
    inverse_relative_transform.invert();

    auto intersections = get_intersections(lhs, rhs, relative_transform);
    auto lhsTriangle = intersections[0].lhsTriangle;
    auto lhsTriangleIndex = 0;

    // Populate the lhs triangle data.
    auto lhsEdge0Vector = lhsTriangle.getPointB() - lhsTriangle.getPointA();
    auto lhsEdge1Vector = lhsTriangle.getPointC() - lhsTriangle.getPointB();
    auto lhsEdge2Vector = lhsTriangle.getPointA() - lhsTriangle.getPointC();
    *//*
     * These 'out' vectors are vectors on the lhs triangle's plane perpendicular to the edges of the triangle
     * and in the direction that is 'outward' from the triangle.
     *//*
    auto lhsEdge0Out = crossProduct(lhsEdge0Vector, intersections[0].lhsNormal);
    lhsEdge0Out.normalize();
    auto lhsEdge1Out = crossProduct(lhsEdge1Vector, intersections[0].lhsNormal);
    lhsEdge1Out.normalize();
    auto lhsEdge2Out = crossProduct(lhsEdge2Vector, intersections[0].lhsNormal);
    lhsEdge2Out.normalize();

    // The intersections with the current lhs triangle, separated according to which edge they are most aligned.
    auto currentIntersections = vector<mesh_intersection>[3]();

    // Determines whether the intersections with the current lhs triangle end on the edge of the triangle.
    bool currentIntersectionsAtEdge[3] = {false, false, false};

    for (mesh_intersection& intersection : intersections)
    {
      // Disregard point intersections.
      if (intersection.isPointIntersection())
      {
        continue;
      }

      // If we've finished processing an lhs triangle.
      if (intersection.lhsTriangleIndex != lhsTriangleIndex)
      {
        remove_intersection(lhs, rhs, new_vertices, new_indices, lhsTriangle, lhsTriangleIndex,
                            intersection.lhsTriangleIndex, currentIntersections, currentIntersectionsAtEdge,
                            inverse_relative_transform);

        lhsTriangle = intersection.lhsTriangle;
        lhsTriangleIndex = intersection.lhsTriangleIndex;

        // Re-populate the lhs triangle data.
        Vector3 lhsEdge0Vector = lhsTriangle.getPointB() - lhsTriangle.getPointA();
        Vector3 lhsEdge1Vector = lhsTriangle.getPointC() - lhsTriangle.getPointB();
        Vector3 lhsEdge2Vector = lhsTriangle.getPointA() - lhsTriangle.getPointC();
        lhsEdge0Out = crossProduct(lhsEdge0Vector, intersection.lhsNormal);
        lhsEdge0Out.normalize();
        lhsEdge1Out = crossProduct(lhsEdge1Vector, intersection.lhsNormal);
        lhsEdge1Out.normalize();
        lhsEdge2Out = crossProduct(lhsEdge2Vector, intersection.lhsNormal);
        lhsEdge2Out.normalize();

        // Reset the intersection data.
        currentIntersections[0].clear();
        currentIntersections[1].clear();
        currentIntersections[2].clear();
        currentIntersectionsAtEdge[0] = false;
        currentIntersectionsAtEdge[1] = false;
        currentIntersectionsAtEdge[2] = false;
      }

      float dotEdge0Out = dotProduct(intersection.rhsNormal, lhsEdge0Out);
      float dotEdge1Out = dotProduct(intersection.rhsNormal, lhsEdge1Out);
      float dotEdge2Out = dotProduct(intersection.rhsNormal, lhsEdge2Out);

      *//*
       * Add the intersection to the vector for the edge of the lhs triangle to which it is most
       * closely aligned. Since we're talking about convex shapes the intersection should be on that
       * edge's side of the rhs mesh.
       *//*
      if (dotEdge0Out > dotEdge1Out && dotEdge0Out > dotEdge2Out)
      {
        currentIntersections[0].push_back(intersection);
        Line edge(intersection.lhsTriangle.getPointA(), intersection.lhsTriangle.getPointB());
        if (near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointA()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointB()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointC()), 0.0f))
        {
          currentIntersectionsAtEdge[0] = true;
        }
      }
      else if (dotEdge1Out > dotEdge0Out && dotEdge1Out > dotEdge2Out)
      {
        currentIntersections[1].push_back(intersection);
        Line edge(intersection.lhsTriangle.getPointB(), intersection.lhsTriangle.getPointC());
        if (near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointA()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointB()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointC()), 0.0f))
        {
          currentIntersectionsAtEdge[1] = true;
        }
      }
      else
      {
        currentIntersections[2].push_back(intersection);
        Line edge(intersection.lhsTriangle.getPointC(), intersection.lhsTriangle.getPointA());
        if (near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointA()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointB()), 0.0f) ||
            near(Distance::distanceBetween(edge, intersection.rhsTriangle.getPointC()), 0.0f))
        {
          currentIntersectionsAtEdge[2] = true;
        }
      }
    }

    remove_intersection(lhs, rhs, new_vertices, new_indices, lhsTriangle, lhsTriangleIndex,
                       lhs.indexCount / 3, currentIntersections, currentIntersectionsAtEdge,
                       inverseRelativeTransform);
  }

  mesh* subtract(context& context,
                 const vertex_format& format,
                 const mesh& lhs,
                 const mesh& rhs,
                 const Matrix44& relative_transform)
  {
    auto inverse_relative_transform = relative_transform;
    inverse_relative_transform.invert();

    *//*auto new_vertices = std::vector<vertex_type>();
    auto new_indices = std::vector<index_type>();

    // Guesstimate to avoid too much allocations
    // TODO Why bother with this? Just go direct to the buffer!!!
    new_vertices.reserve(lhs.vertex_buffer_count + rhs.vertex_buffer_count);
    new_indices.reserve(lhs.index_buffer_count + rhs.index_buffer_count);*//*

    auto difference = add(context, ludo::mesh {}, format, vertex_count, 0, partition);

    remove_intersection(lhs, rhs, *difference, format, relative_transform);
    retain_intersection(rhs, lhs, *difference, format, inverse_relative_transform);

    memcpy(difference->vertex_buffer.data, new_vertices.data(), new_vertices.size());
    memcpy(difference->index_buffer.data, new_indices.data(), new_indices.size());

    transform(new_vertices, relative_transform, new_indices.size());
    flip(new_vertices, new_indices, new_indices.size());

    return difference;
  }*/
}
