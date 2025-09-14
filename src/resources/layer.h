#ifndef MOD_AFTERWORLD_LAYER
#define MOD_AFTERWORLD_LAYER

#include <string>
#include <iostream>

// Doesn't define it, these values are elsewhere
// X & Y == 0  means they collide
struct PhysicsLayer {
  int main = 2;            // 0b00010
  int bones = 0;           // 0b00000
  int enemyBounding = 8;  // 0b01000
  int particles = 1;       // 0b00001
  int effects = 4;         // 0b00100
};                        

int bonesAndObjects();
void printLayerInfo();

extern PhysicsLayer physicsLayers;

#endif