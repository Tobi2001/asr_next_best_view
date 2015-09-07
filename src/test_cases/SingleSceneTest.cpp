/*
 * TestRecognitionManager.h
 *
 *  Created on: Mar 15, 2014
 *      Author: ralfschleicher
 */
#define BOOST_TEST_STATIC_LINK

#include <boost/test/included/unit_test.hpp>

#include "next_best_view/test_cases/BaseTest.h"
#include <chrono>
#include <fstream>

using namespace next_best_view;
using namespace boost::unit_test;

class SingleSceneTest : public BaseTest {

public:
    SingleSceneTest() : BaseTest (){
    }

    virtual ~SingleSceneTest() {}

    void iterationTest() {
		ros::ServiceClient setPointCloudClient = mNodeHandle.serviceClient<SetAttributedPointCloud>("/nbv/set_point_cloud");
        ros::ServiceClient getPointCloudClient = mNodeHandle.serviceClient<GetAttributedPointCloud>("/nbv/get_point_cloud");
		ros::ServiceClient getPointCloud2Client = mNodeHandle.serviceClient<GetPointCloud2>("/nbv/get_point_cloud2");
		ros::ServiceClient getNextBestViewClient = mNodeHandle.serviceClient<GetNextBestView>("/nbv/next_best_view");
		ros::ServiceClient updatePointCloudClient = mNodeHandle.serviceClient<UpdatePointCloud>("/nbv/update_point_cloud");
		ros::ServiceClient getSpaceSamplingClient = mNodeHandle.serviceClient<GetSpaceSampling>("/nbv/get_space_sampling");

        GetAttributedPointCloud gpc;
		SetAttributedPointCloud apc;
        std::chrono::time_point<std::chrono::system_clock> start, end;
        std::ofstream file("test.txt", std::ios::out | std::ios::trunc);
        SimpleVector3* hp = new SimpleVector3[15];
        hp[0] = SimpleVector3(3.09807621388, 21.7942286312,0.8);
        hp[1] = SimpleVector3(2.2320508103, 22.2942286316,0.8);
        hp[2] = SimpleVector3(3.96410161747, 21.2942286309,0.8);
        hp[3] = SimpleVector3(4.83012702105, 20.7942286305,0.8);
        hp[4] = SimpleVector3(5.69615242463, 20.2942286302,0.8);
        hp[5] = SimpleVector3(6.56217782822, 19.7942286298,0.8);
        hp[6] = SimpleVector3(7.4282032318, 19.2942286295,0.8);
        hp[7] = SimpleVector3(8.29422863538, 18.7942286291,0.8);
        hp[8] = SimpleVector3(9.16025403897, 18.2942286288,0.8);
        hp[9] = SimpleVector3(10.0262794426, 17.7942286284,0.8);
        hp[10] = SimpleVector3(10.8923048461, 17.2942286281,0.8);
        hp[11] = SimpleVector3(11.7583302497, 16.7942286277,0.8);
        hp[12] = SimpleVector3(12.6243556533, 16.2942286274,0.8);
        hp[13] = SimpleVector3(13.4903810569, 15.794228627,0.8);
        hp[14] = SimpleVector3(14.3564064605, 15.2942286267,0.8);

		ROS_INFO("Generiere Häufungspunkte");
		// Häufungspunkte
        for(unsigned int hpSize = 0; hpSize < 15; hpSize++)
        {
            for(unsigned int sampleSize = 50; sampleSize<400; sampleSize+=10)
            {
                SimpleVector3* hp = new SimpleVector3[hpSize];
                hp[0] = SimpleVector3(-1.35357105732, 1.06068396568,0.8);

                SimpleQuaternion* orientation = new SimpleQuaternion[hpSize];
                for(unsigned int i=0; i<hpSize; i++){ orientation[i] = SimpleQuaternion(0.0,0.0,-0.26,0.96);}

                std::string* types = new std::string[hpSize];
                for(unsigned int i=0; i<hpSize; i++){types[i] = "Smacks";}


               std::map<std::string, std::vector<AttributedPoint>* > objectPointCloudsMap;

                for (std::size_t idx = 0; idx < hpSize; idx++) {

                    if(objectPointCloudsMap.find(types[idx]) == objectPointCloudsMap.end())
                    {
                        objectPointCloudsMap[types[idx]]= new std::vector<AttributedPoint>();
                    }
                    for (std::size_t cnt = 0; cnt < sampleSize; cnt++) {
                        SimpleVector3 randomVector;
                        randomVector = MathHelper::getRandomVector(hp[idx], SimpleVector3(.05, .05, 0.01));

                        AttributedPoint element;

                        geometry_msgs::Pose pose;
                        pose.orientation.w = orientation[idx].w();
                        pose.orientation.x = orientation[idx].x();
                        pose.orientation.y = orientation[idx].y();
                        pose.orientation.z = orientation[idx].z();
                        pose.position.x = randomVector[0];
                        pose.position.y = randomVector[1];
                        pose.position.z = randomVector[2];

                        element.object_type = types[idx];
                        element.pose = pose;
                        objectPointCloudsMap[types[idx]]->push_back(element);
                        apc.request.point_cloud.elements.push_back(element);
                    }
                }

                ROS_INFO("Setze initiale Pose");
                geometry_msgs::Pose initialPose;
                initialPose.position.x = -0.974955737591;
                initialPose.position.y = -0.157173499465;
                initialPose.position.z = 1.32;

                initialPose.orientation.w = 0.718685498907;
                initialPose.orientation.x = 0.0;
                initialPose.orientation.y = 0.0;
                initialPose.orientation.z = 0.695335281472;
                this->setInitialPose(initialPose);

                apc.request.pose = initialPose;

                // Setze PointCloud
                setPointCloudClient.call(apc.request, apc.response);
                ros::Rate r(2);
                GetNextBestView nbv;
                nbv.request.current_pose = initialPose;
                ViewportPointCloudPtr viewportPointCloudPtr(new ViewportPointCloud());
                bool setPointCloud = false;
                int x = 1;
                while(ros::ok()) {
                    if(apc.request.point_cloud.elements.size() == 0)
                    {
                        ROS_ERROR("No elements were found");
                        break;
                    }
                    else if(setPointCloud)
                    {
                        setPointCloud = false;
                        if (!setPointCloudClient.call(apc.request, apc.response))
                        {
                            ROS_ERROR("Could not set point cloud");
                            break;
                        }
                    }

                    ROS_INFO_STREAM("Kalkuliere NBV "<< x);
                    start = std::chrono::system_clock::now();
                    if (!getNextBestViewClient.call(nbv.request, nbv.response)) {
                        ROS_ERROR("Something went wrong in next best view");
                        break;
                    }
                    end = std::chrono::system_clock::now();
                    std::chrono::duration<double> elapsed_seconds = end-start;
                    file << "Calc nextBestView : " << elapsed_seconds.count() << " " << ros::Time::now().toNSec() << std::endl;
                    file << "HaufungPunkt : " << hpSize << ", SamplingSize " << sampleSize << std::endl;
                    if (nbv.response.object_name_list.size() > 0)
                    {
                        getPointCloudClient.call(gpc);
                        apc.request.point_cloud.elements.clear();
                        apc.request.point_cloud.elements.insert(apc.request.point_cloud.elements.end(), gpc.response.point_cloud.elements.begin(), gpc.response.point_cloud.elements.end());

                        for(int i=0;i<nbv.response.object_name_list.size();i++)
                        {
                            std::vector<AttributedPoint> temp;
                            for (std::vector<AttributedPoint>::iterator it = apc.request.point_cloud.elements.begin(); it != apc.request.point_cloud.elements.end(); ++it)
                            {
                                if ((nbv.response.object_name_list[i].compare(it->object_type)) != 0)
                                {
                                    temp.push_back(*it);
                                }
                            }
                            apc.request.point_cloud.elements.clear();
                            apc.request.point_cloud.elements.insert(apc.request.point_cloud.elements.end(), temp.begin(), temp.end());
                            setPointCloud = true;
                        }
                    }
                    else
                    {
                        break;
                    }

                    if (!nbv.response.found) {
                        ROS_ERROR("No NBV found");
                        break;
                    }

                    UpdatePointCloud upc_req;
                    upc_req.request.update_pose = nbv.response.resulting_pose;

                    start = std::chrono::system_clock::now();
                    if(!updatePointCloudClient.call(upc_req)) {
                        ROS_ERROR("Update Point Cloud failed!");
                        break;
                    }
                    end = std::chrono::system_clock::now();
                    elapsed_seconds = end-start;
                    file << "Calc nextBestView : " << elapsed_seconds.count() << " " << ros::Time::now().toNSec() << std::endl;
                    file << "HaufungPunkt : " << hpSize << ", SamplingSize " << sampleSize << std::endl;

                    x++;

                    nbv.request.current_pose = nbv.response.resulting_pose;

                    ros::spinOnce();
                    waitForEnter();
                    ros::Duration(2).sleep();
                }
            }
        }
        file.close();
    }
};

test_suite* init_unit_test_suite( int argc, char* argv[] ) {
	ros::init(argc, argv, "nbv_test");
	ros::start();

	ros::Duration(5).sleep();

	test_suite* evaluation = BOOST_TEST_SUITE("Evaluation NBV");

    boost::shared_ptr<SingleSceneTest> testPtr(new SingleSceneTest());

    evaluation->add(BOOST_CLASS_TEST_CASE(&SingleSceneTest::iterationTest, testPtr));

    framework::master_test_suite().add(evaluation);

    return 0;
}
