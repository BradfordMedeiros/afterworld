; Movement parameters
(define movement-speed 0.1)
(define jump-height 10)
(define gravity 1)
(define tilt (list 0 0))
(define jump-sound-effect "./res/sounds/sample.wav")

;; GUN DATA
(define gundata
  (list 
    (list "pistol" (list
      (list "gun-model" "../gameresources/weapons/pistol.dae")
      (list "gun-fire-animation" "default:0")
      (list "gun-fire-sound" "./res/sounds/silenced-gunshot.wav")
      (list "hud-element" "../gameresources/ui/hud1/hud.png")
      (list "x-offset-position" 0)
      (list "y-offset-position" 0)
      (list "z-offset-position" 0)
      (list "firing-rate" 1000)
      (list "can-hold" #f)
      (list "spread" '(0 0))
      (list "max-ammo" 30)
      (list "is-raycast" #t))
    )
    (list "fork" (list
      (list "gun-model" "../gameresources/weapons/fork.dae")
      (list "gun-fire-animation" "default:0")
      (list "gun-fire-sound" "./res/sounds/silenced-gunshot.wav")
      (list "hud-element" "../gameresources/ui/hud1/hud.data.png")
      (list "x-offset-position" 0)
      (list "y-offset-position" 0)
      (list "z-offset-position" 0)
      (list "firing-rate" 100)
      (list "can-hold" #t)
      (list "spread" '(5 100))
      (list "max-ammo" 30)
      (list "is-raycast" #f))
    )
  )
)

; Gun parameters
(define gun-name "default-gun")
(define gun-fire-animation "default:0")
(define hud-element "../gameresources/ui/hud1/hud.png")
(define firing-rate 100)
(define can-hold #t)
(define spread '(5 100))
(define max-ammo 30)
(define is-raycast #t)
;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Movement code
(define (move x y z) 
  (if is-grounded
    (applyimpulse-rel mainobj (list x y (* -1 z)))
    (applyimpulse-rel mainobj (list (* x 0.1) (* y 0.1) (* -1 z)))
  )
)
(define (look x y) (gameobj-setrotd! mainobj (* 0.1 x) (* -0.1 y) 0))
(define (set-gravity vec3) (display "set-gravity placeholder\n"))

(define (reverse-quat quat) (orientation-from-pos '(0 0 0) (move-relative '(0 0 0) quat -1)))

;;;;;;;;; JUMPING CODE
(define is-grounded #t)
(define grounded-obj #f)
(define (filterMainObj obj1 obj2 normal fn)
  (define id (gameobj-id mainobj))
  (define otherObject #f)
  (define relativeNormal normal)
  (if (equal? (gameobj-id obj1) id) 
    (set! otherObject obj2)
  )
  (if (equal? (gameobj-id obj2) id) 
    (begin
      (set! otherObject obj1)
      (set! relativeNormal (reverse-quat normal)) ;; This normal is always obj2 on obj1 so have to reverse in this case 
    )
  )
  (if otherObject (fn relativeNormal otherObject))
)
(define (filterMainObjWithOther obj1 obj2 filterid fn) 
  (define id (gameobj-id mainobj))
  (define otherObject #f)
  (if (equal? (gameobj-id obj1) id) (set! otherObject obj2))
  (if (equal? (gameobj-id obj2) id) (set! otherObject obj1))
  (if (and otherObject (equal? (gameobj-id otherObject) filterid))
    (fn)
  )
)
(define (set-grounded normal otherobj)
  (define offsetvec (move-relative '(0 0 0) normal 1))
  (display "set-grounded called!\n")
  (if (> (cadr offsetvec) 0)
    (begin
      (set! is-grounded #t)
      (set! grounded-obj (gameobj-id otherobj))
    )
  )
)
(define (set-jumping)
  (set! is-grounded #f)
  (set! grounded-obj #f)
)

(define (onCollideEnter obj1 obj2 hitpoint normal) 
  (display "collide enter -- \n")
  (filterMainObj obj1 obj2 normal set-grounded)
)
(define (onCollideExit obj1 obj2)
  (display "collide exit -- grounded obj: ")
  (display grounded-obj)
  (display "\n")
  (if grounded-obj
    (filterMainObjWithOther obj1 obj2 grounded-obj set-jumping)
  )
)

(define (jump amount) 
  (if is-grounded
    (begin
      (display "Calling jump\n")
      (applyimpulse mainobj (list 0 amount 0))
      (playclip "&jumpsound")
    )
  )
)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; Gunplay code
(define (set-gun-model) (display "set gun model placeholder\n"))
(define (set-hud-element) (display "set hud element placeholder\n"))
(define (set-gun-offset x y) (display "set gun offset placeholder \n"))
(define (fire-gun) (display "fire gun placeholder\n"))

(define look-velocity-x 0)
(define look-velocity-y 0)
(define (reset-look-velocity) 
  (set! look-velocity-x 0)
  (set! look-velocity-y 0)
)
(define (onMouseMove x y)
  ;(display (string-append "mouse move: (" (number->string x) ", " (number->string y) ")\n"))
  (set! look-velocity-x x)
  (set! look-velocity-y y)
)

(define go-forward #f)
(define go-left #f)
(define go-backward #f)
(define go-right #f)

;; Input binding to the gamelogic. TODO decouple from this file. 
(define (onKey key scancode action mods) 
  (display "key: ")
  (display key)
  (display ", action: ")
  (display action)
  (display "\n")
  (if (= key 87)
    (begin 
      (if (= action 1) (set! go-forward #t))
      (if (= action 0) (set! go-forward #f))
    )
  )
  (if (= key 65)
    (begin
      (if (= action 1) (set! go-left #t))
      (if (= action 0) (set! go-left #f))
    )
  )
  (if (= key 83)
    (begin
      (if (= action 1) (set! go-backward #t))
      (if (= action 0) (set! go-backward #f))
    )
  )
  (if (= key 68)
    (begin
      (if (= action 1) (set! go-right #t))
      (if (= action 0) (set! go-right #f))
    )
  )

  (if (= key 32) (jump jump-height))

  (if (and (= key 82) (= action 1))
    (update-values)
  )
)

(define is-holding #f)
(define (onMouse button action mods) 
  (display "button: ")
  (display button)
  (display ", action: ")
  (display action)
  (display "\n")
  (if (and (= button 0) (= action 1))
    (begin
      (fire-gun)
      (set! is-holding #t)
    )
  )
  (if (and (= button 0) (= action 0))
    (set! is-holding #f)
  )
)


(define (onFrame)
  (look look-velocity-x look-velocity-y)
  (reset-look-velocity)
  (begin
    (if (equal? go-forward  #t) (move 0 0 movement-speed))
    (if (equal? go-left     #t) (move (* -0.8 movement-speed) 0 0))
    (if (equal? go-right    #t) (move (* 0.8 movement-speed) 0 0))
    (if (equal? go-backward #t) (move 0 0 (* -1 movement-speed)))
  )
  (if (and can-hold is-holding) (fire-gun))
  (draw-line last-ray-from last-ray-to)
)


(define current-ammo max-ammo)
(define last-shooting-time -1000)

(define last-ray-from '(0 0 0))
(define last-ray-to '(0 0 0))
(define (fire-ray)
  (define mainobjpos (gameobj-pos mainobj))
  (define hitpoints (raycast mainobjpos (gameobj-rot mainobj) 500))   ;; BUG  DUE TO COLLISION GROUPS SO THIS DOESN'T WORK   

  (if (> (length hitpoints) 0)
    (begin
      (let* (
        (firsthit (car hitpoints))  
        (frompoint (cadr firsthit)) 
        (id (car firsthit))
        (topoint (move-relative (cadr firsthit) (caddr firsthit) 10)) 
      )
        (begin
          (set! last-ray-from frompoint)
          (set! last-ray-to topoint)
          (display hitpoints)
          ; obviously only should do this if the id of the object here is in the hitpoints
          (sendnotify "bullet" (number->string id))
        )
      )
    )
  )
)

(define (display-debug-gun hasAmmo lessThanFiringRate)
  (if #f
    (begin
      (display "cannot shoot because: ")
      (display "(ammo: ")
      (display hasAmmo)
      (display ", ")
      (display "firing-rate: ")
      (display lessThanFiringRate)
      (display ")\n")
    )
  )
)

(define (fire-gun)
  (define elapsedMilliseconds (* 1000 (time-seconds)))
  (define timeSinceLastShot (- elapsedMilliseconds last-shooting-time))
  (define hasAmmo (> current-ammo 0))
  (define lessThanFiringRate (> timeSinceLastShot firing-rate))
  
  (define x-offset (random (+ 1 (car spread))))
  (define y-offset (random (+ 1 (cadr spread))))
  ;(display (string-append "offset: (" (number->string x-offset) ", " (number->string y-offset) ")"))
  ;(display (string-append "current ammo: " (number->string current-ammo) "\n"))

  (if (and hasAmmo lessThanFiringRate)
    (begin
      (gameobj-playanimation (lsobj-name "gun") gun-fire-animation)
      (playclip "&gunsound")
      (if is-raycast
        (fire-ray)
        (emit (gameobj-id (lsobj-name "+particles")))
      )
      (set! current-ammo (- current-ammo 1))
      (set! last-shooting-time elapsedMilliseconds)
    )
    (display-debug-gun hasAmmo lessThanFiringRate)
  )
)

(define (switch-gun gundata)
  (define gunname (car gundata))
  (define gun-attr (cadr gundata))

  (define x-offset-position (cadr (assoc "x-offset-position" gun-attr)))
  (define y-offset-position (cadr (assoc "y-offset-position" gun-attr)))
  (define z-offset-position (cadr (assoc "z-offset-position" gun-attr)))
  (set! firing-rate (cadr (assoc "firing-rate" gun-attr)))
  (set! can-hold (cadr (assoc "can-hold" gun-attr)))
  (set! spread (cadr (assoc "spread" gun-attr)))
  (set! max-ammo (cadr (assoc "max-ammo" gun-attr)))
  (set! gun-fire-animation (cadr (assoc "gun-fire-animation" gun-attr)))
  (set! is-raycast (cadr (assoc "is-raycast" gun-attr)))

  (rm-obj (gameobj-id (lsobj-name "gun")))
  (rm-obj (gameobj-id (lsobj-name "&gunsound")))

  (mk-obj-attr "gun"       (list (list "mesh" (cadr (assoc "gun-model" gun-attr)))))
  (mk-obj-attr "&gunsound" (list (list "clip" (cadr (assoc "gun-fire-sound" gun-attr))))) 
  (gameobj-setpos! (lsobj-name "gun") (list x-offset-position y-offset-position z-offset-position))
  (set-texture (gameobj-id (lsobj-name "gunhud")) (cadr (assoc "hud-element" gun-attr)))
  ; todo add set rotation (everything right now is relative?)
)

(define (update-values)
  (switch-gun (list-ref gundata (random (length gundata))))
)
