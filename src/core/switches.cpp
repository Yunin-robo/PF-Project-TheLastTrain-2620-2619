#include "switches.h"
#include "util.h"
#include "physics.h"

bool canUseLift1( int fromFloor, int toFloor){
  if ((fromFloor==1 || fromFloor==2) && (toFloor==2 || toFloor==1) && (fromFloor!=toFloor)){ return true;}
  else return false;}

bool canUseLift2( int fromFloor, int toFloor, bool lift2Operational, int emergencyTripsLeft){
  if ((fromFloor==2 || fromFloor==4 || fromFloor==5) && (toFloor==2 || toFloor==4 || toFloor==5) && (fromFloor!=toFloor)){
    return lift2Operational || emergencyTripsLeft>0;}
  return false;}
  
bool unlockFloor3Door(const char inventory[], int inventoryCount){
  return hasItem(inventory, inventoryCount, ENT_MASTER_KEY);}
  
bool installFuseControlPanel(char inventory[], int& inventoryCount, bool objectives[], int& score){
  if (!hasItem(inventory, inventoryCount, ENT_FUSE)){return false;}
  if (objectives[OBJ_FUSE_INSTALLED]){return false;}
  removeItem(inventory, inventoryCount, ENT_FUSE);
  objectives[OBJ_FUSE_INSTALLED] = true;
  objectives[OBJ_LIFT2_ACTIVE] = true;
  applyScoreDelta(score, SCORE_INSTALL_FUSE);
  return true;}
  
bool verifyCabinetCode(const char* enteredCode, const bool objectives[]){
  if (enteredCode == nullptr){return false;}
  if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7]){return false;}
  return stringEquals(enteredCode, "427");}

// Door lock mechanics, lifts, and cabinet verification without structures
