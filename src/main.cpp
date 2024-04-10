#include <Arduino.h>
#include "namedMesh.h"
#include "config.h"

Scheduler userScheduler;
namedMesh mesh;

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, []() {
  // String msg = String("This is a message from: ") + nodeName + String(" for logNode");
  // String to = "NODE_1";
  // mesh.sendSingle(to, msg);
  String msg = String("Hello everynyan");
  mesh.sendBroadcast(msg);
});

void setup() {
  Serial.begin(115200);

  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1, HIDDEN_NETWORK);

  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("[%s] Message from %s: %s\n", nodeName, from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("[INFORMATION] Network topology has changed\n");
  });

  mesh.onNewConnection([](uint32_t nodeId) {
    Serial.printf("[INFORMATION] New node: %u\n", nodeId);
  });
  
  mesh.onNodeTimeAdjusted([](int32_t offset) {
    Serial.printf("[%s] Adjusted time %u. Offset = %d\n", nodeName, mesh.getNodeTime(), offset);
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  mesh.update();
}
