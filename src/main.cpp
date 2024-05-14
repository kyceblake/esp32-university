#include <Arduino.h>
#include <map>
#include <string>

#include "namedMesh.h"
#include "config.h"
#include "./utils/utils.h"

Scheduler userScheduler;
namedMesh mesh;
std::map<String, double> rooms;

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, []()
                     {
  rooms[nodeName] = getTemperature();
  String msg = String(rooms[nodeName]);

  mesh.sendBroadcast(msg); });

void setup()
{
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1, HIDDEN_NETWORK);

  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg)
                 { Serial.printf("[%s] %s temperature is: %s", nodeName, from.c_str(), msg.c_str()); });

  mesh.onChangedConnections([]()
                            { Serial.printf("[INFORMATION] Network topology has changed\n"); });

  mesh.onNewConnection([](uint32_t nodeId)
                       { Serial.printf("[INFORMATION] New node: %u\n", nodeId); });

  // mesh.onNodeTimeAdjusted([](int32_t offset)
  //                         { Serial.printf("[%s] Adjusted time %u. Offset = %d\n", nodeName, mesh.getNodeTime(), offset); });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop()
{
  mesh.update();
}