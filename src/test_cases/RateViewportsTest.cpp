/**

Copyright (c) 2016, Aumann Florian, Borella Jocelyn, Heller Florian, Meißner Pascal, Schleicher Ralf, Stöckle Patrick, Stroh Daniel, Trautmann Jeremias, Walter Milena, Wittenbeck Valerij
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. The use is explicitly not permitted to any application which deliberately try to kill or do harm to any living creature.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#define BOOST_TEST_STATIC_LINK

#include <boost/test/included/unit_test.hpp>
#include "next_best_view/test_cases/BaseTest.h"
#include "asr_next_best_view/GetAttributedPointCloud.h"
#include "asr_next_best_view/RateViewports.h"
#include "asr_next_best_view/RatedViewport.h"

using namespace asr_next_best_view;
using namespace boost::unit_test;

class RateViewportsTest : public BaseTest {
public:
    RateViewportsTest(bool silent) : BaseTest (true, silent) { }

    virtual ~RateViewportsTest() {}

    geometry_msgs::Pose setup() {
        ROS_INFO("Generate clusters");
        // number of clusters
        int nHp = 2;
        SimpleVector3* location = new SimpleVector3[nHp];
        location[0] = SimpleVector3(-0.43232, 0.6958, 0.7211);
        location[1] = SimpleVector3(-0.283309, 0.710407,  0.718543);

        SimpleQuaternion* orientation = new SimpleQuaternion[nHp];
        orientation[0] = euler2Quaternion(-90, 0.0, 0.0);
        orientation[1] = euler2Quaternion(-90, 0.0, 0.0);

        std::string* types = new std::string[nHp];
        types[0] = "Smacks";
        types[1] = "PlateDeep";

        int elementsPerHp[2] = { 3, 1 };

        SetAttributedPointCloud apc;
        for (std::size_t idx = 0; idx < (std::size_t)nHp; idx++) {
            for (std::size_t cnt = 0; cnt < (std::size_t)elementsPerHp[idx]; cnt++) {
                SimpleVector3 curLocation = location[idx];

                asr_msgs::AsrAttributedPoint element;

                geometry_msgs::Pose pose;
                pose.orientation.w = orientation[idx].w();
                pose.orientation.x = orientation[idx].x();
                pose.orientation.y = orientation[idx].y();
                pose.orientation.z = orientation[idx].z();
                pose.position.x = curLocation[0] - 0.1 * cnt; // put smacks in row next to each other
                pose.position.y = curLocation[1];
                pose.position.z = curLocation[2];

                element.type = types[idx];
                element.identifier = std::to_string(idx);
                element.pose = pose;
                apc.request.point_cloud.elements.push_back(element);
            }
        }

        ROS_INFO("Setting initial pose");
        geometry_msgs::Pose initialPose;
        initialPose.position.x = -0.383223;
        initialPose.position.y = 0.157997;
        initialPose.position.z = 1.32;
        initialPose.orientation.x = -0.17266;
        initialPose.orientation.y = 0.2115588;
        initialPose.orientation.z = 0.60825979;
        initialPose.orientation.w = 0.74528563;
        this->setInitialPose(initialPose);

        // set PointCloud
        if (!mSetPointCloudClient.call(apc.request, apc.response)) {
            ROS_ERROR("Could not set initial point cloud.");
        }

        asr_next_best_view::GetAttributedPointCloud gpc;
        if (!mGetPointCloudClient.call(gpc)) {
            ROS_ERROR("Could not get initial point cloud.");
        }
        return initialPose;
    }

    void test() {
        geometry_msgs::Pose initialPose = setup();

        ros::ServiceClient rateViewportsClient;
        ros::service::waitForService("/nbv/rate_viewports", -1);
        rateViewportsClient = mNodeHandle->serviceClient<asr_next_best_view::RateViewports>("/nbv/rate_viewports");

        // get nbv
        asr_next_best_view::GetNextBestView nbv;
        if (!mGetNextBestViewClient.call(nbv.request, nbv.response)) {
            ROS_ERROR("Something went wrong in next best view");
            return;
        }

        asr_next_best_view::RateViewports rateViewports;
        std::vector<std::string> requestObjectTypes = { "Smacks" };
        rateViewports.request.object_type_name_list = requestObjectTypes;
        rateViewports.request.current_pose = initialPose;
        rateViewports.request.viewports.push_back(nbv.response.viewport.pose);

        if (!rateViewportsClient.call(rateViewports.request, rateViewports.response)) {
            ROS_ERROR("Something went wrong in rate viewports");
            return;
        }
        asr_next_best_view::RatedViewport ratedViewport = rateViewports.response.sortedRatedViewports[0];
        ROS_INFO_STREAM("nbv: " << nbv.response);
        ROS_INFO_STREAM("ratedView: " << ratedViewport);
        BOOST_CHECK_MESSAGE(rateViewports.request.viewports.size() == rateViewports.response.sortedRatedViewports.size(), "ratedViewports should be equal");
        BOOST_CHECK_MESSAGE(ratedViewport.object_type_name_list.size() == requestObjectTypes.size(), "object_type_name_list was modified");

        // generate multiple poses in a + shape around nbv
        double variation = 0.1;
        rateViewports = RateViewports();
        geometry_msgs::Pose variationPose = nbv.response.viewport.pose;
        variationPose.position.x += variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.x -= variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.y += variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.y -= variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.x += 2.0 * variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.x -= 2.0 * variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.y += 2.0 * variation;
        rateViewports.request.viewports.push_back(variationPose);

        variationPose = nbv.response.viewport.pose;
        variationPose.position.y -= 2.0 * variation;
        rateViewports.request.viewports.push_back(variationPose);

        requestObjectTypes = {"Smacks"};
        rateViewports.request.object_type_name_list = requestObjectTypes;

        if (!rateViewportsClient.call(rateViewports.request, rateViewports.response)) {
            ROS_ERROR("Something went wrong in rate viewports");
            return;
        }

        ratedViewport = rateViewports.response.sortedRatedViewports[0];
        BOOST_CHECK_MESSAGE(rateViewports.request.viewports.size() == rateViewports.response.sortedRatedViewports.size(), "ratedViewports should be equal");
        BOOST_CHECK_MESSAGE(ratedViewport.object_type_name_list.size() == requestObjectTypes.size(), "object_type_name_list was modified");

        ROS_INFO_STREAM("rated viewports:");
        for (asr_next_best_view::RatedViewport viewport : rateViewports.response.sortedRatedViewports) {
            ROS_INFO_STREAM(viewport);
        }

        // empty object_type_name_list
        rateViewports = RateViewports();
        variationPose = nbv.response.viewport.pose;
        variationPose.position.x += variation;
        rateViewports.request.viewports.push_back(variationPose);

        requestObjectTypes = {};
        rateViewports.request.object_type_name_list = requestObjectTypes;

        if (!rateViewportsClient.call(rateViewports.request, rateViewports.response)) {
            ROS_ERROR("Something went wrong in rate viewports");
            return;
        }

        // empty viewports
        rateViewports = RateViewports();

        if (!rateViewportsClient.call(rateViewports.request, rateViewports.response)) {
            ROS_ERROR("Something went wrong in rate viewports");
            return;
        }
    }
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
    ros::init(argc, argv, "nbv_test");
    ros::start();

    test_suite* evaluation = BOOST_TEST_SUITE("Evaluation NBV");

    bool silent = false;
    if (argc >= 2) {
        // caseinsensitive comparison
        if (boost::iequals(argv[1], "silent") || boost::iequals(argv[1], "s")) {
            silent = true;
        }
    }
    boost::shared_ptr<RateViewportsTest> testPtr(new RateViewportsTest(silent));

    evaluation->add(BOOST_CLASS_TEST_CASE(&RateViewportsTest::test, testPtr));

    framework::master_test_suite().add(evaluation);

    return 0;
}




