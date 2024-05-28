#include <Arduino.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <ESPAsyncWebServer.h>
#include <map>
#include <string>

#include "namedMesh.h"
#include "config.h"
#include "./utils/utils.h"

Scheduler userScheduler;
namedMesh mesh;
std::map<String, String> rooms;
StaticJsonDocument<128> serializedRooms;

AsyncWebServer server(80);
IPAddress getlocalIP();
IPAddress myIP(0, 0, 0, 0);

Task taskSendMessage(TASK_SECOND * 5, TASK_FOREVER, []()
                     {
                       rooms[nodeName] = String(getTemperature());
                       String msg = String(rooms[nodeName]);
                       mesh.sendBroadcast(msg); });

void setup()
{
  Serial.begin(115200);

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  // mesh channel set to 1, it should be the same as station
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1, HIDDEN_NETWORK);

  mesh.setName(nodeName);
  mesh.setHostname(HOSTNAME);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setRoot(IS_ROOT); // only one root
  mesh.setContainsRoot(true);

  mesh.onReceive([](String &from, String &msg)
                 { 
                  String room = from.c_str();
                  String temp = msg.c_str();
                  // Serial.printf("[%s] %s temperature is: %s \n", nodeName, room, temp); 
                  rooms[room] = temp; });

  mesh.onChangedConnections([]()
                            { Serial.printf("[INFORMATION] Network topology has changed\n"); });

  mesh.onNewConnection([](uint32_t nodeId)
                       { Serial.printf("[INFORMATION] New node: %u\n", nodeId); });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  if (IS_ROOT)
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              /*
              {
                [
                  NODE_0 = 6,
                  NODE_1 = 4,
                  NODE_3 = 1
                ]
              }
              */
              for (const auto &[k, v] : rooms)
              {
    
                serializedRooms[k] = v;
                Serial.printf("[DEBUG (%s)] rooms[%s] = %s \n", nodeName, k, v);
              }
              String data;
              serializeJsonPretty(serializedRooms, data);
              serializeJsonPretty(serializedRooms, Serial);
              request->send(200, "application/json", data); });

    server.begin();
  }
}

void loop()
{
  mesh.update();
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}
