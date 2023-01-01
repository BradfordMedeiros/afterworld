#include "./ai.h"

/*
  ai:
  core loop:
  - detect goals
  - pick highest utility goal
  - action 
  ---> repeast

  behavior tree: 
    - each agent has a state, and gets triggered into other states based on events
    - eg idle -> attacking
  
  goal 
    - goals exist as things in a scene -> real objects with position, thing you can do 
    - of the detected goals, each goal is evaluated with some utility value fn(actor, distance, goal) => number
    - do the goal that has the highest utility

  so to open a door 
    - possible goals: open door, unlock door
    - unlock door prereq to open door, so unlock door, then open door

    - kill player
    - take cover prereq to kill player
    - or just kill player directly
    - but those are different nodes and then have different probabilities
    - so will take cover since it'll increase probability


  needs a detectioun system to find possible goals 

  and then going to goals require actions

  actions 
  - move to place, 
  - shoot
  etc


*/

/*
  
;;;;;;;;; Ideally this code should be in a seperate script related to the detection objects
;; maybe could have a system that broadcasts it upward in hierachy?
(define current-target #f)
(define (get-target-pos) (if current-target (gameobj-pos-world current-target) #f))
(define (set-target obj) (set! current-target obj))

(define (is-visible obj) 
  (define hitpoints 
    (raycast 
      (gameobj-pos mainobj) 
      (orientation-from-pos (gameobj-pos mainobj) (gameobj-pos obj))
      100
    )
  )   
  (if (> (length hitpoints) 0)
    (equal? (gameobj-id obj) (car (car hitpoints)))
    #f
  )
)
(define (is-target obj) (string-contains (gameobj-name obj) "main"))            
(define (process-detection obj) 
  (if (is-target obj) 
    (if (is-visible obj)  
      (begin
        (set-target obj)
        (set-state 'attack)
      )
      (display "it is not visibile\n")
    )
  )
)

(define (filterMainObj id obj1 obj2 fn)
  (define otherObject #f)
  (if (equal? (gameobj-id obj1) id) (set! otherObject obj2))
  (if (equal? (gameobj-id obj2) id) (set! otherObject obj1))
  (if otherObject (fn otherObject))
)
(define (onGlobalCollideEnter obj1 obj2 pos normal oppositeNormal)
  (filterMainObj (gameobj-id (lsobj-name "enemy_detection")) obj1 obj2 process-detection)
) 

;;;;;;;;;;;;;;;;;;;

(define speed-per-second 4)
(define (move-toward targetpos)
  (define currentpos (gameobj-pos mainobj))
  (define toward-target (orientation-from-pos currentpos targetpos))
  (define newpos (move-relative currentpos toward-target (* (time-elapsed) speed-per-second)))
  (gameobj-setpos! mainobj newpos)
  (gameobj-setrot! mainobj toward-target)
)

(define last-shooting-time 0)
(define (fire-bullet)
  (define elapsedMilliseconds (* 1000 (time-seconds)))
  (define timeSinceLastShot (- elapsedMilliseconds last-shooting-time))
  (define lessThanFiringRate (> timeSinceLastShot 1000))
  (define mainobjrot (gameobj-rot mainobj))
  (if lessThanFiringRate
    (begin
      (set! last-shooting-time elapsedMilliseconds)
      (emit 
        (gameobj-id (lsobj-name "+enemy_particles"))
        (move-relative (gameobj-pos mainobj) mainobjrot 5)
        mainobjrot
        (move-relative (list 0 0 0) mainobjrot 10)
      )
    )
  )
)


(define (distance point1 point2)
  (define xdelta (- (car  point1) (car  point2)))
  (define ydelta (- (cadr point1) (cadr point2)))
  (define zdelta (- (caddr point1) (caddr point2)))
  (sqrt (+ (* xdelta xdelta) (* ydelta ydelta) (* zdelta zdelta)))
)
(define (is-close-enough targetpos) (< (distance targetpos (gameobj-pos mainobj)) 20))

;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (wander) (move-toward (list (random 10) 0 (random 10))))

;;;;;;;;;;;;;;;;;;;
(define modeToAnimation (list
  
))
(define mode #f)
(define (set-state newmode) 
  (define animationPair (assoc newmode modeToAnimation))
  (set! mode newmode)
  (if animationPair (gameobj-playanimation mainobj (cadr animationPair)))
) 

(set-state 'idle)

(define (onFrame)
  ;(if (equal? mode 'idle) (wander))
  (if (equal? mode 'attack)
    (let ((targetpos (get-target-pos)))
      (if targetpos
        (if (not (is-close-enough targetpos))
          (move-toward (get-target-pos))
          (fire-bullet)
        )
      )
    )
  )
)

;(set-target (lsobj-name ">maincamera"))
*/


