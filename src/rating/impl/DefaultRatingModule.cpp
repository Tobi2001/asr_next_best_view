/*
 * DefaultRatingModule.cpp
 *
 *  Created on: Sep 29, 2014
 *      Author: ralfschleicher
 */

#include "next_best_view/rating/impl/DefaultRatingModule.hpp"

namespace next_best_view {

    DefaultRatingModule::DefaultRatingModule() : RatingModule(), mNormalAngleThreshold(M_PI * .5) { }

    DefaultRatingModule::~DefaultRatingModule() { }

    bool DefaultRatingModule::setBestScoreContainer(const ViewportPoint &currentViewport, ViewportPoint &candidateViewport) {
        // reset the cached data
        this->resetCache();

        // get the power set of the object name set
        ObjectNameSetPtr objectNameSetPtr = candidateViewport.object_name_set;
        ObjectNamePowerSetPtr powerSetPtr = MathHelper::powerSet<ObjectNameSet> (objectNameSetPtr);

        //Create list of all viewports
        ViewportPointCloudPtr viewports = ViewportPointCloudPtr(new ViewportPointCloud());

        // do the filtering for the single object types
        for (std::set<ObjectNameSetPtr>::iterator subSetIter = powerSetPtr->begin(); subSetIter != powerSetPtr->end(); ++subSetIter) {
            ViewportPoint viewport;
            if (!candidateViewport.filterObjectNames(*subSetIter, viewport)) {
                continue;
            }

            // if the rating is not feasible: skip adding to point cloud
            if (!this->setSingleScoreContainer(currentViewport, viewport)) {
                continue;
            }

            if (!viewport.score) {
                ROS_ERROR("Single score result is nullpointer.");
            }

            viewports->push_back(viewport);
        }

        if (!this->getBestViewport(viewports, candidateViewport)) {
            return false;
        }

        return true;
    }

    bool DefaultRatingModule::getBestViewport(const ViewportPointCloudPtr &viewports, ViewportPoint &bestViewport) {
        // if there aren't any viewports, the search failed.
        ROS_DEBUG_STREAM("Viewport Point size : " << viewports->size());
        if (viewports->size() == 0) {
            return false;
        }

        ViewportPointCloud::iterator maxElement = std::max_element(viewports->begin(), viewports->end(), boost::bind(&RatingModule::compareViewports, *this, _1, _2));

        if (maxElement == viewports->end()) {
            ROS_DEBUG_STREAM("No maximum element found.");
            return false;
        }

        bestViewport = *maxElement;

        ROS_DEBUG_STREAM("bestViewport position: (" << bestViewport.getPosition()(0,1) << ","
                            << bestViewport.getPosition()(0,2) << "," << bestViewport.getPosition()((0,3)) << ")");

        return true;
    }

    bool DefaultRatingModule::compareViewports(ViewportPoint &a, ViewportPoint &b) {
        return this->compareScoreContainer(a.score, b.score);
    }

    bool DefaultRatingModule::compareScoreContainer(const BaseScoreContainerPtr &a, const BaseScoreContainerPtr &b) {
        if (!a) {
            ROS_ERROR("Score container a is nullpointer");
        }

        if (!b) {
            ROS_ERROR("Score container b is nullpointer");
        }

        DefaultScoreContainerPtr defA = boost::static_pointer_cast<DefaultScoreContainer>(a);
        DefaultScoreContainerPtr defB = boost::static_pointer_cast<DefaultScoreContainer>(b);

        Precision ratingA = this->getRating(defA);
        Precision ratingB = this->getRating(defB);

        return ratingA < ratingB;
    }

    // TODO remove?
    void DefaultRatingModule::updateObjectPoints(const ViewportPoint &viewport) {
        BOOST_FOREACH(int index, *viewport.child_indices) {
            ObjectPoint &objectPoint = viewport.child_point_cloud->at(index);

            SimpleVector3CollectionPtr normalVectorCollectionPtr(new SimpleVector3Collection());
            BOOST_FOREACH(SimpleVector3 normalVector, *objectPoint.normal_vectors) {
                float rating = this->getNormalRating(viewport, normalVector);
                // filter all the object normals that are in the given viewport
                if (rating == 0.0) {
                    normalVectorCollectionPtr->push_back(normalVector);
                }
            }
            objectPoint.normal_vectors = normalVectorCollectionPtr;
        }
    }

    BaseScoreContainerPtr DefaultRatingModule::getScoreContainerInstance() {
        return BaseScoreContainerPtr(new DefaultScoreContainer());
    }

