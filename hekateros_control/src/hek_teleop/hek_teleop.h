////////////////////////////////////////////////////////////////////////////////
//
// Package:   RoadNarrows Robotics Hekateros Robotic Manipulator ROS Package
//
// Link:      https://github.com/roadnarrows-robotics/hekateros
//
// ROS Node:  hek_teleop
//
// File:      hek_teleop.h
//
/*! \file
 *
 * $LastChangedDate$
 * $Rev$
 *
 * \brief The ROS hek_teleop node class interface.
 *
 * \author Daniel Packard (daniel@roadnarrows.com)
 * \author Robin Knight (robin.knight@roadnarrows.com)
 *
 * \par Copyright:
 * (C) 2013-2014  RoadNarrows
 * (http://www.roadnarrows.com)
 * \n All Rights Reserved
 */
/*
 * @EulaBegin@
 * 
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that
 * (1) The above copyright notice and the following two paragraphs
 * appear in all copies of the source code and (2) redistributions
 * including binaries reproduces these notices in the supporting
 * documentation.   Substantial modifications to this software may be
 * copyrighted by their authors and need not follow the licensing terms
 * described here, provided that the new terms are clearly indicated in
 * all files where they apply.
 * 
 * IN NO EVENT SHALL THE AUTHOR, ROADNARROWS LLC, OR ANY MEMBERS/EMPLOYEES
 * OF ROADNARROW LLC OR DISTRIBUTORS OF THIS SOFTWARE BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHORS OR ANY OF THE ABOVE PARTIES HAVE BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND ROADNARROWS LLC SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN
 * "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 * 
 * @EulaEnd@
 */
////////////////////////////////////////////////////////////////////////////////

#ifndef _HEK_TELEOP_H
#define _HEK_TELEOP_H

//
// System
//
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>

//
// Boost
//
#include <boost/bind.hpp>
#include "boost/assign.hpp"

//
// ROS
//
#include "ros/ros.h"
#include "actionlib/server/simple_action_server.h"

//
// ROS generated core, industrial, and hekateros messages.
//
#include "trajectory_msgs/JointTrajectory.h"
#include "sensor_msgs/JointState.h"
#include "industrial_msgs/RobotStatus.h"
#include "hekateros_control/HekJointStateExtended.h"
#include "hekateros_control/HekRobotStatusExtended.h"

//
// ROS generatated hekateros services.
//
#include "hekateros_control/Calibrate.h"
#include "hekateros_control/ClearAlarms.h"
#include "hekateros_control/CloseGripper.h"
#include "hekateros_control/EStop.h"
#include "hekateros_control/Freeze.h"
#include "hekateros_control/GetProductInfo.h"
#include "hekateros_control/GotoBalancedPos.h"
#include "hekateros_control/GotoParkedPos.h"
#include "hekateros_control/GotoZeroPt.h"
#include "hekateros_control/IsAlarmed.h"
#include "hekateros_control/IsCalibrated.h"
#include "hekateros_control/IsDescLoaded.h"
#include "hekateros_control/OpenGripper.h"
#include "hekateros_control/Release.h"
#include "hekateros_control/ResetEStop.h"
#include "hekateros_control/SetRobotMode.h"
#include "hekateros_control/Stop.h"

//
// ROS generated action servers.
//

//
// ROS generated HID messages.
//
#include "hid/ConnStatus.h"           // subscribe
#include "hid/Controller360State.h"   // subscribe
#include "hid/LEDPattern.h"           // service
#include "hid/RumbleCmd.h"            // publish

//
// ROS generatated HID services.
//
#include "hid/SetLED.h"
#include "hid/SetRumble.h"

//
// RoadNarrows
//
#include "rnr/rnrconfig.h"
#include "rnr/log.h"
#include "rnr/hid/HIDXbox360.h"

//
// RoadNarrows embedded hekateros library.
//
#include "Hekateros/hekateros.h"
#include "Hekateros/hekXmlCfg.h"
#include "Hekateros/hekRobot.h"
#include "Hekateros/hekUtils.h"


namespace hekateros_control
{
  /*!
   * \brief The class embodiment of the hek_teleop ROS node.
   */
  class HekTeleop
  {
  public:
    /*! map of ROS server services type */
    typedef std::map<std::string, ros::ServiceServer> MapServices;

