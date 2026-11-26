#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"
#include "config.h"
#include "state.h"
#include "level.h"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;


bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES]){
  if (objectives[OBJ_MISSION_WON]){return true;}
  if (energy<=0 || time <=0){return true;}
  return false;}

bool handleMoveCommand(int targetFloor, int targetRoom,
                       int& currentFloor, int& currentRoom,
                       int& energy, int& time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                       int& emergencyTripsLeft, bool lift2Operational){
                       

  char conntype;
  if (!isDirectlyConnected(currentFloor, currentRoom, targetFloor, targetRoom, horizontalConn,conntype)){
    cout << "[!] No direct route to F" << targetFloor << "-R" << targetRoom << "." << endl; return false;}
  
  if (!canTraverse(targetFloor, targetRoom, conntype, inventory, inventoryCount, emergencyTripsLeft,lift2Operational)){
    if (conntype == 'L'){
      cout << "[!] Door is locked. You need the Master Key." << endl;}
    else if (conntype == '2'){
      cout << "[!] Lift 2 is offline. You need a Lift Pass or emergency trips." << endl;}
    return false;}
  
  int engcost =0;
  int engtime = 0;
  if (conntype=='D' || conntype == 'L' || conntype == 'S'){
    engcost = COST_MOVE_ORDINARY_ENERGY;
    engtime = COST_MOVE_ORDINARY_TIME;}
  else if (conntype == 'W'){
    engcost = COST_MOVE_WINDOW_ENERGY;
    engtime = COST_MOVE_WINDOW_TIME;}
  else if (conntype == '1' || conntype == '2'){
    engcost = COST_MOVE_LIFT_ENERGY;
    engtime =COST_MOVE_LIFT_TIME;}
    
  if (conntype == '2'){
    bool normaluse = lift2Operational && hasItem(inventory, inventoryCount, ENT_LIFT_PASS);
    if (!normaluse){emergencyTripsLeft--;}}
  applyResourceDelta(energy, time, engcost, engtime);
  currentFloor = targetFloor;
  currentRoom = targetRoom;
  return true;}
  
bool handleInspectCommand(int currentFloor, int currentRoom,
                          int& energy, int& time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int& score){
                          
  if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR){
    cout << "[!] Invalid floor or room." << endl;return false;}
  applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);
  
  for (int i = 0;i<entityCount;i++){
    char symb = roomEntities[currentFloor-1][currentRoom-1][i];
    if (symb == '4' && !objectives[OBJ_CLUE_4]){
      objectives[OBJ_CLUE_4] = true;
      applyScoreDelta(score, SCORE_CLUE_FOUND);
      cout << "[+] Clue 4 discovered!" << endl;}
    else if (symb == '2' && !objectives[OBJ_CLUE_2]){
      objectives[OBJ_CLUE_2] = true;
      applyScoreDelta(score, SCORE_CLUE_FOUND);
      cout << "[+] Clue 2 discovered!" << endl;}
    else if (symb == '7' && !objectives[OBJ_CLUE_7]){
      objectives[OBJ_CLUE_7] = true;
      applyScoreDelta(score, SCORE_CLUE_FOUND);
      cout << "[+] Clue 7 discovered!" << endl;}}
      
    return true;}

void removeRoomEntity(char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                      int floor, int room, int& entityCount, int index){
  for (int i = index; i < entityCount - 1; i++){
  roomEntities[floor-1][room-1][i] = roomEntities[floor-1][room-1][i+1];}
  entityCount--;}

bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int& energy, int& time, int& score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int& entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                          bool objectives[NUM_OBJECTIVES]){
                          
if (currentFloor < 1 || currentFloor > NUM_FLOORS || currentRoom < 1 || currentRoom > ROOMS_PER_FLOOR){
  cout << "[!] Invalid floor or room." << endl; return false;}

int index = -1;
for (int i = 0;i< entityCount; i++){
  if (roomEntities[currentFloor-1][currentRoom-1][i] == itemSymbol){index =i;break;}}

if (index == -1){cout << "[!] Nothing like that here." << endl; return false;}

if (itemSymbol == '4' || itemSymbol == '2' || itemSymbol == '7' || itemSymbol == '~' || itemSymbol == '!' || itemSymbol == ';' || itemSymbol == '^'){
  cout <<"[!] This entity cannot be collected." << endl; return false;}

bool isStoredItem = false;
if (itemSymbol == '<' || itemSymbol == '*' || itemSymbol == '(' || itemSymbol == '%' || itemSymbol == '$' || itemSymbol == '@') {
        isStoredItem = true;}
        
if (isStoredItem && inventoryCount >= MAX_INVENTORY_CAPACITY){cout << "[!] Inventory is full." << endl; return false;}
applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);

