#include "logger.h"
#include "util.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

bool saveGameStateToFile(const char* filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
                         
  ofstream file(filename);
  if(!file){return false;}
  
  file << "<-********** PLAYER STATE **********->" << endl;
  file << "FLOOR: " << playerFloor << endl;
  file << "ROOM: " << playerRoom << endl;
  file << "ENERGY: " << energy << endl;
  file << "TIME: " << time << endl;
  file << "SCORE: " << score << endl;
  file << "EMERGENCY_TRIPS: " << emergencyTripsLeft << endl;
  
  file << endl;
  file << "<-********** INVENTORY **********->" << endl;
  file << "INVENTORY_COUNT: " << inventoryCount << endl;
  file << "INVENTORY: ";
  for (int i = 0; i<inventoryCount; i++){
    file << inventory[i] << " ";}
  file << endl;
  file <<endl;
  
  file << "<-********** OBJECTIVES **********->" << endl;
  file << "OBJECTIVES: ";
  for (int i = 0; i<NUM_OBJECTIVES; i++){
    file << objectives[i] << " ";}
  file << endl;
  file << endl;
  
  file << "<-********** BUILDING ITEMS **********->" << endl;
  for (int i = 0; i<NUM_FLOORS; i++){
    for (int k =0; k<ROOMS_PER_FLOOR; k++){
      file<< "Room Items: " << i+1 << " " << k+1 << " "<< entityCounts[i][k];
      for (int j  = 0; j<entityCounts[i][k]; j++){
        file << " " << roomEntities[i][k][j];}
        file << endl;}}
  file << endl;
  
  file << "<-********** END **********->" << endl;  
  file.flush();
  if (file.fail()){ file.close(); return false;}
  file.close();
  return true;}
  
bool loadGameStateFromFile(const char* filename,
                           int& playerFloor, int& playerRoom,
                           int& energy, int& time, int& score,
                           char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                           bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

    ifstream file(filename);
    if (!file) return false;

    string line, key, value;

    for (int i = 0; i < NUM_FLOORS; i++) {
      for (int k = 0; k < ROOMS_PER_FLOOR; k++) {
        entityCounts[i][k] = 0;}}

    while (getline(file, line)) {
      int pos = line.find(':');
      if (pos == -1) continue;

      key = "";
      value = "";

      for (int i = 0; i < pos; i++) key += line[i];
      for (size_t i = pos + 1; i < line.length(); i++) value += line[i];

      if (key == "FLOOR") playerFloor = atoi(value.c_str());
      else if (key == "ROOM") playerRoom = atoi(value.c_str());
      else if (key == "ENERGY") energy = atoi(value.c_str());
      else if (key == "TIME") time = atoi(value.c_str());
      else if (key == "SCORE") score = atoi(value.c_str());
      else if (key == "EMERGENCY_TRIPS") emergencyTripsLeft = atoi(value.c_str());
      else if (key == "INVENTORY_COUNT") inventoryCount = atoi(value.c_str());

      else if (key == "INVENTORY") {
        int j = 0;
        for (size_t i = 0; i < value.length(); i++) {
          if (value[i] != ' ' && value[i] != '\t') {
            if (j >= inventoryCount) return false;
            inventory[j++] = value[i];}}
        if (j != inventoryCount) return false;}
      else if (key == "OBJECTIVES") {
        int j = 0;
        for (size_t i = 0; i < value.length(); i++) {
          if (value[i] == '0' || value[i] == '1') {
            if (j >= NUM_OBJECTIVES) return false;
            objectives[j++] = (value[i] == '1');}
          else if (value[i] != ' ' && value[i] != '\t') return false;}
        if (j != NUM_OBJECTIVES) return false;}

      else if (key == "Room Items") {
        int f = 0, r = 0, count = 0, j = 0;
        size_t i = 0;
        while (i < value.length() && value[i] == ' ') i++;
        while (i < value.length() && value[i] >= '0' && value[i] <= '9') f = f * 10 + value[i++] - '0';
        while (i < value.length() && value[i] == ' ') i++;
        while (i < value.length() && value[i] >= '0' && value[i] <= '9') r = r * 10 + value[i++] - '0';
        while (i < value.length() && value[i] == ' ') i++;
        while (i < value.length() && value[i] >= '0' && value[i] <= '9') count = count * 10 + value[i++] - '0';

        if (f < 1 || f > NUM_FLOORS || r < 1 || r > ROOMS_PER_FLOOR || count > MAX_ENTITIES_PER_ROOM) return false;

        while (i < value.length()) {
          if (value[i] != ' ') roomEntities[f-1][r-1][j++] = value[i];
          i++;}
        if (j != count) return false;
        entityCounts[f-1][r-1] = count;}}

    file.close();
    if (playerFloor < 1 || playerFloor > NUM_FLOORS ||
        playerRoom < 1 || playerRoom > ROOMS_PER_FLOOR ||
        energy < 0 || energy > MAX_ENERGY ||
        time < 0 || time > MAX_TIME ||
        inventoryCount < 0 || inventoryCount > MAX_INVENTORY_CAPACITY) return false;
    return true;}
  
void writeGameSummaryReport(const char* filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES], bool victory){

    ofstream file(filename);

    if (!file) {cout << "Error opening report file!" << endl; return;}
    if (victory) file << "RESULT: VICTORY" << endl;
    else file << "RESULT: DEFEAT" << endl;

    file << "Final Score: " << score << endl;
    file << "Remaining Energy: " << remainingEnergy << endl;
    file << "Remaining Time: " << remainingTime << endl;

    file << endl << "OBJECTIVES CHECKLIST" << endl;

    const char* names[] = {"Master Key","Clue 4","Clue 2","Clue 7","Cabinet","Train Key","Fuse","Lift Pass","Passenger","Cat","Train Prepared"};

    int obj[] = {OBJ_MASTER_KEY,OBJ_CLUE_4,OBJ_CLUE_2,OBJ_CLUE_7,OBJ_CABINET_UNLOCKED,OBJ_TRAIN_KEY,OBJ_FUSE_INSTALLED,OBJ_LIFT_PASS,OBJ_PASSENGER_RESCUED,OBJ_CAT_SECURED,OBJ_TRAIN_PREPARED};

    for (int i = 0; i < 11; i++) {
        file << "[";
        if (objectives[obj[i]]) file << "X";
        else file << " ";
        file << "] " << names[i] << endl;}
  file.close();}

// File persistence for savegame.txt and game_summary.txt using primitive variables