    /*! map of ROS client services type */
    typedef std::map<std::string, ros::ServiceClient> MapClientServices;

    /*! map of ROS publishers type */
    typedef std::map<std::string, ros::Publisher> MapPublishers;

    /*! map of ROS subscriptions type */
    typedef std::map<std::string, ros::Subscriber> MapSubscriptions;

    /*! map of joint names to link indices */
    typedef std::map<std::string, int> MapJoints;

    /*!
     * \brief Teleoperation state.
     */
    enum TeleopState
    {
      TeleopStateUninit,    ///< not initialized
      TeleopStatePaused,    ///< paused
      TeleopStateReady      ///< ready and running
    };

    /*!
     * \brief Teleoperation move modes.
     */
    enum TeleopMode
    {
      TeleopModeFirstPerson,  ///< first person, multi-joint move mode
      TeleopModeShoulder,     ///< move shoulder in isolation
      TeleopModeElbow         ///< move elbow in isolation
    };

    /*!
     * \brief Xbox360 button map ids.
     */
    enum ButtonId
    {
      ButtonIdGotoBalPos    = rnr::Xbox360FeatIdAButton,  ///< goto balance pos
      ButtonIdEStop         = rnr::Xbox360FeatIdBButton,  ///< emergency stop
      ButtonIdGotoParkedPos = rnr::Xbox360FeatIdXButton,  ///< goto parked pos
      ButtonIdGotoZeroPt    = rnr::Xbox360FeatIdYButton,  ///< goto zero point

      ButtonIdPause         = rnr::Xbox360FeatIdBack,     ///< pause teleop
      ButtonIdToggleMode    = rnr::Xbox360FeatIdCenterX,  ///< toggle op mode
      ButtonIdStart         = rnr::Xbox360FeatIdStart,    ///< start teleop

      ButtonIdPrevJoint     = rnr::Xbox360FeatIdPadLeft,  ///< previous joint
      ButtonIdNextJoint     = rnr::Xbox360FeatIdPadRight, ///< next joint

      ButtonIdMoveJoints    = rnr::Xbox360FeatIdLeftJoyY, ///< move fp/shldr
      ButtonIdRotBase       = rnr::Xbox360FeatIdRightJoyX,///< rotate base
      ButtonIdPitchWrist    = rnr::Xbox360FeatIdRightJoyY,///< pitch wrist

      ButtonIdRotWristCw    = rnr::Xbox360FeatIdLeftBump, ///< rotate wrist CW
      ButtonIdRotWristCcw   = rnr::Xbox360FeatIdRightBump,///< rotate wrist CCW

      ButtonIdOpenGripper   = rnr::Xbox360FeatIdLeftTrigger, ///< open gripper
      ButtonIdCloseGripper  = rnr::Xbox360FeatIdRightTrigger ///< close gripper
    };

    /*! teleop button state type */
    typedef std::map<int, int> ButtonState;

    /*!
     * \brief Xbox360 LED patterns.
     */
    enum LEDPat
    {
      LEDPatOff       = XBOX360_LED_PAT_ALL_OFF,    ///< all off
      LEDPatOn        = XBOX360_LED_PAT_ALL_BLINK,  ///< default xbox on pattern
      LEDPatPaused    = XBOX360_LED_PAT_4_ON,       ///< pause teleop pattern
      LEDPatReady     = XBOX360_LED_PAT_ALL_SPIN,   ///< spin, first-person mode
      LEDPatShoulder  = XBOX360_LED_PAT_1_ON,       ///< isolated shoulder move
      LEDPatElbow     = XBOX360_LED_PAT_2_ON        ///< isolated elbow move
    };

    /*!
     * \brief First person state.
     */
    struct FirstPersonState
    {
      bool    m_bNewGoal;   ///< [do not] have a new goal
      double  m_goalSign;   ///< goal sign
      struct
      {
        double  alpha;      ///< shoulder
        double  beta;       ///< elbow
        double  gamma;      ///< wrist pitch
      } m_goalJoint;        ///< goal in joint angles alpha, beta, gamma
      struct
      {
        double x;           ///< x position is planar space
        double y;           ///< y position is planar space
      } m_goalCart;         ///< goal in cartesian 2D coordinates x, y
    };