if (itemSymbol == '<') {
  addItem(inventory, inventoryCount, itemSymbol);
  objectives[OBJ_MASTER_KEY] = true;
  applyScoreDelta(score, SCORE_KEY_COLLECT);}
else if (itemSymbol == '*') {
  addItem(inventory, inventoryCount, itemSymbol);
  objectives[OBJ_FUSE_COLLECTED] = true;}
else if (itemSymbol == '(') {
  addItem(inventory, inventoryCount, itemSymbol);
  objectives[OBJ_LIFT_PASS] = true;
  applyScoreDelta(score, SCORE_LIFT_PASS);}
else if (itemSymbol == '%' || itemSymbol == '$') {
  addItem(inventory, inventoryCount, itemSymbol);}
else if (itemSymbol == '@') {
  addItem(inventory, inventoryCount, itemSymbol);
  objectives[OBJ_CAT_CARRIER] = true;}
else if (itemSymbol == ')') {
  applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);}
else if (itemSymbol == '\\') {
  applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);}
else if (itemSymbol == '&') {
  applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);}
removeRoomEntity(roomEntities, currentFloor, currentRoom, entityCount, index);
cout << "[+] Item collected successfully." << endl;
return true;}

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES]){

if (!hasItem(inventory, inventoryCount, itemSymbol)){cout << "[!] You do not have this item in your inventory." << endl;return false;}

switch (itemSymbol){
  case '%':
    if (currentFloor != 4 || currentRoom != 1){cout << "[!] You cannot rescue the passenger here." << endl;return false;}
    if (objectives[OBJ_PASSENGER_RESCUED]){cout << "[!] The passenger has already been rescued." << endl;return false;}
    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME); objectives[OBJ_PASSENGER_RESCUED] = true;
    applyScoreDelta(score, SCORE_RESCUE_PASSENGER); removeItem(inventory, inventoryCount, itemSymbol);
    cout << "[+] Passenger rescued successfully!" << endl; return true;

  case '@':
    if (currentFloor != 3 || currentRoom != 3){cout << "[!] You cannot secure the cat here." << endl;return false;}
    if (objectives[OBJ_CAT_SECURED]){cout << "[!] The cat has already been secured." << endl;return false;}
    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME); objectives[OBJ_CAT_SECURED] = true;
    applyScoreDelta(score, SCORE_SECURE_CAT); removeItem(inventory, inventoryCount, itemSymbol);
    cout << "[+] Cat secured successfully!" << endl; return true;

  case '*':
    if (currentFloor != 3 || currentRoom != 4){cout << "[!] You cannot install the fuse control panel here." << endl;return false;}
    if (!installFuseControlPanel(inventory, inventoryCount, objectives, score)){return false;}
    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME); cout << "[+] Fuse control panel installed!" << endl;
    return true;
  
  case '^': cout << "[!] The train key is used automatically during train preparation." << endl; return false;
  default: cout << "[!] Nothing to use that on here." << endl; return false;}}
  

bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int& score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int& entityCount){

if (currentFloor != 5 || currentRoom != 1){cout << "[!] There is no cabinet to unlock here." << endl;return false;}
if (objectives[OBJ_CABINET_UNLOCKED]){cout << "[!] The cabinet is already unlocked." << endl;return false;}
if (!verifyCabinetCode(code, objectives)){cout << "[!] Wrong code or missing clues." << endl;return false;}
objectives[OBJ_CABINET_UNLOCKED] = true;
score += SCORE_CABINET_UNLOCK;
objectives[OBJ_TRAIN_KEY] = true;
score += SCORE_TRAIN_KEY;

for (int i = 0; i < entityCount; i++){
  if (roomEntities[currentFloor-1][currentRoom-1][i] == '^'){
    for (int j = i; j < entityCount - 1; j++){
      roomEntities[currentFloor-1][currentRoom-1][j] = roomEntities[currentFloor-1][currentRoom-1][j + 1];}
    entityCount--;
    break;}}
cout << "[+] Cabinet unlocked successfully!" << endl;
cout << "[+] Train key obtained!" << endl;return true;}




