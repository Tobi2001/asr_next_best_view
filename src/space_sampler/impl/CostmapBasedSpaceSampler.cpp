/*
 * CostmapBasedSpaceSampler.cpp
 *
 *  Created on: Oct 16, 2014
 *      Author: ralfschleicher
 */

#include "next_best_view/space_sampler/impl/CostmapBasedSpaceSampler.hpp"

namespace next_best_view {
	CostmapBasedSpaceSampler::CostmapBasedSpaceSampler(const MapUtilityPtr &mapUtilityPtr) : mMapUtilityPtr(mapUtilityPtr) {
	}

	CostmapBasedSpaceSampler::~CostmapBasedSpaceSampler() { }

	SamplePointCloudPtr CostmapBasedSpaceSampler::getSampledSpacePointCloud(SimpleVector3 currentSpacePosition, float contractor) {
		SamplePointCloudPtr sampledSpacePointCloudPtr = SamplePointCloudPtr(new SamplePointCloud());

		// Calculate maximum span
		SimpleVector4 minVector, maxVector;
		pcl::getMinMax3D(*this->getInputCloud(), minVector, maxVector);
		// the surrounding cube.
		SimpleVector3 spanCube = maxVector.block<3,1>(0,0) - minVector.block<3,1>(0,0);

		double maximumSpan = spanCube.lpNorm<Eigen::Infinity>() * contractor;

		uint32_t samples = this->getSamples();

		// add current viewport
		SamplePoint currentSamplePoint(currentSpacePosition);
		sampledSpacePointCloudPtr->push_back(currentSamplePoint);

		SimpleQuaternionCollectionPtr spherePointsPtr = MathHelper::getOrientationsOnUnitSphere(samples);
		BOOST_FOREACH(SimpleQuaternion orientation, *spherePointsPtr) {
			SimpleVector3 vector = maximumSpan * (orientation.toRotationMatrix() * SimpleVector3::UnitX());
			vector[2] = 0.0;
			vector += currentSpacePosition;


			Ray ray;
			if (!mMapUtilityPtr->doRaytracing(currentSpacePosition, vector, ray)) {
				SimpleVector3 obstacle;
				for (Ray::reverse_iterator iter = ray.rbegin(); iter != ray.rend(); iter++) {
					if (mMapUtilityPtr->isOccupancyValueAcceptable(iter->occupancy)) {
						mMapUtilityPtr->mapToWorldCoordinates(iter->x, iter->y, obstacle);
						break;
					}
				}
				vector = obstacle;
				vector[2] = currentSpacePosition[2];
			}

			SamplePoint samplePoint(vector);
			sampledSpacePointCloudPtr->push_back(samplePoint);
		}

		return sampledSpacePointCloudPtr;
	}
}


