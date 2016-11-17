#pragma once

#include <ros/package.h>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <assimp_devel/Importer.hpp>
#include <assimp_devel/scene.h>
#include <assimp_devel/postprocess.h>
#include <Eigen/Dense>
#include <iostream>
#include <tuple>
#include "next_best_view/helper/MapHelper.hpp"
#include "next_best_view/helper/TypeHelper.hpp"
#include "next_best_view/helper/VoxelGridHelper.hpp"
#include "next_best_view/helper/VisualizationsHelper.hpp"
#include "typedef.hpp"

namespace next_best_view {
    class WorldHelper {
    private:
        const double EPSILON = 10e-6;

        MapHelperPtr mMapHelperPtr;
        VoxelGridHelperPtr mVoxelGridHelperPtr;
        VisualizationHelperPtr mVisHelperPtr;
        DebugHelperPtr mDebugHelperPtr;

        double mMapVoxelSize, mWorldVoxelSize;

        bool mVisualizeRaytracing;

    public:
        WorldHelper(MapHelperPtr mapHelperPtr, std::string filePath, double voxelSize, double worldHeight, bool visualizeRaytracing);

        void filterOccludedObjects(SimpleVector3 cameraPosition, ObjectPointCloudPtr objectPointCloud, IndicesPtr indices,
                                                                                                            IndicesPtr &filteredIndices);

        bool isOccluded(SimpleVector3 cameraPosition, SimpleVector3 targetPosition);

    private:
        bool isOccluded(SimpleVector3 cameraPos, SimpleVector3 targetPos, GridVector3 currVoxelPos, std::vector<GridVector3> &traversedVoxels);

        void worldToVoxelGridCoordinates(const SimpleVector3 &worldPos, std::vector<GridVector3> &result);

        void worldToVoxelGridCoordinates(const std::vector<SimpleVector3> &worldPositions, std::vector<GridVector3> &results);

        // TODO remove
        void mapToWorldCoordinates(const SimpleVector3 &mapPos, SimpleVector3 &result);

        void initializeVoxelGridHelper(double worldHeight);

        void loadVoxelGrid(std::string filePath);

        void parseXMLFile(const string &filePath, std::vector<string> &meshResources, std::vector<SimpleVector3> &positions,
                                                    std::vector<SimpleQuaternion> &orientations, std::vector<SimpleVector3> &scales);

        bool parsePoseString(const std::string &poseString, std::vector<double> &poseVec);

        std::string getAbsolutePath(std::string filePath);

        void loadMeshFile(const std::string &meshResource, const SimpleVector3 &position, const SimpleQuaternion &orientation,
                                                const SimpleVector3 &scale, std::vector<std::vector<SimpleVector3>> &faces);

        void addPoint(const SimpleVector3& point);

        void addLine(const std::vector<SimpleVector3>& vertices);

        void addTriangle(const std::vector<SimpleVector3>& vertices);

        SimpleVector3 voxelToWorldPosition(const GridVector3& gridPos);

        void voxelToWorldBox(const GridVector3& gridPos, SimpleVector3& min, SimpleVector3& max);

        void voxelGridBox(const std::vector<GridVector3> &gridPositions, GridVector3 &min, GridVector3 &max);

        bool voxelVerticesAreNeighbours(const SimpleVector3 &vertexA, const SimpleVector3 &vertexB);

        bool lineIntersectsVoxel(const SimpleVector3& lineStartPos, const SimpleVector3& lineEndPos, const GridVector3& gridPos);

        bool lineIntersectsVoxel(const SimpleVector3& lineStartPos, const SimpleVector3& lineEndPos, const GridVector3& gridPos,
                                                                                                                double tLowerBound);

        bool lineIntersectsVoxel(const SimpleVector3& lineStartPos, const SimpleVector3& lineEndPos, const GridVector3& gridPos,
                                                                                                    double tLowerBound, double &tMin);

        bool lineIntersectsVoxelHelper(const SimpleVector3 &lineStartPos, const SimpleVector3 &lineEndPos, const SimpleVector3 &voxelCoord,
                                                const SimpleVector3 &voxelMin, const SimpleVector3 &voxelMax, double tLowerBound, double &tMin);

        bool lineIntersectsTriangle(const SimpleVector3 &lineStartPos, const SimpleVector3 &lineEndPos,
                                                        const std::vector<SimpleVector3> triangleVertices);

        bool equalVoxels(const GridVector3& a, const GridVector3& b);

    };

    typedef boost::shared_ptr<WorldHelper> WorldHelperPtr;
    typedef std::tuple<int,int,int> VoxelTuple;
    typedef std::map<VoxelTuple, IndicesPtr> VoxelToIndices;
    typedef std::pair<VoxelTuple, IndicesPtr> VoxelAndIndices;
}
