#ifndef MOD_AFTERWORLD_LAYER
#define MOD_AFTERWORLD_LAYER

// Doesn't define it, these values are elsewhere
// X & Y == 0  means they collide
struct PhysicsLayer {
  int main = 2;           // 0b00010
  int bones = 0;          // 0b00000
  int enemyBounding = 2;  // 0b00010
  int particles = 1;      // 0b00001
  int effects = 4;        // 0b00100
};                        // 0b00yxy



int bonesAndObjects();

extern PhysicsLayer physicsLayers;

#endif