void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {

  string line, command;
  bool running = true;
  bool success = false;
  //Select starting room
  if (playerFloor == PLAYER_SELECTS || playerRoom == PLAYER_SELECTS){
    while (true){
      cout << "Enter starting floor and room: ";
      if (!getline(cin, line)){return;}
      istringstream in(line);
      int startFloor, startRoom;
      if (!(in >> startFloor >> startRoom)){cout << "[!] Enter a valid floor and room." << endl;continue;}
      if (!validateStartingRoom(startFloor, startRoom, horizontalConn)){
        cout << "[!] Invalid starting room. Try again." << endl;
        continue;}
      playerFloor = startFloor;
      playerRoom = startRoom;
      break;}}
      
//Main simulation loop
string message = "";
while (running){
  clearScreen();
  renderDashboard(playerFloor, playerRoom, energy, time, score,inventory, inventoryCount, emergencyTripsLeft);
  cout << endl;
  cout << "+**********<-.-.-.-.-.-.-.-.-.-.-.-.-.>**********+" << endl;
  cout << "|" << setfill(' ') << setw(49) << "|" << endl;
  cout << "|           Available Commands:" << setfill(' ') << setw(19) << "|"<< endl;
  cout << "|" << setfill(' ') << setw(49) << "|" << endl;
  cout << "| move F R       - Move to another room" << setfill(' ') << setw(11) << "|" << endl;
  cout << "| inspect        - Inspect current room" << setfill(' ') << setw(11) << "|" << endl;
  cout << "| collect X      - Collect an item" << setfill(' ') << setw(16) << "|" << endl;
  cout << "| use X          - Use an inventory item" << setfill(' ') << setw(10) << "|" << endl;
  cout << "| unlock CODE    - Unlock the cabinet" << setfill(' ') << setw(13) << "|" << endl;
  cout << "| board          - Board the train" << setfill(' ') << setw(16) << "|" << endl;
  cout << "| status         - Display player status" << setfill(' ') << setw(10) << "|" << endl;
  cout << "| objectives     - Display objectives" << setfill(' ') << setw(13) << "|" << endl;
  cout << "| connections    - to check the connections" << setfill(' ') << setw(7) << "|" << endl;
  cout << "| building       - to load the whole building Map" << setfill(' ') << setw(1) << "|" << endl;
  cout << "| save           - Save current game" << setfill(' ') << setw(14) << "|" << endl;
  cout << "| load           - Load saved game" << setfill(' ') << setw(16) << "|" << endl;
  cout << "| board          - Board Train from the Platfrom |" << endl;
  cout << "| quit           - Exit the game" << setfill(' ') << setw(18) << "|" << endl;
  cout << "|" << setfill(' ') << setw(49) << "|" << endl;
  cout << "+**********<-.-.-.-.-.-.-.-.-.-.-.-.-.>***********+" << endl;
  cout << endl;
  cout << "***** CURRENT POSITION *****" << endl;
  cout << endl;
  renderAsciiBuilding(playerFloor, playerRoom);
  cout << endl;
  cout << endl;
  cout << "****** ROOM DETAIL ******" << endl;
  cout << endl;
  renderRoomDetails(playerFloor, playerRoom,roomEntities,entityCounts[playerFloor - 1][playerRoom - 1]);
  cout <<endl;
  cout << "******** MESSAGE ********" << endl;
  if (message != ""){cout << endl << message << endl; message = "";}
  cout << endl;
  cout << "> ";

  if (!getline(cin, line)){break;}
  istringstream in(line);
  command = "";
  in >> command;
  success = false;
  ostringstream captured;
  streambuf* oldBuf = cout.rdbuf(captured.rdbuf());
  //MOVE
  if (command == "move"){
    int targetFloor, targetRoom;
    if (!(in >> targetFloor >> targetRoom)){
      cout << "[!] Usage: move FLOOR ROOM" << endl; cout.rdbuf(oldBuf); message = captured.str(); continue;}
    success = handleMoveCommand(targetFloor, targetRoom,playerFloor, playerRoom,energy, time,horizontalConn,inventory,inventoryCount,
                                emergencyTripsLeft,objectives[OBJ_LIFT2_ACTIVE]);}


  //INSPECT
  else if (command == "inspect"){
    success = handleInspectCommand(playerFloor, playerRoom,energy, time,roomEntities,entityCounts[playerFloor - 1][playerRoom - 1],objectives, score);}


  //COLLECT
  else if (command == "collect"){
    char itemSymbol;
    if (!(in >> itemSymbol)){
      cout << "[!] Usage: collect ITEM" << endl;cout.rdbuf(oldBuf); message = captured.str(); continue;}
    success = handleCollectCommand(itemSymbol, playerFloor, playerRoom, energy, time, score, roomEntities,entityCounts[playerFloor - 1][playerRoom - 1],
                                  inventory, inventoryCount, objectives);
    if (success){
      if (itemSymbol == '$'){
        emergencyTripsLeft = 3;}}}


  //USE
  else if (command == "use"){
    char itemSymbol;
    if (!(in >> itemSymbol)){
      cout << "[!] Usage: use ITEM" << endl;cout.rdbuf(oldBuf); message = captured.str();continue;}
    success = handleUseCommand(itemSymbol,playerFloor, playerRoom,energy, time, score,inventory, inventoryCount,objectives);}

  //UNLOCK CABINET
  else if (command == "unlock"){
    string code;
    if (!(in >> code)){
      cout << "[!] Usage: unlock CODE" << endl;cout.rdbuf(oldBuf); message = captured.str();continue;}
    success = handleUnlockCabinetCommand(code.c_str(),playerFloor, playerRoom,objectives, score,roomEntities,entityCounts[playerFloor - 1][playerRoom - 1]);
    if (success){
      applyResourceDelta(energy, time,COST_ITEM_USE_ENERGY,COST_ITEM_USE_TIME);}}


  //BOARD TRAIN
  else if (command == "board"){
    if (playerFloor != 5 || playerRoom != 2){
      cout << "[!] There is no train here." << endl;}
    else if (!objectives[OBJ_TRAIN_KEY] || !objectives[OBJ_PASSENGER_RESCUED] || !objectives[OBJ_CAT_SECURED]){
      cout << "[!] Complete all train preparation requirements first." << endl;}
    else if (objectives[OBJ_TRAIN_PREPARED]) {
      cout << "[!] The train is already prepared." << endl;}
    else {
      objectives[OBJ_TRAIN_PREPARED] = true;
      score += SCORE_PREPARE_TRAIN;
      cout << "[+] Train prepared successfully!" << endl;
      objectives[OBJ_MISSION_WON] = true;
      score += SCORE_MISSION_VICTORY;
      cout << "[+] Mission completed! You won!" << endl;
      success = true;}}
      

  //STATUS
  else if (command == "status"){renderDashboard(playerFloor, playerRoom, energy, time, score,inventory, inventoryCount, emergencyTripsLeft);}

  //OBJECTIVES
  else if (command == "objectives"){cout << "[x] mean done and [ ] means not yet!"<< endl;renderObjectivesChecklist(objectives);}
  
  // Building Map
  else if (command == "building"){
    ifstream mapView("data/building.txt");
    string mapLine;
    while (getline(mapView, mapLine)) {cout << mapLine << "\n";}
    cout << "\n";
    cout << endl;}
    
  //CONNECTION
  else if (command == "connections"){
    cout << "Connections in the map: " << endl;
    cout << endl;
    for (int f = NUM_FLOORS; f >= 1; f--) {
        cout << "F" << f << " conn: ";
    for (int k = 0; k <ROOMS_PER_FLOOR - 1; k++){
        cout << "[" << horizontalConn[f-1][k] << "]";}
        cout << endl;}
    cout << endl;}

  //SAVE
  else if (command == "save"){
    if (saveGameStateToFile("./out/savegame.txt",playerFloor, playerRoom,energy, time, score,inventory, inventoryCount,objectives, emergencyTripsLeft,
                            roomEntities, entityCounts)){
        cout << "[+] Game saved successfully." << endl;}
    else {cout << "[!] Could not save the game." << endl;}}

  //LOAD
  else if (command == "load"){
    if (loadGameStateFromFile("./out/savegame.txt",playerFloor, playerRoom,energy, time, score,inventory, inventoryCount,objectives, emergencyTripsLeft,
                            roomEntities, entityCounts)){
        cout << "[+] Game loaded successfully." << endl;
        success = true;}
    else {cout << "[!] Could not load the game." << endl;}}

  //QUIT
  else if (command == "quit"){
    cout << "[!] Leaving the mission..." << endl;
            running = false;}
  
  //INVALID COMMAND
  else {cout << "[!] Unknown command.  Check the commands list above." << endl;}
  cout.rdbuf(oldBuf);
  message = captured.str();

  // Check end conditions after successful actions
  if (success){
    if (checkEndConditions(energy, time, objectives)){running = false;}}}

  //Finish simulation
  if (!objectives[OBJ_MISSION_WON]){objectives[OBJ_MISSION_FAILED] = true;}
  
  writeGameSummaryReport("./out/game_summary.txt",score, energy, time,objectives,objectives[OBJ_MISSION_WON]);
  ifstream report("out/game_summary.txt");
  string reportLine;
  cout << endl;
  while (getline(report, reportLine)){cout << reportLine << endl;}
  report.close();
  cout << endl;
  if (!objectives[OBJ_MISSION_WON]){
  if (energy <= 0) cout << "You ran out of Energy." << endl;
  else if (time <= 0) cout << "You ran out of Time." << endl;
  else cout << "You left the mission." << endl;}
  
  if (objectives[OBJ_MISSION_WON]) {cout << "[+] Final Result: MISSION COMPLETED!" << endl;}
  else {cout << "[!] Final Result: MISSION FAILED." << endl;}}




// Modular simulation loop using primitive variables