struct WorldInfo {
  bool canSeePlayer;
  bool canShootPlayer;
  glm::vec3 playerPosition;
  // can i include the goals themselves here as available state? 
};

struct Agent {
  objid id;
};

struct Goal {
  int name;
  std::function<int(WorldInfo&)> score;
};

std::unordered_map<std::string, int> goalnameToInt;

int symbolIndex = -1;
int goalSymbol(std::string name){
  if (goalnameToInt.find(name) != goalnameToInt.end()){
    return goalnameToInt.at(name);
  }
  symbolIndex++;
  goalnameToInt[name] = symbolIndex;
  return symbolIndex;
} 

std::vector<Goal> goals = {  // these goals should be able to be dynamic, for example, be constructed by placing two bags of gold and having them pick up the one with higher worth
  Goal { 
    .name = goalSymbol("idle"), 
    .score = [](WorldInfo&) -> int { return 1; }
  },
  Goal { 
    .name = goalSymbol("moveToPlayer"), 
    .score = [](WorldInfo& info) -> int { 
      if (info.canSeePlayer){
        return 0;
      }
      return 10;
    }
  },
  Goal {
    .name = goalSymbol("shootPlayer"),
    .score = [](WorldInfo& info) -> int {
      if (info.canShootPlayer){
        return 20;
      }else{
        return 0;  // alternatively maybe make this have a dependency on goal moveToPlayer (and add some idea of goal being completed)
      }
    }
  },
};

Goal& getOptimalGoal(WorldInfo& worldInfo, Agent& agent){
  modassert(goals.size() > 0, "no goals in list for agent");
  int maxScore = goals.at(0).score(worldInfo);
  int maxScoreIndex = 0;
  for (int i = 1; i < goals.size(); i++){
    auto goalScore = goals.at(i).score(worldInfo);
    if (goalScore > maxScoreIndex){
      maxScore = goalScore;
      maxScoreIndex = i;
    }
  }
  return goals.at(maxScoreIndex);
}

void actionMoveTowardPosition(WorldInfo& worldInfo){

}
void actionShootPlayer(WorldInfo& worldInfo){

}

void doGoal(WorldInfo& worldInfo, Agent& agent, Goal& goal){
  static int idle = goalSymbol("idle");
  static int moveToPlayer = goalSymbol("moveToPlayer");
  static int shootPlayer = goalSymbol("shootPlayer");
  if (goal.name == idle){
    // do nothing
    return;
  }
  if (goal.name == moveToPlayer){
    actionMoveTowardPosition(worldInfo);
    return;
  }
  if (goal.name == shootPlayer){
    actionShootPlayer(worldInfo);
    return;
  }
}

bool canSeePlayer(){
  return false;
}
bool canShootPlayer(){
  return false;
}
glm::vec3 getPlayerPosition(){
  return glm::vec3(1.f, 1.f, 1.f);
}

void detectWorldInfo(WorldInfo& worldInfo){
  worldInfo.canSeePlayer = canSeePlayer();
  worldInfo.canShootPlayer = canShootPlayer();
  worldInfo.playerPosition = getPlayerPosition();
}

CScriptBinding aiBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.onFrame = [](int32_t id, void* data) -> void {
    Agent agent { 
      .id = id 
    };
    WorldInfo worldInfo {};
    detectWorldInfo(worldInfo);
    auto goal = getOptimalGoal(worldInfo, agent);
    doGoal(worldInfo, agent, goal);
  };

  return binding;
}