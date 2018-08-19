#include <ros/ros.h>
#include <ltStar_temp.h>
#include <octomap_msgs/Octomap.h>
#include <octomap_msgs/conversions.h>
#include <visualization_msgs/Marker.h>
#include <marker_publishing_utils.h>
#include <std_srvs/Empty.h>

#include <tf2/LinearMath/Transform.h>

#include <array>

#include <visualization_msgs/MarkerArray.h>



namespace LazyThetaStarOctree
{
    octomap::OcTree* octree;
	double sidelength_lookup_table  [16];
	ros::Publisher ltstar_reply_pub;
	ros::Publisher marker_pub;

	bool octomap_init;
	bool publish_free_corridor_arrows;

	void learnTf_theHardWay()
	{
      	tf2::Vector3 toTest_start (0, 0, 0);
		tf2::Vector3 toTest_end (1, 0, 0);

		tf2::Vector3 offset = (toTest_start - toTest_end) / 2;


    	tf2::Vector3 origin (0, 0, 0);
    	tf2::Vector3 yAxis(0, 1, 0);
    	tf2::Vector3 zAxis(0, 0, 1);
    	tf2::Quaternion aroundY (yAxis, M_PI/4);  // Does not work with PI/2!!! Only one rotation is done
    	tf2::Quaternion aroundZ (zAxis, M_PI/4);
    	tf2::Quaternion no_rotation (0, 0, 0, 1);
		tf2::Transform rotation_pitch, rotation_yaw;
		rotation_pitch.setOrigin(origin);
		rotation_pitch.setRotation(aroundY);

		rotation_yaw.setOrigin(origin);
		rotation_yaw.setRotation(aroundZ);

		tf2::Transform translation_to_center;
		translation_to_center.setOrigin(offset);
		translation_to_center.setRotation(no_rotation);


		tf2::Transform translation_from_center;
		translation_from_center.setOrigin(-offset);
		translation_from_center.setRotation(no_rotation);

		tf2::Transform final_transform =  translation_from_center * rotation_yaw * rotation_pitch  * translation_to_center;

      	
		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 10);	

