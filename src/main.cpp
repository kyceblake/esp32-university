#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <string>

#include "namedMesh.h"
#include "config.h"
#include "./utils/utils.h"

Scheduler userScheduler;
namedMesh mesh;
std::map<String, String> rooms;

AsyncWebServer server(80);
IPAddress myAPIP(0, 0, 0, 0);

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, []()
                     {
  rooms[nodeName] = String(getTemperature());
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
                 { 
                  String room = from.c_str();
                  String temp = msg.c_str();
                  String ip = mesh.getAPIP().toString();
                  
                  Serial.printf("[%s (%s)] %s temperature is: %s \n", nodeName, ip, room, temp); 
                  
                  rooms[room] = temp; });

  mesh.onChangedConnections([]()
                            { Serial.printf("[INFORMATION] Network topology has changed\n"); });

  mesh.onNewConnection([](uint32_t nodeId)
                       { Serial.printf("[INFORMATION] New node: %u\n", nodeId); });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hello, world"); });

  server.begin();
}

void loop()
{
  mesh.update();
}