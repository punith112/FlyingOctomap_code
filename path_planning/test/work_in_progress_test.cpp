#include <ltStar_lib_ortho.h>
#include <gtest/gtest.h>
#include <queue>

#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <visualization_msgs/Marker.h>


#include <chrono>

namespace LazyThetaStarOctree
{
	void testStraightLinesForwardNoObstacles(octomap::OcTree octree, octomath::Vector3 disc_initial, octomath::Vector3 disc_final,
		int const& max_time_secs = 55, double safety_margin = 0.1)
	{
		double sidelength_lookup_table  [octree.getTreeDepth()];
	   	LazyThetaStarOctree::fillLookupTable(octree.getResolution(), octree.getTreeDepth(), sidelength_lookup_table); 
		ros::Publisher marker_pub;
		
		// Initial node is not occupied
		octomap::OcTreeNode* originNode = octree.search(disc_initial);
		ASSERT_TRUE(originNode) << "Start node in unknown space";
		ASSERT_FALSE(octree.isNodeOccupied(originNode));
		// Final node is not occupied
		octomap::OcTreeNode* finalNode = octree.search(disc_final);
		ASSERT_TRUE(finalNode) << "Final node in unknown space";
		ASSERT_FALSE(octree.isNodeOccupied(finalNode));
		// The path is clear from start to finish
		octomath::Vector3 direction (1, 0, 0);
		octomath::Vector3 end;
		bool isOccupied = octree.castRay(disc_initial, direction, end, false, weightedDistance(disc_initial, disc_final));
		ASSERT_FALSE(isOccupied); // false if the maximum range or octree bounds are reached, or if an unknown node was hit.

		ResultSet statistical_data;
		std::vector<octomath::Vector3> planeOffsets;
		generateOffsets(octree.getResolution(), safety_margin, dephtZero, semiSphereOut );
		InputData input( octree, disc_initial, disc_final, safety_margin);
		std::list<octomath::Vector3> resulting_path = lazyThetaStar_(input, statistical_data, sidelength_lookup_table, PublishingInput( marker_pub, true), max_time_secs, true);
		// NO PATH
		ASSERT_NE(resulting_path.size(), 0);
		// CANONICAL: straight line, no issues
		// 2 waypoints: The center of start voxel & The center of the goal voxel
		double cell_size_goal = -1;
		octomath::Vector3 cell_center_coordinates_goal = disc_final;
		updateToCellCenterAndFindSize( cell_center_coordinates_goal, octree, cell_size_goal, sidelength_lookup_table);
		ASSERT_LT(      resulting_path.back().distance( cell_center_coordinates_goal ),  cell_size_goal   );
		double cell_size_start = -1;
		octomath::Vector3 cell_center_coordinates_start = disc_initial;
		updateToCellCenterAndFindSize( cell_center_coordinates_start, octree, cell_size_start, sidelength_lookup_table);
		ASSERT_LT(      resulting_path.begin()->distance( cell_center_coordinates_start ),  cell_size_start   );
		// LONG PATHS: 
		if(resulting_path.size() > 2)
		{
			std::list<octomath::Vector3>::iterator it = resulting_path.begin();
			octomath::Vector3 previous = *it;
			++it;
			// Check that there are no redundant parts in the path
			while(it != resulting_path.end())
			{
				ROS_INFO_STREAM("Step: " << *it);
				bool dimensions_y_or_z_change = (previous.y() != it->y()) || (previous.z() != it->z());
				EXPECT_TRUE(dimensions_y_or_z_change) << "(" << previous.y() << " != " << it->y() << ") || (" << previous.z() << " != " << it->z() << ")";
				EXPECT_TRUE(dimensions_y_or_z_change) << " this should be a straight line. Find out why parent node is being replaced.";
				previous = *it;
				++it;
			}
		}
		ASSERT_EQ(0, ThetaStarNode::OustandingObjects());
	}

	TEST(LazyThetaStarTests, LazyThetaStar_CoreDumped_Test)
	{
		int max_time_secs= 120;
		double safety_margin = 3;
		octomap::OcTree octree ("data/from_-0.49_0.089_1.9_to_-8.5_-12_4.5.bt");
		octomath::Vector3 disc_initial(-0.488766, 0.0890607, 1.91693);
		octomath::Vector3 disc_final  (-8.5, -12.5, 4.5);
		testStraightLinesForwardNoObstacles(octree, disc_initial, disc_final, max_time_secs, safety_margin);
	}
}

int main(int argc, char **argv){
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}