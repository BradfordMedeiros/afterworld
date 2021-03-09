; Movement parameters
(define movement-speed 0.1)
(define jump-height 10)
(define gravity 1)
(define tilt (list 0 0))
(define jump-sound-effect "./res/sounds/sample.wav")

; Gun parameters
(define gun-name "default-gun")
(define gun-model "../gameresources/weapons/fork.dae")
(define gun-fire-animation "default:0")
(define gun-fire-sound "./res/sounds/silenced-gunshot.wav")
(define hud-element "")
(define x-offset-position 0)
(define y-offset-position 0)
(define z-offset-position -15)

(define firing-rate 100)
(define can-hold #t)
(define spread '(5 100))
(define max-ammo 30)
;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Movement code
(define (move x y z) (applyimpulse mainobj (list x y (* -1 z))))
(define (look x y) (gameobj-setrotd! mainobj (* 0.1 x) (* -0.1 y) 0))

(define (set-gravity vec3) (display "set-gravity placeholder\n"))
(define is-grounded #t)
(define (jump amount) 
  (display "Calling jump\n")
  (applyimpulse mainobj (list 0 amount 0))
  (playclip "&jumpsound")
)


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
  (display (string-append "mouse move: (" (number->string x) ", " (number->string y) ")\n"))
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
)


(define current-ammo max-ammo)
(define last-shooting-time -1000)

(define (fire-gun)
  (define elapsedMilliseconds (* 1000 (time-seconds)))
  (define timeSinceLastShot (- elapsedMilliseconds last-shooting-time))
  (define hasAmmo (> current-ammo 0))
  (define lessThanFiringRate (> timeSinceLastShot firing-rate))
  
  (define x-offset (random (car spread)))
  (define y-offset (random (cadr spread)))
  (display (string-append "offset: (" (number->string x-offset) ", " (number->string y-offset) ")"))

  (display (string-append "current ammo: " (number->string current-ammo) "\n"))


  (if (and hasAmmo lessThanFiringRate)
    (begin
      (display "should play animation: ")
      (display gun-fire-animation)
      (display "\n")
      (display (gameobj-animations (lsobj-name "gun")))
      (gameobj-playanimation (lsobj-name "gun") gun-fire-animation)
      (playclip "&gunsound")
      (set! current-ammo (- current-ammo 1))
      (set! last-shooting-time elapsedMilliseconds)
      (display (time-seconds))
    )
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

(define (update-values)
  (display "fps script: updating values\n")

  (display "removing objects\n")
  (rm-obj (gameobj-id (lsobj-name "gun")))
  (rm-obj (gameobj-id (lsobj-name "&gunsound")))
  (rm-obj (gameobj-id (lsobj-name "&jumpsound")))
  (display "making objects")

  (mk-obj-attr "gun"       (list (list "mesh" gun-model)))
  (mk-obj-attr "&gunsound" (list (list "clip" gun-fire-sound))) 
  (mk-obj-attr "&jumpsound" (list (list "clip" jump-sound-effect))) 
  (gameobj-setpos! (lsobj-name "gun") (list x-offset-position y-offset-position z-offset-position))
  ; todo add set rotation (everything right now is relative?)
)