    float DefaultRatingModule::getOrientationRating(const ViewportPoint &viewport, ObjectPoint &objectPoint) {
		float maxRating = 0.0;

        // check the ratings for all normals and pick the best
        BOOST_FOREACH(int index, *objectPoint.active_normal_vectors) {
            SimpleVector3 objectNormalVector = objectPoint.normal_vectors->at(index);
            maxRating = std::max(this->getNormalRating(viewport, objectNormalVector), maxRating);
		}

		return maxRating;
	}

    float DefaultRatingModule::getNormalRating(const ViewportPoint &viewport, const SimpleVector3 &objectNormalVector) {
        SimpleQuaternion cameraOrientation = viewport.getSimpleQuaternion();
        SimpleVector3 cameraOrientationVector = MathHelper::getVisualAxis(cameraOrientation);

        // rate the angle between the camera orientation and the object normal
        float angle = MathHelper::getAngle(-cameraOrientationVector, objectNormalVector);
        float rating = this->getNormalizedRating(angle, mNormalAngleThreshold);

        return rating;
	}

    float DefaultRatingModule::getProximityRating(const ViewportPoint &viewport, const ObjectPoint &objectPoint) {

        SimpleVector3 cameraPosition = viewport.getPosition();
        SimpleQuaternion cameraOrientation = viewport.getSimpleQuaternion();
        SimpleVector3 cameraOrientationVector = MathHelper::getVisualAxis(cameraOrientation);
        SimpleVector3 objectPosition = objectPoint.getPosition();

        SimpleVector3 objectToCameraVector = cameraPosition - objectPosition;

        // project the object to the camera orientation vector in order to determine the distance to the mid
        float projection = MathHelper::getDotProduct(-cameraOrientationVector, objectToCameraVector);
        ROS_DEBUG_STREAM("projection value" << projection << " fcp " << fcp << " ncp " << ncp);

        // determine the distance of the object to the mid of the frustum
        float distanceToMid = abs(projection-(fcp+ncp)/2.0);
        float distanceThreshold = (fcp-ncp)/2.0;
        ROS_DEBUG_STREAM("distance to mid " << distanceToMid << " thresh "  << distanceThreshold );

        float rating = this->getNormalizedRating(distanceToMid, distanceThreshold);

        return rating;
    }


    float DefaultRatingModule::getFrustumPositionRating(const ViewportPoint &viewport, ObjectPoint &objectPoint)
    {
        SimpleVector3 cameraPosition = viewport.getPosition();
        SimpleQuaternion cameraOrientation = viewport.getSimpleQuaternion();
        SimpleVector3 cameraOrientationVector = MathHelper::getVisualAxis(cameraOrientation);

        float angleMin = (float) MathHelper::degToRad(std::min(fovV,fovH));
        SimpleVector3 objectPosition = objectPoint.getPosition();
        SimpleVector3 objectToCameraVector = cameraPosition - objectPosition;
        SimpleVector3 objectToCameraVectorNormalized = objectToCameraVector.normalized();

        // rating for how far the object is on the side of the camera view
        float angle = MathHelper::getAngle(-cameraOrientationVector, objectToCameraVectorNormalized);
        float sideRating = this->getNormalizedRating(angle, angleMin);

        // rating for how far the object is away from the camera
        float proximityRating = this->getProximityRating(viewport,objectPoint);

        // the complete frumstum position rating
        float rating = sideRating * proximityRating;

        ROS_DEBUG_STREAM("Frustum side rating "<< sideRating);
        ROS_DEBUG_STREAM("Frustum proximity rating  " << proximityRating);
        return rating;
    }

	float DefaultRatingModule::getRating(const DefaultScoreContainerPtr &a) {
        if (!a) {
            ROS_ERROR("Score container is nullpointer");
        }

        ROS_DEBUG_STREAM("utility: " << a->getUtility() << " costs: " << a->getCosts());

        // TODO divide instead of multiply?
        float result = a->getUtility() * a->getCosts();
        return result;
	}

    bool DefaultRatingModule::setSingleScoreContainer(const ViewportPoint &currentViewport,
                                                        ViewportPoint &candidateViewport) {
        DefaultScoreContainerPtr defRatingPtr(new DefaultScoreContainer());

        // set the utility
        float utility = this->getUtility(candidateViewport);
        defRatingPtr->setUtility(utility);

        // set the ratings for the orientations and the positions of the objects in the candidate viewport
        float orientationRating = this->getFullOrientationRating(candidateViewport);
        float positionRating = this->getFullPositionRating(candidateViewport);

        defRatingPtr->setOrientationRating(orientationRating);
        defRatingPtr->setPositionRating(positionRating);

        // TODO set recognizer costs only for viewports with multiple objects types?
        // set the costs
        double costs = this->getCosts(currentViewport, candidateViewport);
        defRatingPtr->setCosts(costs);

        candidateViewport.score = defRatingPtr;
        ROS_DEBUG("Orientation rating %f, Position rating %f", defRatingPtr->getOrientationRating(), defRatingPtr->getPositionRating());
        return (defRatingPtr->getUtility() > 0);
    }

