/*
 * vel_cmd_filter_node.cpp
 *
 *  Created on: Jan 20, 2016
 *      Author: ben
 */

#include <ros/ros.h>
#include "/opt/ros/indigo/include/ros/ros.h"
//#include <j5_msgs/j5StatusMsg.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Twist.h>
#include <string>
#include <vector>

//============================================================================
// Constants
//============================================================================

/// ROS node name
const std::string ComponentName = "vel_cmd_filter_node";

/// ROS topics --> do not change these
const std::string J5CommandTopic = "/j5_cmd";
const std::string J5StatusTopic = "/j5_status";
const std::string MoveBaseCmd = "/cmd_vel";

/// default forward linear velocity command in m/s
const float DefaultVelocityCmd = 0.0;

/// default angular velocity command in rad/s
const float DefaultTurnRateCmd = 0.0;

/// the rate at which the component will publish commands
const int LoopRate = 10; // Hz

/// input value limits
/// these are just here for safety and do not reflect the actual limits of the platform
const double MaxVelocityCommand = 3.0;
const double MaxTurnRateCommand = 1.0;

geometry_msgs::Twist incoming;

void chatterCallback(const geometry_msgs::Twist& velcmd);
geometry_msgs::Twist getCommandMsg();

//============================================================================
// Main
//============================================================================

int main(int argc, char **argv) {

	// get all command line arguments that aren't ROS specific
	// these should be the velocity command parameters

	// initialize ROS
	std::vector<std::string> args;
	geometry_msgs::Twist velcmd;


	ros::init(argc,argv, ComponentName);
	ros::removeROSArgs(argc, argv, args);
	ros::NodeHandle nh;
	ros::Subscriber sub = nh.subscribe(MoveBaseCmd, 1000, chatterCallback);



	//I added these lines......This will subscribe to the /vel_cmd topic and get the messages from there
	/// node handle

	// velocity command publisher
	ros::Publisher velCmdPub = nh.advertise<geometry_msgs::Twist>(J5CommandTopic, 1000);

	// J5 status subscriber
	// ros::Subscriber statusSub = nh.subscribe<j5_msgs::j5StatusMsg>(J5StatusTopic, 1, statusMsgHandler);

	// asynchronous spinner to receive ROS messages
	ros::AsyncSpinner spinner(0);

	// loop rate used to control the frequency of message publication
	ros::Rate loop_rate(LoopRate);

	// set up message command
	geometry_msgs::Twist cmdMsg = getCommandMsg();

	// run loop
	spinner.start();
	while (ros::ok()) {

		// publish velocity command
		ROS_INFO("Sending Velocity Command: {%f, %f}", cmdMsg.linear.x,
				cmdMsg.angular.z);
		if (cmdMsg.linear.x || cmdMsg.angular.z) {
			velCmdPub.publish(cmdMsg);
		}
		ros::spinOnce();
		loop_rate.sleep();
	}

	// shutdown component
	spinner.stop();
	ros::shutdown();

	return 0;
}

//============================================================================
// Functions
//============================================================================

void chatterCallback(const geometry_msgs::Twist& velcmd) {
	ROS_INFO("I heard this velocity: [%f]", velcmd.linear.x);
	ROS_INFO("I heard this rotation: [%f]", velcmd.angular.z);
	incoming = velcmd;
}

//void statusMsgHandler(const j5_msgs::j5StatusMsg::ConstPtr& msg){

//	ROS_INFO("RCV Status: EXT_CONTROL: %i FAULT: %i CONTACTORS: %i VOLTAGE: %.1f", msg->externalControl, msg->fault, msg->contactors, msg->voltage);
//}

/**
 * Get Command Message
 * Returns the command message given the input arguments.  If the function
 * fails to parse the input, default parameters are used
 */
geometry_msgs::Twist getCommandMsg() {

	geometry_msgs::Twist msg;

	// body coordinates
	// x is forward linear motion
	msg.linear.x = DefaultVelocityCmd;

	// rotation around the Z axis
	msg.angular.z = DefaultTurnRateCmd;

	// all other parameters are ignored in the Twist message
	// NOTE:: args[0] is the name of the
	if (incoming.linear.x) {
		try {
			msg.linear.x = incoming.linear.x;
			msg.linear.x = std::max(msg.linear.x, -MaxVelocityCommand);
			msg.linear.x = std::min(msg.linear.x, MaxVelocityCommand);
		}

		catch (...) {
			// could not parse the input, use default value
			msg.linear.x = DefaultVelocityCmd;
		}
	}

	if (incoming.angular.z) {
		try {
			msg.angular.z = incoming.angular.z;
			msg.angular.z = std::max(msg.angular.z, -MaxTurnRateCommand);
			msg.angular.z = std::min(msg.angular.z, MaxTurnRateCommand);
		}

		catch (...) {
			// could not parse the input, use default value
			msg.angular.z = DefaultTurnRateCmd;
		}
	}
	return msg;
}