		toTest_start = final_transform * toTest_start;
		toTest_end = final_transform * toTest_end;

		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 2);	
	}

	void learnTf_theHardWay_pointSet()
	{
		// PUBLISH ORIGINAL
		const int point_count = 5;
		std::array < tf2::Vector3, point_count> start_points =
			{
				tf2::Vector3(0,  2, 0),
				tf2::Vector3(0,  1, 0),
				tf2::Vector3(0,  0, 0),
				tf2::Vector3(0, -1, 0),
				tf2::Vector3(0, -2, 0),
			};
		std::array < tf2::Vector3, point_count> end_points =
			{
				tf2::Vector3(5, -3, 0),
				tf2::Vector3(5, -4, 0),
				tf2::Vector3(5, -5, 0),
				tf2::Vector3(5, -6, 0),
				tf2::Vector3(5, -7, 0),
			};
		visualization_msgs::MarkerArray  marker_array_original, marker_array_rotated;
		for (int i = 0; i < point_count; ++i)
		{
			rviz_interface::push_arrow_corridor(
				octomath::Vector3 (start_points[i].getX(), start_points[i].getY(), start_points[i].getZ()), 
				octomath::Vector3 (end_points[i].getX(), end_points[i].getY(), end_points[i].getZ()), 
				marker_pub, 10+i, marker_array_original);
		}
		marker_pub.publish(marker_array_original);

		// CALCULATE ROTATION 
    	tf2::Vector3 origin(0, 0, 0);
    	tf2::Vector3 zAxis (0, 0, 1);
    	tf2::Quaternion aroundZ (zAxis, -(M_PI/4));
		tf2::Transform rotation_pitch, rotation_yaw;
		rotation_yaw.setOrigin(origin);
		rotation_yaw.setRotation(aroundZ);

    	tf2::Quaternion no_rotation (0, 0, 0, 1);
		// tf2::Vector3 offset = (start_points[0] - end_points[0]) / 2;
		tf2::Vector3 offset = end_points[2];
		tf2::Transform translation_to_center;
		translation_to_center.setOrigin(offset);
		translation_to_center.setRotation(no_rotation);
		tf2::Transform translation_from_center;
		translation_from_center.setOrigin(-offset);
		translation_from_center.setRotation(no_rotation);

		tf2::Transform final_transform_start = rotation_yaw;
		tf2::Transform final_transform_end   = translation_from_center * rotation_yaw  * translation_to_center;

		//     APPLY ROTATION 
		tf2::Vector3 rotated_start, rotated_end;
		for (int i = 0; i < point_count; ++i)
		{
			rotated_start = final_transform_start * start_points[i];
			rotated_end   = final_transform_end   * end_points[i];
			rviz_interface::push_arrow_corridor(
				octomath::Vector3 (rotated_start.getX(), rotated_start.getY(), rotated_start.getZ()), 
				octomath::Vector3 (rotated_end.getX(), rotated_end.getY(), rotated_end.getZ()), 
				marker_pub, 20+i, marker_array_rotated);
		}
		marker_pub.publish(marker_array_rotated);
	}

	void learnTf_builtInFunctions()
	{
      	tf2::Vector3 toTest_start (0, 0, 0);
		tf2::Vector3 toTest_end (1, 0, 0);
    	tf2::Vector3 origin (0, 0, 0);
    	tf2::Quaternion ypr_yaw_pitch ;
    	ypr_yaw_pitch.setEuler(M_PI/2, 0, M_PI/2) ;	
		tf2::Transform rotation_yaw_pitch;

		rotation_yaw_pitch.setOrigin(origin);
		rotation_yaw_pitch.setRotation(ypr_yaw_pitch);
      	
		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 10);	

		toTest_start = rotation_yaw_pitch * toTest_start;
		toTest_end = rotation_yaw_pitch * toTest_end;

		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 2);	
	}

	void learnTf_builtInFunctions_twoSteps()
	{
      	tf2::Vector3 toTest_start (0, 0, 0);
		tf2::Vector3 toTest_end (1, 0, 0);
    	tf2::Vector3 origin (0, 0, 0);
    	tf2::Quaternion ypr_yaw, ypr_pitch ;
    	ypr_yaw.setEuler(0, 0, M_PI/2 ) ;	
    	ypr_pitch.setEuler(M_PI/2, 0, 0) ;	
		tf2::Transform rotation_yaw, rotation_pitch;

		rotation_yaw.setOrigin(origin);
		rotation_yaw.setRotation(ypr_yaw);

		rotation_pitch.setOrigin(origin);
		rotation_pitch.setRotation(ypr_pitch);
      	
		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 10);	

		toTest_start = rotation_yaw * toTest_start;
		toTest_end = rotation_yaw * toTest_end;

		toTest_start = rotation_pitch * toTest_start;
		toTest_end = rotation_pitch * toTest_end;


		rviz_interface::publish_arrow_path_unreachable(
			octomath::Vector3 (toTest_start.getX(), toTest_start.getY(), toTest_start.getZ()), 
			octomath::Vector3 (toTest_end.getX(), toTest_end.getY(), toTest_end.getZ()), 
			marker_pub, 2);	
	}

	octomath::Vector3 getOrthogonalInXNormalized(octomath::Vector3 const& start, octomath::Vector3 const& goal)
	{
		octomath::Vector3 xAxis (1, 0, 0);
		octomath::Vector3 sg = goal - start;
		ROS_WARN_STREAM("sg = " << start << " - " << goal << " = " << sg); 
		octomath::Vector3 orthogonal = sg.cross(xAxis);
		orthogonal.normalize();
		ROS_WARN_STREAM("orthogonal       = " << orthogonal);
		orthogonal += start;
		ROS_WARN_STREAM("Applied to start = " << orthogonal);

		return orthogonal;
	}

	void rotateByCrossProduct()
	{
		// octomath::Vector3 toTest_start (0, 0, 0);
		// octomath::Vector3 toTest_end (0, 0, 1);
		// octomath::Vector3 orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (0, -1, 0);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);


		// octomath::Vector3 toTest_start (0, 0, 0);
		// octomath::Vector3 toTest_end (0, 0, 10);
		// octomath::Vector3 orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (0, -1, 0);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);


		// octomath::Vector3 toTest_start (0, 0, 0);
		// octomath::Vector3 toTest_end (0, 1, 3);
		// octomath::Vector3 orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (0 -0.948683 0.316228);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);

		// octomath::Vector3 toTest_start (2, 2, 2);
		// octomath::Vector3 toTest_end (2, 2, 3);
		// octomath::Vector3 orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (2 3 2);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);


		octomath::Vector3 toTest_start (0, 0, 0);
		octomath::Vector3 toTest_end (0, 2, 0);
		octomath::Vector3 orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (2 3 2);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);
		

		rviz_interface::publish_arrow_corridor(toTest_start, toTest_end, marker_pub, 10);
		rviz_interface::publish_arrow_path_unreachable(toTest_start, orthogonal, marker_pub, 2);	



		toTest_start = octomath::Vector3 (0, 0, 0);
		toTest_end= octomath::Vector3 (5, 0, 0);
		orthogonal = getOrthogonalInXNormalized(toTest_start, toTest_end);
		// octomath::Vector3 orthogonal_rightAnswer (2 3 2);
		// ASSERT_EQ(orthogonal_rightAnswer, orthogonal);
		

		rviz_interface::publish_arrow_corridor(toTest_start, toTest_end, marker_pub, 11);
		rviz_interface::publish_arrow_path_unreachable(toTest_start, orthogonal, marker_pub, 3);	
	}

	
	void octomap_callback(const octomap_msgs::Octomap::ConstPtr& octomapBinary){
		delete octree;
		ROS_WARN_STREAM("Got the octomap");
		octree = (octomap::OcTree*)octomap_msgs::binaryMsgToMap(*octomapBinary);
	    LazyThetaStarOctree::fillLookupTable(octree->getResolution(), octree->getTreeDepth(), sidelength_lookup_table); 
		octomap_init = true;
	}

	void ltstar_benchmark_callback_debug(const path_planning_msgs::LTStarBenchmarkRequest::ConstPtr& path_request)
	{
		rviz_interface::publish_deleteAll(marker_pub);
		path_planning_msgs::LTStarReply reply;
		reply.waypoint_amount = 0;
		reply.success = false;
		// if(octomap_init)
		// {
			
		learnTf_theHardWay_pointSet();



			// octomath::Vector3 disc_initial(path_request->start.x, path_request->start.y, path_request->start.z);
			// octomath::Vector3 disc_final(path_request->goal.x, path_request->goal.y, path_request->goal.z);
			// octomath::Vector3 geofence(path_request->safety_margin, path_request->safety_margin, path_request->safety_margin);
			// getCorridorOccupancy_reboot(*octree, disc_initial, disc_final, geofence, marker_pub, true);
			// getCorridorOccupancy       (*octree, disc_initial, disc_final, geofence, marker_pub, true);
		// }
		// else
		// {
		// 	ROS_ERROR_STREAM("[LTStar] Cannot generate path because no octomap has been received.");
		// 	reply.success=false;
		// 	reply.request_id = path_request->request_id;
		// 	reply.waypoint_amount = 0;
		// }
		
	}
}

int main(int argc, char **argv)
{
	LazyThetaStarOctree::publish_free_corridor_arrows = true;
	ros::init(argc, argv, "ltstar_debug_node");
	ros::NodeHandle nh;
	LazyThetaStarOctree::marker_pub = nh.advertise<visualization_msgs::MarkerArray>("ltstar_path", 1);
	ros::Subscriber octomap_sub = nh.subscribe<octomap_msgs::Octomap>("/octomap_binary", 10, LazyThetaStarOctree::octomap_callback);
	ros::Subscriber ltstar_benchmark_sub = nh.subscribe<path_planning_msgs::LTStarBenchmarkRequest>("ltstar_request_benchmark", 10, LazyThetaStarOctree::ltstar_benchmark_callback_debug);
	LazyThetaStarOctree::ltstar_reply_pub = nh.advertise<path_planning_msgs::LTStarReply>("ltstar_reply", 10);

  	ros::spin();
}