    void DefaultRatingModule::setNormalAngleThreshold(double angle) {
        mNormalAngleThreshold = angle;
	}

    double DefaultRatingModule::getNormalAngleThreshold() {
        return mNormalAngleThreshold;
	}

    void DefaultRatingModule::setOmegaParameters(double omegaPan, double omegaTilt, double omegaRot, double omegaBase, double omegaRecognition) {
        this->mOmegaPan = omegaPan;
        this->mOmegaTilt = omegaTilt;
        this->mOmegaRot = omegaRot;
        this->mOmegaBase = omegaBase;
        this->mOmegaRecognition = omegaRecognition;
    }

    float DefaultRatingModule::getNormalizedRating(float deviation, float threshold) {
        if (deviation < threshold) {
            return .5 + .5 * cos(deviation * M_PI / threshold);
        }
        return 0.0;
    }

    double DefaultRatingModule::getUtility(const ViewportPoint &candidateViewport) {
        double utility = 0.0;

        // get the utility for each object type and sum them up
        BOOST_FOREACH(string objectType, *(candidateViewport.object_name_set)) {
            // set the utility for the object type if not already done
            if (mObjectUtilities.count(objectType) == 0) {
                setObjectUtilities(candidateViewport, objectType);
            }

            utility += mObjectUtilities[objectType];
        }

        // TODO: normalize?
        return utility;
    }

    double DefaultRatingModule::getFullOrientationRating(const ViewportPoint &candidateViewport) {
        double orientationRating = 0.0;

        // get the orientation rating for each object type and sum them up
        BOOST_FOREACH(string objectType, *(candidateViewport.object_name_set)) {
            // set the orientation rating for the object type if not already done
            if (mObjectOrientationRatings.count(objectType) == 0) {
                setObjectUtilities(candidateViewport, objectType);
            }

            orientationRating += mObjectOrientationRatings[objectType];
        }

        // TODO: normalize?
        return orientationRating;
    }

    double DefaultRatingModule::getFullPositionRating(const ViewportPoint &candidateViewport) {
        double positionRating = 0.0;

        // get the orientation rating for each object type and sum them up
        BOOST_FOREACH(string objectType, *(candidateViewport.object_name_set)) {
            // set the orientation rating for the object type if not already done
            if (mObjectPositionRatings.count(objectType) == 0) {
                setObjectUtilities(candidateViewport, objectType);
            }

            positionRating += mObjectPositionRatings[objectType];
        }

        // TODO: normalize?
        return positionRating;
    }

    void DefaultRatingModule::setObjectUtilities(const ViewportPoint &candidateViewport, string objectType) {
        float orientationRating = 0.0;
        float positionRating = 0.0;

        double maxElements = this->getInputCloud()->size();

        // build the sum of the orientation and frustum position ratings of all object points in the candidate camera view
        BOOST_FOREACH(int index, *(candidateViewport.child_indices)) {
            ObjectPoint &objectPoint = this->getInputCloud()->at(index);

            if (objectPoint.active_normal_vectors->size() == 0) {
                continue;
            }

            float currentOrientationRating = this->getOrientationRating(candidateViewport, objectPoint);
            float currentFrustumPositionRating = this->getFrustumPositionRating(candidateViewport, objectPoint);

            orientationRating += currentOrientationRating;
            positionRating += currentFrustumPositionRating;
        }

        // set the orientation and position ratings in relation to the amount of object points
        orientationRating /= maxElements;
        positionRating /= maxElements;

        // set the utility
        float utility = orientationRating * positionRating;

        // cache the utility and the orientation and position rating
        mObjectUtilities[objectType] = utility;
        mObjectOrientationRatings[objectType] = orientationRating;
        mObjectPositionRatings[objectType] = positionRating;
    }

