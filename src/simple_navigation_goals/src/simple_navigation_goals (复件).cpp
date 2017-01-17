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
const int point_length=10;
const int point_number = 4;
//int   point[point_length][point_length];
std::string point[point_length][point_length];
double pose_point[point_length][point_length];
std::string line;


void mySigintHandler(int sig)
{
    ros::shutdown();
}

void VoiceCallBack(const std_msgs::Int32::ConstPtr& goals)
{

    int a=0;
    std::ifstream point_file("/home/gqy/Desktop/simple_navigation_goals_readtxt/point_file.txt");
    //////////////////////The file is saved from phone app.//////////////////////////////////////////////////////
    //////Every line is  pose.position.x  pose.position.y  pose.orientation.z  pose.orientation.w////////////////////////
    //////Read method is word by word,space is end symbol.///////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////If read failed......////////////////////////////////////////////////////
    if(!point_file)
    {
        ROS_INFO("hello2********************************");
        ROS_ERROR("Error reading point_file.txt");
    }

    ///////////////////////////////Save number to 2D array.///////////////////////////////////////////////////
    do{
        for(int b=0;b<point_number;b++)
        {
            point_file >> point[a][b];
            pose_point[a][b] = std::atof(point[a][b].c_str());///////////////////////////string to double//////////////////////////////////////////
//           ROS_INFO("point[%d][%d]=",a,b);
//           std::cout << point[a][b]<<std::endl;

        }
          a++;
    }
    while(std::getline(point_file,line));////////////////////////read next line,and whether is it the last line.////////////////////////////////////

/*
   a--;
   ROS_INFO("a=%d\n",a);
   int counter =0;
   for(int i =0;i<a;i++)
   {
       for(int j=0;j<4;j++)
       {
           ROS_WARN("pose_point[%d][%d]=",i,j);
           std::cout << pose_point[i][j]<<"\t";
           counter=counter+1;
           if(counter % 4==0)
                std::cout << "the next line........";
       }
   }
*/


///////////////////////// If voicenumber received the same number or beyond the max range of 2d array ,returned///////////////////////////////////////
    if( (voicenumber == goals->data) || (goals->data > point_length-1) )
    {
        return;
    }

    ROS_INFO("received data %d",goals->data);


    MoveBaseClient ac("move_base",true);
    while(!ac.waitForServer(ros::Duration(10.0)))
    {
        ROS_INFO("Waiting for the move_base action server to come up");
    }

    ////////////////////////////////////////////Send goals/////////////////////////////////////////////////////////////////////

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

   ROS_INFO("pose_point[%d][%d]=%f\n pose_point[%d][%d]=%f\n pose_point[%d][%d]=%f\n pose_point[%d][%d]=%f\n ",x,y,pose_point[x][y],x,y+1,pose_point[x][y+1],x,y+2,pose_point[x][y+2],x,y+3,pose_point[x][y+3]);
/*
    for(int count=0;count<4;count++)
    {
        std::printf("send_goal_point[%d][%d]",x,count);
        ROS_INFO("hello5********************************");
    }
*/
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