    /*!
     * \brief Default initialization constructor.
     *
     * \param nh  Bound node handle.
     * \param hz  Application nominal loop rate in Hertz.
     */
    HekTeleop(ros::NodeHandle &nh, double hz);

    /*!
     * \brief Destructor.
     */
    virtual ~HekTeleop();

    /*!
     * \brief Advertise all server services.
     */
    virtual void advertiseServices()
    {
      // none
    }

    /*!
     * \brief Initialize client services.
     */
    virtual void clientServices();

    /*!
     * \brief Advertise all publishers.
     *
     * \param nQueueDepth   Maximum queue depth.
     */
    virtual void advertisePublishers(int nQueueDepth=10);

    /*!
     * \brief Subscribe to all topics.
     *
     * \param nQueueDepth   Maximum queue depth.
     */
    virtual void subscribeToTopics(int nQueueDepth=10);

    /*!
     * \brief Publish.
     *
     * Call in main loop.
     */
    virtual void publish()
    {
      // No periodic publishing. All event driven.
    }

    /*!
     * \brief Check communications.
     *
     * Call in main loop.
     */
    virtual void commCheck();

    /*!
     * \brief Put robot into safe mode.
     *
     * \param bHard   Harden safe mode. When teleop node dies or xbox is 
     *                physically disconnected, robot is set to known defaults.
     */
    void putRobotInSafeMode(bool bHard);