    double DefaultRatingModule::getCosts(const ViewportPoint &sourceViewport,
                                            const ViewportPoint &targetViewport) {
        // set the cache members needed for the costs if not already done
        if (!mTargetState) {
            this->setRobotState(sourceViewport, targetViewport);
        }

        if (mOmegaPTU < 0) {
            this->setOmegaPTU();
        }

        if (mMovementCosts < 0) {
            this->setMovementCosts();
        }

        if (mCostsNormalization < 0) {
            this->setCostsNormalization();
        }

        // TODO it could happen that this check does not work when the input cloud changed
        // problem: this->setInputCloud cannot be overwritten
        if (mMaxRecognitionCosts < 0 || mLastInputCloud != this->getInputCloud()) {
            this->setMaxRecognitionCosts();
        }

        // get the costs for the recoginition of the objects
        double normalizedRecognitionCosts = this->getNormalizedRecognitionCosts(targetViewport);

        // calculate the full movement costs
        double fullCosts = (mMovementCosts + normalizedRecognitionCosts * mOmegaRecognition) / mCostsNormalization;

        return fullCosts;
    }

    double DefaultRatingModule::getNormalizedRecognitionCosts(const ViewportPoint &targetViewport) {
        // get the costs for the recoginition of each object type
        Precision recognitionCosts = 0;
        BOOST_FOREACH(string objectName, *(targetViewport.object_name_set)) {
            recognitionCosts += mCameraModelFilterPtr->getRecognizerCosts(objectName);
        }
        double normalizedRecognitionCosts = 1.0 - recognitionCosts / mMaxRecognitionCosts;

        ROS_DEBUG_STREAM("Recognitioncosts: " << recognitionCosts << " max " << mMaxRecognitionCosts);

        return normalizedRecognitionCosts;
    }

    void DefaultRatingModule::setRobotState(const ViewportPoint &sourceViewport, const ViewportPoint &targetViewport) {
        // set the source robot state
        RobotStatePtr sourceState = mRobotModelPtr->calculateRobotState(sourceViewport.getPosition(), sourceViewport.getSimpleQuaternion());
        mRobotModelPtr->setCurrentRobotState(sourceState);

        // set the target robot state
        mTargetState = mRobotModelPtr->calculateRobotState(targetViewport.getPosition(), targetViewport.getSimpleQuaternion());
    }

    void DefaultRatingModule::setOmegaPTU() {
        Precision movementCostsPTU_Pan = mRobotModelPtr->getPTU_PanMovementCosts(mTargetState);
        Precision movementCostsPTU_Tilt = mRobotModelPtr->getPTU_TiltMovementCosts(mTargetState);

        // set the PTU movement omega parameter
        if (movementCostsPTU_Tilt*mOmegaTilt > movementCostsPTU_Pan*mOmegaPan)
        {
            mOmegaPTU = mOmegaPan;
        }
        else
        {
            mOmegaPTU = mOmegaTilt;
        }
    }

    void DefaultRatingModule::setMovementCosts() {
        // get the different movement costs
        Precision movementCostsBase_Translation = mRobotModelPtr->getBase_TranslationalMovementCosts(mTargetState);
        Precision movementCostsBase_Rotation = mRobotModelPtr->getBase_RotationalMovementCosts(mTargetState);
        Precision movementCostsPTU;

        if (mOmegaPTU == mOmegaPan) {
            movementCostsPTU = mRobotModelPtr->getPTU_PanMovementCosts(mTargetState);
        }
        else {
            movementCostsPTU = mRobotModelPtr->getPTU_TiltMovementCosts(mTargetState);
        }

        ROS_DEBUG_STREAM("PTU Costs Tilt: " << movementCostsPTU);

        // set the movement costs
        mMovementCosts = movementCostsBase_Translation * mOmegaBase + movementCostsPTU * mOmegaPTU
                            + movementCostsBase_Rotation * mOmegaRot;

        ROS_DEBUG_STREAM("Movement; " << movementCostsBase_Translation << ", rotation " << movementCostsBase_Rotation << ", movement PTU: " << movementCostsPTU);
    }

    void DefaultRatingModule::setCostsNormalization() {
        mCostsNormalization = mOmegaBase + mOmegaPTU + mOmegaRot + mOmegaRecognition;
    }

    void DefaultRatingModule::setMaxRecognitionCosts() {
        mMaxRecognitionCosts = -1;
        mLastInputCloud = this->getInputCloud();

        double maxRecognitionCosts = 0;
        BOOST_FOREACH(ObjectPoint objectPoint, *(mLastInputCloud)) {
            maxRecognitionCosts += mCameraModelFilterPtr->getRecognizerCosts(objectPoint.type);
        }
        mMaxRecognitionCosts = maxRecognitionCosts;
    }

    void DefaultRatingModule::resetCache() {
        mObjectUtilities.clear();
        mObjectOrientationRatings.clear();
        mObjectPositionRatings.clear();
        mMovementCosts = -1;
        mCostsNormalization = -1;
        mOmegaPTU = -1;
        mTargetState = NULL;
    }
}

