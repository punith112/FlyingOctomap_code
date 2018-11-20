#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <nav_msgs/Path.h>
#include <ros/ros.h>
#include <fstream>

geometry_msgs::PoseStamped uav_pose, target_pose, target_pose_prev;
geometry_msgs::TwistStamped uav_velocity;
nav_msgs::Path uav_path_actual, uav_path_intendeed;
std::vector<double> poseListX, poseListY, poseListZ;
int init_target = 0;
bool flag = false;

void pose_cb(const geometry_msgs::PoseStamped::ConstPtr &msg) {
    uav_pose = *msg;
    uav_path_actual.header.frame_id = "map";
    uav_path_actual.poses.push_back(*msg);
}

void velocity_cb(const geometry_msgs::TwistStamped::ConstPtr &msg) {
    uav_velocity = *msg;
    uav_velocity.header.frame_id = "base_link";
}

void target_pose_cb(const geometry_msgs::PoseStamped::ConstPtr &msg) {
    target_pose = *msg;
    if (init_target == 0) {
        target_pose_prev = *msg;
        init_target++;
        uav_path_intendeed.header.frame_id = "map";
        target_pose.pose.position.x = 0;
        target_pose.pose.position.y = 0;
        target_pose.pose.position.z = 2;
        target_pose.pose.orientation.x = 0;
        target_pose.pose.orientation.y = 0;
        target_pose.pose.orientation.z = 0;
        target_pose.pose.orientation.w = 0;
        uav_path_intendeed.poses.push_back(target_pose);
        uav_path_intendeed.poses.push_back(*msg);
    } else {
        if (target_pose.pose.position.x != target_pose_prev.pose.position.x ||
            target_pose.pose.position.y != target_pose_prev.pose.position.y ||
            target_pose.pose.position.z != target_pose_prev.pose.position.z ||
            target_pose.pose.orientation.x != target_pose_prev.pose.orientation.x ||
            target_pose.pose.orientation.y != target_pose_prev.pose.orientation.y ||
            target_pose.pose.orientation.z != target_pose_prev.pose.orientation.z ||
            target_pose.pose.orientation.w != target_pose_prev.pose.orientation.w) {
            target_pose_prev = target_pose;
            flag = true;
            uav_path_intendeed.header.frame_id = "map";
            uav_path_intendeed.poses.push_back(target_pose);
        } else {
            flag = false;
        }
    }
}

double calculateDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
    return sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2) + pow((z1 - z2), 2));
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "save_poses");
    ros::NodeHandle nh;

    ros::Subscriber pose_sub = nh.subscribe<geometry_msgs::PoseStamped>("mavros/local_position/pose", 10, pose_cb);
    ros::Subscriber velocity_sub = nh.subscribe<geometry_msgs::TwistStamped>("mavros/local_position/velocity", 10, velocity_cb);
    ros::Subscriber target_pose_sub = nh.subscribe<geometry_msgs::PoseStamped>("target_wp", 10, target_pose_cb);

    ros::Publisher pub_uav_path_actual = nh.advertise<nav_msgs::Path>("uav_path_actual", 1);
    ros::Publisher pub_uav_path_intendeed = nh.advertise<nav_msgs::Path>("uav_path_intendeed", 1);
    ros::Publisher pub_uav_velocity = nh.advertise<geometry_msgs::TwistStamped>("uav_velocity", 1);

    char *envvar_home;
    std::stringstream home;
    envvar_home = std::getenv("HOME");
    home << envvar_home << "/matlab_ws/_LazyTheta";

    std::ofstream file_uav_poses, file_uav_poses_times, file_target_wp, file_target_wp_times;
    file_uav_poses.open(home.str() + "/uav_poses");
    file_uav_poses_times.open(home.str() + "/uav_poses_times");
    file_target_wp.open(home.str() + "/target_wp");
    file_target_wp_times.open(home.str() + "/target_wp_times");

    int cont_velocity = 0;
    int cont_start_time = 0;
    double start_time, distance_target;

    ros::Rate rate(10);
    while (ros::ok()) {
        if (cont_start_time == 0) {
            start_time = ros::Time::now().toSec();
            cont_start_time++;
        }

        distance_target = calculateDistance(uav_pose.pose.position.x, uav_pose.pose.position.y, uav_pose.pose.position.z,
                                            target_pose.pose.position.x, target_pose.pose.position.y, target_pose.pose.position.z);

        if (flag == true) {
            // ROS_WARN_STREAM("[save_pose] Going to target");
            file_target_wp << target_pose.pose.position.x << " "
                           << target_pose.pose.position.y << " "
                           << target_pose.pose.position.z << " "
                           << target_pose.pose.orientation.x << " "
                           << target_pose.pose.orientation.y << " "
                           << target_pose.pose.orientation.z << " "
                           << target_pose.pose.orientation.w << "\n";

            file_target_wp_times << ros::Time::now().toSec() - start_time << " ";

            flag = false;
        }

        if (distance_target > 0.2 && cont_velocity != 0) {
            cont_velocity = 0;
        }

        if (distance_target < 0.2 && cont_velocity == 0 && uav_pose.pose.position.z > 0.5) {
            // ROS_WARN_STREAM("[save_pose] Goal!");
            file_target_wp_times << ros::Time::now().toSec() - start_time << " ";
            cont_velocity++;
        }

        file_uav_poses << uav_pose.pose.position.x << " " << uav_pose.pose.position.y << " " << uav_pose.pose.position.z << " "
                       << uav_pose.pose.orientation.x << " " << uav_pose.pose.orientation.y << " " << uav_pose.pose.orientation.z << " "
                       << uav_pose.pose.orientation.w << std::endl;

        file_uav_poses_times << ros::Time::now().toSec() - start_time << std::endl;

        pub_uav_velocity.publish(uav_velocity);
        pub_uav_path_actual.publish(uav_path_actual);
        pub_uav_path_intendeed.publish(uav_path_intendeed);

        ros::spinOnce();
        rate.sleep();
    }
    file_uav_poses.close();
    file_uav_poses_times.close();
    file_target_wp.close();
    file_target_wp_times.close();
}
