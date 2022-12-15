#include "./weapon.h"

extern CustomApiBindings* gameapi;

struct WeaponParams {
  float firingRate;
  float recoilLength;
  float recoilPitchRadians;
  bool canHold;
  bool isIronsight;
  bool isRaycast;

};

struct Weapons {
  bool isHoldingLeftMouse;
  bool isHoldingRightMouse;
  WeaponParams weaponParams;
};


/*(define (change-gun modelpath xoffset yoffset zoffset xrot yrot zrot xscale yscale zscale)
  (define gunpos (list xoffset yoffset zoffset))
  (define gunattrs (list 
    (list "mesh" modelpath)
    (list "position" gunpos)
    (list "scale" (list xscale yscale zscale))
    (list "physics" "disabled")
    (list "layer" "no_depth")
  ))
  (if (not (equal? gunid #f)) (rm-obj gunid))
  (let ((id (mk-obj-attr "weapon" gunattrs)))
    (set! gunid id)
    (set! initial-gun-pos gunpos)
    (set! initial-gun-rot 
      (orientation-from-pos 
        (list 0 0 0) 
        (list 
          (string->number xrot) 
          (string->number yrot)
          (string->number zrot)
        )
      )
    )
    (format #t "the gun id is: ~a, modelpath: ~a\n" id modelpath)
    (gameobj-setrot! (gameobj-by-id id) initial-gun-rot) ; mk-obj-attr has funky format so this for now
    (make-parent gunid (gameobj-id (get-parent)))
  )
)*/


void changeGun(Weapons& weapons, std::string gun){
  auto gunQuery = gameapi -> compileSqlQuery(
   std::string("select modelpath, fire-animation, fire-sound, xoffset-pos, ") +
   "yoffset-pos, zoffset-pos, xrot, yrot, zrot, xscale, yscale, zscale, " + 
   "firing-rate, hold, raycast, ironsight, iron-xoffset-pos, iron-yoffset-pos, " + 
   "iron-zoffset-pos, particle, hit-particle, recoil-length, recoil-angle, " + 
   "recoil, recoil-zoom, projectile, bloom, script " + 
   "from guns where name = " + gun
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(gunQuery, &validSql);
  modassert(validSql, "error executing sql query");

  modassert(result.size() > 0, "no gun named: " + gun);
  modassert(result.size() == 1, "more than one gun named: " + gun);
  std::cout << "gun: result: " << print(result.at(0)) << std::endl;

  weapons.weaponParams.firingRate = floatFromFirstSqlResult(result, 12);
  weapons.weaponParams.recoilLength = floatFromFirstSqlResult(result, 21);
  weapons.weaponParams.recoilPitchRadians = floatFromFirstSqlResult(result, 22);


  weapons.weaponParams.canHold = boolFromFirstSqlResult(result, 13);
  weapons.weaponParams.isIronsight = boolFromFirstSqlResult(result, 15);
  weapons.weaponParams.isRaycast = boolFromFirstSqlResult(result, 14);


  /*
    (format #t "warning: no gun named: ~a\n" gunname)
    (let* ((guninfo (car gunstats)) (bloomVec (parse-stringvec (list-ref guninfo 26))))
      (change-gun 
        (list-ref guninfo 0) 
        (string->number (list-ref guninfo 3)) 
        (string->number (list-ref guninfo 4))
        (string->number (list-ref guninfo 5))
        (list-ref guninfo 6)
        (list-ref guninfo 7)
        (list-ref guninfo 8)
        (string->number (list-ref guninfo 9))
        (string->number (list-ref guninfo 10))
        (string->number (list-ref guninfo 11))
      )
      (set-animation     (list-ref guninfo 1))

      ; Firing Parameters

      (set! ironsight-offset (list 
        (string->number (list-ref guninfo 16))
        (string->number (list-ref guninfo 17))
        (string->number (list-ref guninfo 18))
      ))
      (set! recoilTranslate (parse-stringvec (list-ref guninfo 23)))
      (set! recoilZoomTranslate (parse-stringvec (list-ref guninfo 24)))
      (set! bloom-radius (list-ref bloomVec 0))

      (format #t "traits: firing-params - rate: ~a, can-hold: ~a, raycast: ~a, ironsight: ~a, ironsight-offset: ~a\n" firing-rate can-hold is-raycast is-ironsight ironsight-offset)
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      (set! last-shooting-time initFiringTime)
      (change-sound (list-ref guninfo 2))
      (change-emitter 
        (list-ref guninfo 19)
        (list-ref guninfo 20)
        (list-ref guninfo 25)
      )
      (change-script (list-ref guninfo 27))
 
  )*/
}

std::string weaponsToString(Weapons& weapons){
  std::string str;
  str += std::string("isHoldingLeftMouse: ") + (weapons.isHoldingLeftMouse ? "true" : "false") + "\n";
  str += std::string("isHoldingRightMouseo: ") + (weapons.isHoldingRightMouse ? "true" : "false") + "\n";
  return str;
}

void tryFireGun(Weapons& weapons){
  std::cout << "try fire gun" << std::endl;
}

bool debugAssert = false;

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    Weapons* weapons = new Weapons;

    weapons -> isHoldingLeftMouse = false;
    weapons -> isHoldingRightMouse = false;

    changeGun(*weapons, "pistol");

  	return weapons;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    delete weapons;
  };
  binding.onMouseCallback = [](objid scriptId, void* data, int button, int action, int mods) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    if (button == 0){
      if (action == 0){
        weapons -> isHoldingLeftMouse = false;
        tryFireGun(*weapons);
      }else if (action == 1){
        weapons -> isHoldingLeftMouse = true;
      }
    }else if (button == 1){
      if (action == 0){
        weapons -> isHoldingRightMouse = false;
        modassert(!debugAssert, "should add ironsight");
      }else if (action == 1){
        weapons -> isHoldingRightMouse = true;
        modassert(!debugAssert, "should add ironsight");
      }
    }
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    Weapons* weapons = static_cast<Weapons*>(data);
    if (key == "change-gun"){
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "change-gun value invalid");
      changeGun(*weapons, *strValue);
    }
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    Weapons* weapons = static_cast<Weapons*>(data);
    //std::cout << weaponsToString(*weapons) << std::endl;
  };
  return binding;
}