    /*!
     * \brief Test if robot can move.
     *
     * \return Returns true or false.
     */
    bool canMove()
    {
      if((m_eState == TeleopStateReady) &&
         (m_msgRobotStatus.is_calibrated.val ==
                                       industrial_msgs::TriState::TRUE) &&
         (m_msgRobotStatus.e_stopped.val == industrial_msgs::TriState::FALSE) &&
         (m_msgRobotStatus.in_error.val == industrial_msgs::TriState::FALSE))
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    /*!
     * \brief Get bound node handle.
     *
     * \return Node handle.
     */
    ros::NodeHandle &getNodeHandle()
    {
      return m_nh;
    }

    /*!
     * \brief Convert seconds to loop counts.
     *
     * \param seconds Seconds.
     *
     * \return Count.
     */
    int countsPerSecond(double seconds)
    {
      return (int)(seconds * m_hz);
    }

  protected:
    ros::NodeHandle  &m_nh;       ///< the node handler bound to this instance
    double            m_hz;       ///< application nominal loop rate

    // ROS services, publishers, subscriptions.
    MapServices       m_services;       ///< teleop server services
    MapClientServices m_clientServices; ///< teleop client services
    MapPublishers     m_publishers;     ///< teleop publishers
    MapSubscriptions  m_subscriptions;  ///< teleop subscriptions

    // state
    TeleopState       m_eState;         ///< teleoperation state
    TeleopMode        m_eMode;          ///< teleoperation mode
    bool              m_bHasXboxComm;   ///< Xbox communications is [not] good
    int               m_nWdXboxCounter; ///< Xbox watchdog counter
    int               m_nWdXboxTimeout; ///< Xbox watchdog timeout
    bool              m_bHasRobotComm;  ///< robot communications is [not] good
    int               m_nWdRobotCounter;///< robot watchdog counter
    int               m_nWdRobotTimeout;///< robot watchdog timeout
    bool              m_bRcvdRobotStatus; ///< has [not] recieved any bot status
    bool              m_bRcvdJointState; ///< has [not] recieved any joint state
    bool              m_bHasFullComm;   ///< good full communications
    ButtonState       m_buttonState;    ///< saved previous button state
    int               m_rumbleLeft;     ///< saved previous left rumble speed 
    int               m_rumbleRight;    ///< saved previous right rumble speed 
    FirstPersonState  m_fpState;        ///< first person state data
    MapJoints         m_mapJoints;      ///< map of joint names - link indices

    // messages
    hekateros_control::HekRobotStatusExtended m_msgRobotStatus;
                                                    ///< current robot status 
    hekateros_control::HekJointStateExtended m_msgJointState;
                                                    ///< current joint state
    trajectory_msgs::JointTrajectory m_msgJointTraj;
                                                    ///< working joint traj.
    trajectory_msgs::JointTrajectoryPoint m_msgJointTrajPoint;
                                                    ///< working joint traj. pt
    hid::ConnStatus m_msgConnStatus;                ///< saved last conn status 


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Server Service callbacks
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    // none


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Client Services
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    /*!
     * \brief Set Xbox360 LED pattern client service.
     *
     * \param pattern   LED pattern.
     */
    void setLED(int pattern);

    /*!
     * \brief Set Xbox360 rumble motors client service.
     *
     * \param motorLeft   Left motor speed.
     * \param motorRight  Right motor speed.
     */
    void setRumble(int motorLeft, int motorRight);

    /*!
     * \brief Close robot gripper client service.
     */
    void closeGripper();

    /*!
     * \brief Emergency stop robot client service.
     */
    void estop();

    /*!
     * \brief Freeze (stop) robot client service.
     */
    void freeze();

    /*!
     * \brief Move robot to balanced position client service.
     */
    void gotoBalancedPos();

    /*!
     * \brief Move robot to parked position client service.
     */
    void gotoParkedPos();

    /*!
     * \brief Move robot to zero point position client service.
     */
    void gotoZeroPt();

    /*!
     * \brief Reset emergency stop client service.
     */
    void resetEStop();

    /*!
     * \brief Set robot mode client service.
     *
     * \param mode    Auto or manual mode.
     */
    void setRobotMode(hekateros::HekRobotMode mode);


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Topic Publishers
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    /*!
     * \brief Publish brake command.
     */
    void publishJointCmd();

    /*!
     * \brief Publish Xbox360 rumble command.
     */
    void publishRumbleCmd(int motorLeft, int motorRight);


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Subscribed Topic Callbacks
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    /*!
     * \brief Robot status callback.
     *
     * \param msg Received subscribed topic.
     */
    void cbRobotStatus(const hekateros_control::HekRobotStatusExtended &msg);

    /*!
     * \brief Joint state callback.
     *
     * \param msg Received subscribed topic.
     */
    void cbJointState(const hekateros_control::HekJointStateExtended &msg);
    
    /*!
     * \brief Xbox360 HID connectivity status callback.
     *
     * \param msg Received subscribed topic.
     */
    void cbXboxConnStatus(const hid::ConnStatus &msg);

    /*!
     * \brief Xbox360 HID button state callback.
     *
     * \param msg Received subscribed topic.
     */
    void cbXboxBttnState(const hid::Controller360State &msg);


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Xbox Actions
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    /*!
     * \brief Convert Xbox360 controller message to button state.
     *
     * \param msg           Message.
     * \param buttonState   New button state.
     */
    void msgToState(const hid::Controller360State &msg,
                    ButtonState                   &buttonState);

    /*!
     * \brief Test if button transitions from off to on.
     *
     * \param id            Button id.
     * \param buttonState   New button state.
     *
     * \return Returns true or false.
     */
    bool buttonOffToOn(int id, ButtonState &buttonState)
    {
      return (m_buttonState[id] == 0) && (buttonState[id] == 1);
    }

    /*!
     * \brief Test if button state is differenet.
     *
     * \param id            Button id.
     * \param buttonState   New button state.
     *
     * \return Returns true or false.
     */
    bool buttonDiff(int id, ButtonState &buttonState)
    {
      return m_buttonState[id] != buttonState[id];
    }

    /*!
     * \brief Execute all button actions.
     *
     * \param buttonState   New button state.
     */
    void execAllButtonActions(ButtonState &buttonState);

    /*!
     * \brief Execute start button action.
     *
     * \param buttonState   New button state.
     */
    void buttonStart(ButtonState &buttonState);

    /*!
     * \brief Execute pause button action.
     *
     * \param buttonState   New button state.
     */
    void buttonPause(ButtonState &buttonState);

    /*!
     * \brief Execute toggle mode button action.
     *
     * \param buttonState   New button state.
     */
    void buttonToggleMode(ButtonState &buttonState);

    /*!
     * \brief Execute shift to previous joint button action.
     *
     * \param buttonState   New button state.
     */
    void buttonPrevJoint(ButtonState &buttonState);

    /*!
     * \brief Execute shift to next joint button action.
     *
     * \param buttonState   New button state.
     */
    void buttonNextJoint(ButtonState &buttonState);

    /*!
     * \brief Execute emergency stop button action.
     *
     * \param buttonState   New button state.
     */
    void buttonEStop(ButtonState &buttonState);

    /*!
     * \brief Execute move to balanced position button action.
     *
     * \param buttonState   New button state.
     */
    void buttonGotoBalancedPos(ButtonState &buttonState)
    {
      if( buttonOffToOn(ButtonIdGotoBalPos, buttonState) )
      {
        gotoBalancedPos();
        m_fpState.m_bNewGoal = true;
      }
    }

    /*!
     * \brief Execute move to parked position button action.
     *
     * \param buttonState   New button state.
     */
    void buttonGotoParkedPos(ButtonState &buttonState)
    {
      if( buttonOffToOn(ButtonIdGotoParkedPos, buttonState) )
      {
        gotoParkedPos();
        m_fpState.m_bNewGoal = true;
      }
    }

    /*!
     * \brief Execute move to zero point button action.
     *
     * \param buttonState   New button state.
     */
    void buttonGotoZeroPt(ButtonState &buttonState)
    {
      if( buttonOffToOn(ButtonIdGotoZeroPt, buttonState) )
      {
        gotoZeroPt();
        m_fpState.m_bNewGoal = true;
      }
    }

    /*!
     * \brief Execute close gripper button action.
     *
     * \param buttonState   New button state.
     */
    void buttonCloseGripper(ButtonState &buttonState);

    /*!
     * \brief Execute open gripper button action.
     *
     * \param buttonState   New button state.
     */
    void buttonOpenGripper(ButtonState &buttonState);

    /*!
     * \brief Execute (multi-)joint move button action.
     *
     * \param buttonState   New button state.
     */
    void buttonMoveJoints(ButtonState &buttonState);

    /*!
     * \brief Execute first person multi-joint move button action.
     *
     * \param joy   New joystick value.
     */
    void buttonMoveFirstPerson(int joy);

    /*!
     * \brief Execute shoulder joint move button action.
     *
     * \param joy   New joystick value.
     */
    void buttonMoveShoulder(int joy);

    /*!
     * \brief Execute elbow joint move button action.
     *
     * \param joy   New joystick value.
     */
    void buttonMoveElbow(int joy);
    
    /*!
     * \brief Execute rotation base button action.
     *
     * \param buttonState   New button state.
     */
    void buttonRotateBase(ButtonState &buttonState);

    /*!
     * \brief Execute pitch wrist base button action.
     *
     * \param buttonState   New button state.
     */
    void buttonPitchWrist(ButtonState &buttonState);

    /*!
     * \brief Execute rotation wrist clockwise button action.
     *
     * \param buttonState   New button state.
     */
    void buttonRotateWristCw(ButtonState &buttonState);

    /*!
     * \brief Execute rotation wrist counter-clockwise button action.
     *
     * \param buttonState   New button state.
     */
    void buttonRotateWristCcw(ButtonState &buttonState);


    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
    // Support 
    //. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    /*!
     * \brief Go to pause teleoperation state.
     */
    void pause();

    /*!
     * \brief Go to ready to teleoperate state.
     */
    void ready();

    /*!
     * \brief Drive Xbox360 LEDs into a figure 8 pattern.
     */
    void driveLEDsFigure8Pattern();

    /*!
     * \brief Get the index of the joint of the robot.
     *
     * \param strJointName    Joint name.
     * 
     * \return Returns index \h_ge 0 if found. Otherwise returns -1.
     */
    ssize_t indexOfRobotJoint(const std::string &strJointName);

    /*!
     * \brief Get the index of the joint in the current joint trajectory.
     *
     * \param strJointName    Joint name.
     * 
     * \return Returns index \h_ge 0 if found. Otherwise returns -1.
     */
    ssize_t indexOfTrajectoryJoint(const std::string &strJointName);
    
    /*!
     * \brief Add a joint to the current joint trajectory point.
     *
     *  The joint is only added if it does not exist in point.
     *
     * \param strJointName    Joint name.
     * 
     * \return Returns index \h_ge 0 of the added or existing joint.
     * If the joint does not exists in the arm, -1 is returned.
     */
    ssize_t addJointToTrajectoryPoint(const std::string &strJointName);

    /*
     * \brief Set null joint trajectory.
     *
     * A null trajectory is a trajectory to the current joint position at
     * zero velocity.
     */
    void nullJointTrajectory();

    /*!
     * \brief Clear joint trajectory.
     */
    void clearJointTrajectory();
    
  };

} // namespace hekateros_control


#endif // _HEK_TELEOP_H