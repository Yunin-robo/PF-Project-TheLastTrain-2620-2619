#include "level.h"
#include "util.h"
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
using namespace std;

using namespace std;
const char* getEntityName(char entitySymbol){
  switch(entitySymbol){
    case ENT_NONE: return "None";
    case ENT_MASTER_KEY: return "Master Key";
    case ENT_ENERGY_PACK: return "Energy Pack";
    case ENT_EMERGENCY_TOOLKIT: return "Emergency Toolkit";
    case ENT_CLUE_4: return "Clue Digit 4";
    case ENT_FUSE: return "Fuse";
    case ENT_CAT: return "Stranded Cat";
    case ENT_CONTROL_PANEL: return "Control Panel Terminal";
    case ENT_LIFT_PASS: return "Lift Pass";
    case ENT_PASSENGER: return "Stranded Passenger";
    case ENT_MEDICAL_KIT: return "Medical Kit";
    case ENT_EMERGENCY_CARD: return "Emergency Access Card";
    case ENT_CLUE_2: return "Clue Digit 2";
    case ENT_CLUE_7: return "Clue Digit 7";
    case ENT_CABINET_TRAIN_KEY: return "Cabinet / Train Key";
    case ENT_CAT_CARRIER: return "Cat Carrier";
    case ENT_OBSERVATION_TOKEN: return "Observation Token";
    default: return "Unknown";}}
    
bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1]){
  (void)horizontalConn;
  if (startFloor < 1 || startFloor > NUM_FLOORS || startRoom < 1 || startRoom > ROOMS_PER_FLOOR){return false;}
  return true;}
  

bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft){
    (void) playerFloor;
    (void) playerRoom;
    (void) inventory;
   energy = INITIAL_ENERGY;
   time = INITIAL_TIME;
   score = INITIAL_SCORE;
   for (int i = 0; i < NUM_OBJECTIVES; i++){objectives[i] = false;}
   inventoryCount = 0;
   emergencyTripsLeft = 0;
   ifstream file(filename);
   if (!file){return false;}
   char line[MAX_LINE_LEN];
   char key[MAX_LINE_LEN];
   char value[MAX_LINE_LEN];
   while (file.getline(line, MAX_LINE_LEN)){
    int p = 0, q = 0;
    while (line[p] != ':' && line[p] != '\0'){
            key[q] = line[p]; p++; q++;}
    if (line[p] == '\0') {continue;}
    key[q] = '\0'; p++; q = 0;
    while (line[p] != '\0'){value[q] = line[p]; p++; q++;}
    value[q] = '\0';
    trimWhitespace(key);
    trimWhitespace(value);
    if (stringEquals(key, "INITIAL_ENERGY")) energy = atoi(value);
    else if (stringEquals(key, "INITIAL_TIME")) time = atoi(value);
    else if (stringEquals(key, "INITIAL_SCORE")) score = atoi(value);
    else if (stringEquals(key, "PASSENGER_RESCUED")) objectives[OBJ_PASSENGER_RESCUED] = stringEquals(value, "YES");
    else if (stringEquals(key, "CAT_RESCUED")) objectives[OBJ_CAT_SECURED] = stringEquals(value, "YES");
    else if (stringEquals(key, "CABINET_UNLOCKED")) objectives[OBJ_CABINET_UNLOCKED] = stringEquals(value, "YES");
    else if (stringEquals(key, "TRAIN_KEY_COLLECTED")) objectives[OBJ_TRAIN_KEY] = stringEquals(value, "YES");
    else if (stringEquals(key, "FUSE_INSTALLED")) objectives[OBJ_FUSE_INSTALLED] = stringEquals(value, "YES");
    else if (stringEquals(key, "LIFT_PASS_COLLECTED")) objectives[OBJ_LIFT_PASS] = stringEquals(value, "YES");
    else if (stringEquals(key, "TRAIN_PREPARED")) objectives[OBJ_TRAIN_PREPARED] = stringEquals(value, "YES");}
    file.close();
    energy = clampInt(energy, 0, MAX_ENERGY);
    time = clampInt(time, 0, MAX_TIME);
    return true;}
    
    
bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
                     
  for (int i = 0; i<NUM_FLOORS; i++){
    for (int j = 0; j< ROOMS_PER_FLOOR-1; j++){
      horizontalConn[i][j] = CONN_NONE;}
    for (int j = 0; j<ROOMS_PER_FLOOR; j++){
      entityCounts[i][j] = 0;
      for (int k = 0; k < MAX_ENTITIES_PER_ROOM; k++){
        roomEntities[i][j][k] = ENT_NONE;}}}
  ifstream file(filename);
  if(!file){return false;}
  char line[MAX_LINE_LEN];
  char entityLine[MAX_LINE_LEN];
  int floorParse = 0;
  while (file.getline(line, MAX_LINE_LEN)){
    int labCol[ROOMS_PER_FLOOR];
    bool found[ROOMS_PER_FLOOR] = {false};
    int floor = 0;
    int len = strlen(line);
    for (int i = 0; i<=len-5;i++){
      if (line[i] == 'F' && line[i+1] >= '0' && line[i+1] <='9' && line[i + 2] == '-' && line[i + 3] == 'R' && line[i + 4] >= '1' && line[i + 4] <= '4'){
        int curr_floor = line[i+1] -'0';
        int room = line[i+4] - '0';
        if (floor == 0){floor = curr_floor;}
        if (floor == curr_floor){ labCol[room-1] = i; found[room-1] = true;}}}
        
    if (floor < 1 || floor> NUM_FLOORS || !found[0] || !found[1] || !found[2] || !found[3]){continue;}
    // horizontal CONN
    for (int i = 0; i< ROOMS_PER_FLOOR-1; i++){
      char d = CONN_NONE;
      for (int j = labCol[i]+5; j< labCol[i+1] && j<len; j++){
        if (line[j]!=' '){d = line[j];}}
      
      if (d == 'D' || d == 'W'|| d == 'L'){
        horizontalConn[floor-1][i] = d;}
      else if (d == '#') {horizontalConn[floor-1][i]= CONN_SOLID_WALL;}
      else {horizontalConn[floor-1][i] = CONN_NONE;}}
    
    // Entity from next line
    
    if (!file.getline(entityLine, MAX_LINE_LEN)){ file.close(); return false;}
    int entitylen = strlen(entityLine);
    for (int i = 0; i<ROOMS_PER_FLOOR; i++){
      int st = labCol[i]-1;
      int en = labCol[i]+9;
      if (st<0){st = 0;}
      if (en > entitylen){en = entitylen-1;}
      
      for (int j = st; j<=en; j++){
        if (entityLine[j]!= ' ' && entityLine[j] != '|'){
          if (entityCounts[floor-1][i] >= MAX_ENTITIES_PER_ROOM){
            file.close(); return false;}
          
          roomEntities[floor-1][i][entityCounts[floor-1][i]] = entityLine[j];
          entityCounts[floor-1][i]++;}}}
      floorParse++;
      if (floorParse == NUM_FLOORS){break;}}
      file.close();
      return floorParse == NUM_FLOORS;}
  

// Modular implementation using primitive arrays for map parsing and verification
