//
// Created by Ian Parker on 03/10/2024.
//

#ifndef UFC_COMMANDS_H
#define UFC_COMMANDS_H

#include <string>

namespace UFC
{
const std::string AUTOPILOT_AIRSPEED_UP = "autopilot/airspeed/up";
const std::string AUTOPILOT_AIRSPEED_DOWN = "autopilot/airspeed/down";
const std::string AUTOPILOT_AIRSPEED_MANAGE = "autopilot/airspeed/manage";
const std::string AUTOPILOT_AIRSPEED_GUIDANCE = "autopilot/airspeed/guidance";
const std::string AUTOPILOT_HEADING_UP = "autopilot/heading/up";
const std::string AUTOPILOT_HEADING_DOWN = "autopilot/heading/down";
const std::string AUTOPILOT_HEADING_MANAGE = "autopilot/heading/manage";
const std::string AUTOPILOT_HEADING_GUIDANCE = "autopilot/heading/guidance";
const std::string AUTOPILOT_ALTITUDE_UP = "autopilot/altitude/up";
const std::string AUTOPILOT_ALTITUDE_DOWN = "autopilot/altitude/down";
const std::string AUTOPILOT_ALTITUDE_MANAGE = "autopilot/altitude/manage";
const std::string AUTOPILOT_ALTITUDE_GUIDANCE = "autopilot/altitude/guidance";
const std::string AUTOPILOT_ALTITUDE_STEP_100 = "autopilot/altitude/step/100";
const std::string AUTOPILOT_ALTITUDE_STEP_1000 = "autopilot/altitude/step/1000";
const std::string AUTOPILOT_VERTICAL_SPEED_UP = "autopilot/verticalSpeed/up";
const std::string AUTOPILOT_VERTICAL_SPEED_DOWN = "autopilot/verticalSpeed/down";
const std::string AUTOPILOT_VERTICAL_SPEED_MANAGE = "autopilot/verticalSpeed/manage";
const std::string AUTOPILOT_VERTICAL_SPEED_GUIDANCE = "autopilot/verticalSpeed/guidance";

const std::string AUTOPILOT_AP1_TOGGLE = "autopilot/ap1/toggle";
const std::string AUTOPILOT_AP2_TOGGLE = "autopilot/ap2/toggle";
const std::string AUTOPILOT_AUTO_THRUST_TOGGLE = "autopilot/autoThrust/toggle";
const std::string AUTOPILOT_LOCALISER_TOGGLE = "autopilot/localiser/toggle";
const std::string AUTOPILOT_EXPEDITE_TOGGLE = "autopilot/expedite/toggle";
const std::string AUTOPILOT_APPROACH_TOGGLE = "autopilot/approach/toggle";
const std::string AUTOPILOT_SPD_MACH_TOGGLE = "autopilot/spdMach/toggle";
const std::string AUTOPILOT_HDG_TRK_TOGGLE = "autopilot/hdkTrk/toggle";
const std::string AUTOPILOT_VS_FPA_TOGGLE = "autopilot/vsFpa/toggle";

const std::string COMMS_COM1_STANDBY_UP_COARSE = "comms/com1/standby/coarse/up";
const std::string COMMS_COM1_STANDBY_DOWN_COARSE = "comms/com1/standby/coarse/down";
const std::string COMMS_COM1_STANDBY_UP_FINE = "comms/com1/standby/fine/up";
const std::string COMMS_COM1_STANDBY_DOWN_FINE = "comms/com1/standby/fine/down";
const std::string COMMS_COM1_SWAP = "comms/com1/swap";

}

#endif //COMMANDS_H
