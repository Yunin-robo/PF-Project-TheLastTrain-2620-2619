#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "util.h"
#include "viewer_iface.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    // Player State Primitives
    int playerFloor = PLAYER_SELECTS;
    int playerRoom = PLAYER_SELECTS;
    int energy = INITIAL_ENERGY;
    int time = INITIAL_TIME;
    int score = INITIAL_SCORE;
    
    char inventory[MAX_INVENTORY_CAPACITY];
    int inventoryCount = 0;
    int emergencyTripsLeft = 0;

    // Objective State Booleans
    bool objectives[NUM_OBJECTIVES] = { false };

    // Building Connectivity & Entity Parallel Matrices
    char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1];
    char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
    int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR] = { 0 };

    const char* mapFile = "data/building.txt";
    const char* initStateFile = "data/initial-state.txt";

    if (!loadBuildingMap(mapFile, horizontalConn, roomEntities, entityCounts)) {
        cerr << "Error: Could not load " << mapFile << "\n";
        return 1;
    }
    
    ifstream mapView(mapFile);
    string mapLine;
    while (getline(mapView, mapLine)) {cout << mapLine << "\n";}
    cout << "\n";
    cout << endl;
    cout << "Connections in the map: " << endl;
    for (int f = NUM_FLOORS; f >= 1; f--) {
        cout << "F" << f << " conn: ";
    for (int k = 0; k <ROOMS_PER_FLOOR - 1; k++){
        cout << "[" << horizontalConn[f-1][k] << "]";}
        cout << endl;}
    cout << endl;
        
    roomEntities[0][3][1] = ENT_ENERGY_PACK;
    entityCounts[0][3] = 2;
    if (!loadInitialState(initStateFile, playerFloor, playerRoom, energy, time, score,
                          inventory, inventoryCount, objectives, emergencyTripsLeft)) {
        std::cerr << "Error: Could not load " << initStateFile << "\n";
        return 1;
    }

    cout << "===========================================\n";
    cout << " THE LAST TRAIN - SIMULATION INITIALIZED   \n";
    cout << "===========================================\n";

    runSimulation(playerFloor, playerRoom, energy, time, score,
                  inventory, inventoryCount, objectives, emergencyTripsLeft,
                  horizontalConn, roomEntities, entityCounts);

    return 0;
}
