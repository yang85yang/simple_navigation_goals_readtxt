#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <std_msgs/String.h>
#include <nav_msgs/Odometry.h>
#include <sstream>
#include <std_msgs/Int32.h>
#include <math.h>
#include <signal.h>

#include  <fstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#define NAVIGATION_VOICE_BASE 0x30


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
int32_t voicenumber = INFINITY;
std_msgs::Int32 msg;
ros::Publisher home_point_pub;
ros::Publisher voice_reply_pub;
const int point_length=10;//数组长度
const int point_number = 4;//txt文件每一行包含这四个元数 goal.pose.position.x  goal.pose.position.position.y  goal.pose.orientation.z  goal.pose.orientation.w
                                               //每个数据用空格隔开,最后一行没有不得有回车符号
std::string point[point_length][point_length];
double pose_point[point_length][point_length];
std::string line;


void mySigintHandler(int sig)
{
    ros::shutdown();
}

void VoiceCallBack(const std_msgs::Int32::ConstPtr& goals)
{

    int a=0,b=0;
//    int file_lines=0;
    std::ifstream point_file("point_file.txt");//读取文件，逐字逐字读


    /////////////////////////////////////////////////////错误的读取文件......////////////////////////////////////////////////////
    if(!point_file)
    {
        ROS_ERROR("Error reading point_file.txt");
    }

    ////////////////////////read next line,and whether is it the last line.判断是否是最后一行，并记录行号，行号从1开始,读取行号////////////////////////////////////
//   while(std::getline(point_file,line)){
//   file_lines++;
//   std::cout<<file_lines<<std::endl;
//   }



   //将从文件读取到的存放到数组中
    do{
        for(b=0;b<point_number;b++)
        {
            point_file >> point[a][b];
//            std::cout << point[a][b];
            //将string类型数组转变成double型数组
            pose_point[a][b] = std::atof(point[a][b].c_str());

        }
          a++;
    }
    while(std::getline(point_file,line));

    ROS_INFO("received data %d,\tfile_lines=%d",goals->data,a-1);
    /////////////////////////////判断是否重复收到命令，或者超出数组长度，或者大于txt文件的最后一行行号/////////////////////////////////////////////////////

    if( (voicenumber == goals->data) || (goals->data > point_length-1) || (goals->data >=a-1) )
    {
        ROS_WARN("goals/voicenumber is error");
        ROS_WARN("The txt fils only is %d lines ",a-1);
        return;
    }

    MoveBaseClient ac("move_base",true);
    while(!ac.waitForServer(ros::Duration(10.0)))
    {
        ROS_INFO("Waiting for the move_base action server to come up");
    }

    ////////////////////////////////////////////Send goals/////////////////////////////////////////////////////////////////////
    //发送目标

   voicenumber = goals->data;
    int32_t x=voicenumber;
    int32_t y=0;

    move_base_msgs::MoveBaseGoal goal;
    goal.target_pose.pose.position.x =pose_point[x][y] ;
    goal.target_pose.pose.position.y = pose_point[x][y+1];
    goal.target_pose.pose.orientation.z =pose_point[x][y+2] ;
    goal.target_pose.pose.orientation.w = pose_point[x][y+3];
    goal.target_pose.header.frame_id = "/map";
    goal.target_pose.header.stamp = ros::Time::now();

   ROS_INFO("pose_point_position_x[%d][%d]=%f",x,y,pose_point[x][y]);
   ROS_INFO("pose_point_position_y[%d][%d]=%f",x,y+1,pose_point[x][y+1]);
   ROS_INFO("pose_point_orientation_z[%d][%d]=%f",x,y+2,pose_point[x][y+2]);
   ROS_INFO("pose_point_orientation_w[%d][%d]=%f",x,y+3,pose_point[x][y+3]);


    ROS_INFO("Sending goal...");
    ac.sendGoal(goal);
    ac.waitForResult();



    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("I got it.Next goal!");
//        msg.data = NAVIGATION_VOICE_BASE + voicenumber;
         msg.data = voicenumber;
         voice_reply_pub.publish(msg);

    }


}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"simple_navigation_goals_publisher",ros::init_options::NoSigintHandler);
    ros::NodeHandle n;
    ROS_INFO("program is running...");
//    home_point_pub=n.advertise<std_msgs::Int32>("home_point",10);
    voice_reply_pub = n.advertise<std_msgs::Int32>("voice_reply",10);
    ros::Subscriber voice_sub=n.subscribe<std_msgs::Int32>("/voice_control",10,VoiceCallBack);
    signal(SIGINT, mySigintHandler);
    ros::spin();
    return 0;
}
