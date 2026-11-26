#include "physics.h"
#include "util.h"
#include "state.h"

void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime){
    energy+=deltaEnergy;
    time+=deltaTime;
    energy = clampInt(energy, 0, MAX_ENERGY);
    time = clampInt(time, 0, MAX_TIME);}
    
void applyScoreDelta(int& score, int deltaScore) {score+= deltaScore;}

bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol){
  for (int i = 0; i<inventoryCount; i++){ if (inventory[i]  == itemSymbol) return true;}
  return false;}
  
bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol){
  if (inventoryCount >= MAX_INVENTORY_CAPACITY) {return false;}
  if (hasItem(inventory, inventoryCount, itemSymbol)) {return false;}
  inventory[inventoryCount] = itemSymbol; inventoryCount++; return true;}
  
bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol){
  int index = -1;
  for (int i = 0; i< inventoryCount; i++){
    if (inventory[i] == itemSymbol) {index = i; break;}}
  if (index == -1) return false;
  for (int i = index; i<inventoryCount-1; i++){
    inventory[i] = inventory[i+1];}
  inventoryCount--;
  return true;}

bool isDirectlyConnected(int fromF, int fromR, int toF, int toR, const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],char& outConnType) {
  if (fromF < 1 || fromF > NUM_FLOORS || toF < 1 || toF > NUM_FLOORS){return false;}
  if (fromR < 1 || fromR > ROOMS_PER_FLOOR || toR < 1 || toR > ROOMS_PER_FLOOR){return false;}
  if (fromF == toF && fromR == toR) {return false;}
  if (fromF == toF){
    if (fromR - toR == 1 || toR - fromR == 1){
      int smaller = (fromR<toR) ? fromR:toR;
      if (toR < smaller){smaller = toR;}
      char d = horizontalConn[fromF-1][smaller-1];
      if (d == CONN_NONE || d == CONN_SOLID_WALL){ return false;}
      outConnType = d; return true;} return false;}
  if (fromR ==1 && toR == 1 && (fromF-toF ==1 || toF-fromF ==1)){
    int lower = fromF;
    if (toF<lower){lower = toF;}
    if (lower==1 || lower == 2 || lower==4){outConnType = CONN_STAIRS; return true;}}
  if ((fromF == 1 || fromF == 2) && (toF == 1 || toF == 2) && (fromR == 2 || fromR == 3) && (toR == 2 || toR == 3)){
    outConnType = CONN_LIFT1; return true;}
  if (fromR == 4 && toR == 4 && (fromF == 2 || fromF == 4 || fromF == 5) && (toF == 2 || toF == 4 || toF == 5)){
    outConnType = CONN_LIFT2; return true;}
  return false;}
  
  
bool canTraverse(int targetFloor, int targetRoom, char connType, 
                const char inventory[MAX_INVENTORY_CAPACITY], 
                int inventoryCount, int emergencyTripsLeft, bool lift2Operational){
                
  (void)targetFloor;
  (void)targetRoom;
  if (connType == CONN_LOCKED_DOOR){return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);}
  if (connType == CONN_LIFT2){
    if (lift2Operational && hasItem(inventory, inventoryCount, ENT_LIFT_PASS)){return true;}
    if (emergencyTripsLeft > 0){return true;}
  return false;}
  if (connType == CONN_DOOR || connType == CONN_WINDOW || connType == CONN_STAIRS || connType == CONN_LIFT1){return true;}
  return false;}
// Movement validation and resource adjustments without structures
