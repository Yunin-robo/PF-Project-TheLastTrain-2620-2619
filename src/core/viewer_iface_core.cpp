#include "viewer_iface.h"
#include "util.h"
#include "config.h"
#include "level.h"
#include <iostream>
using namespace std;

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft){
printHeader("Status");
cout << "Location : F" << playerFloor << "-R" << playerRoom << endl;
cout << "Energy   : " << energy << " / " << MAX_ENERGY << endl;
cout << "Time     : " << time << " / " << MAX_TIME << endl;
cout << "Score    : " << score << endl;
cout << "Trips    : " << emergencyTripsLeft << endl;
cout << "Inventory:  ";
if (inventoryCount ==0){cout << "empty" << endl;}
else {
  for (int i = 0; i<inventoryCount; i++){
    cout << getEntityName(inventory[i]);
    if (i<inventoryCount-1){ cout << ", ";}}
    cout << endl;}}
    
void renderRoomDetails(int floor, int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount){

if (floor < 1 || floor > NUM_FLOORS || room < 1 || room > ROOMS_PER_FLOOR) { cout << "Invalid floor or room." << endl; return;}
cout << "Room F" << floor << "-R" << room << endl;
if (entityCount == 0){cout << "Nothing here." << endl;}
else { for (int i =0;i < entityCount;i++){
        char symbol = roomEntities[floor-1][room-1][i];
        cout << symbol << " " << getEntityName(symbol) << endl;}}}
        
void renderAsciiBuilding(int playerFloor, int playerRoom){
  for (int i=NUM_FLOORS;i>=1; i--){
    for (int j = 1; j<=ROOMS_PER_FLOOR; j++){
      if (i == playerFloor && j== playerRoom){cout << "[ P ]";}
      else {cout << "[ . ]";}}
    cout << " Floor " << i << endl;}}

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES]){
  const char* objectiveNames[NUM_OBJECTIVES] = {"Master Key collected","Clue 4 discovered", "Clue 2 discovered","Clue 7 discovered", "Cabinet unlocked",
                                                "Train Key acquired", "Fuse collected", "Fuse installed", "Lift Pass collected", "Lift 2 active",
                                                "Passenger rescued", "Cat Carrier collected","Cat secured","Train prepared","Mission won","Mission failed"};
  cout << endl;
  printHeader("OBJECTIVES CHECKLIST");
  for (int i = 0; i< NUM_OBJECTIVES ;i++){
    if (objectives[i] == true){cout << "[X] ";}
    else { cout << "[ ] ";}
    cout << objectiveNames[i]<< endl;}
  cout << endl;}


// Terminal UI and dashboard rendering using primitive parameters
