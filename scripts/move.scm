(define using-fpscam #t)
(define (onCameraSystemChange defaultCam)
  (set! using-fpscam (not defaultCam))
)

(define movement-speed 0.1)
(define movement-speed-air 0.1)
(define (set-movement-speed speed speed-air)
  (set! movement-speed speed)
  (set! movement-speed-air speed-air)
  (format #t "traits: setting speed: ~a, air: ~a\n" speed speed-air)
)
(define jump-height 10)
(define jump-obj-name #f)
(define land-obj-name #f)
(define dash-obj-name #f)
(define can-dash #f)
(define dash-delay 0)

(define (create-sound soundname clip oldobjname)
  (if oldobjname (rm-obj (gameobj-id (lsobj-name oldobjname))))
  (if (> (string-length clip) 0)
    (begin
      (make-parent (mk-obj-attr soundname (list (list "clip" clip) (list "physics" "disabled"))) (gameobj-id mainobj))
      soundname
    )
    #f
  )
)
(define (set-jump-config height jumpsound landsound dashdelaytime dashsound)
  (set! jump-height height)
  (set! can-dash (>= dashdelaytime 0))
  (if (>= dashdelaytime 0)
    (set! dash-delay dashdelaytime)
    (set! dash-delay 0)
  )
  (format #t "traits: setting jump height: ~a, jump-sound: ~a, land-sound: ~a\n" height jumpsound landsound)
  (set! jump-obj-name (create-sound "&jump-sound" jumpsound jump-obj-name))
  (set! land-obj-name (create-sound "&land-sound" landsound land-obj-name))
  (set! dash-obj-name (create-sound "&dash-sound" dashsound dash-obj-name))
)

(define (set-object-values gravity restitution friction mass)
  (format #t "traits: set gravity ~a\n" gravity)
  (format #t "traits: set restitution ~a\n" restitution)
  (format #t "traits: set friction ~a\n" friction)
  (format #t "traits: set mass ~a\n" mass)

  (gameobj-setattr! 
    mainobj
    (list
      (list "physics_gravity"     (list 0 gravity 0))
      (list "physics_restitution" restitution)
      (list "physics_friction"    friction)
      (list "physics_mass"    mass)
    )
  )
)

(define pi 3.141592)
(define 2pi (* pi 2))
(define (clamp-pi value)
  (if (>= value 0)
    (let* ((numtimes (floor (/ value 2pi))) (remain (- value (* 2pi numtimes))))
      (if (> remain pi) (- remain 2pi) remain)
    )
    (let* ((numtimes (floor (/ value (* -1 2pi)))) (remain (+ value (* 2pi numtimes))))
      (if (< remain (* -1 pi)) (+ remain 2pi) remain)
    )
  )
)
(define max-angleup -1.8)
(define max-angledown 1.1)
(define (set-max-lookangles angleup angledown)
  (set! max-angleup (clamp-pi angleup))
  (set! max-angledown (clamp-pi angledown))
  (format #t "traits: set angle up: ~a, angle down: ~a\n" max-angleup max-angledown)
)

(define xsensitivity 1)
(define ysensitivity 1)
(define (set-sensitivity xvalue yvalue)
  (set! xsensitivity xvalue)
  (set! ysensitivity yvalue)
  (format #t "traits: setting xsensitivity: ~a, ysensitivity: ~a\n" xsensitivity ysensitivity)
)

(define configIndex 0)
(define (nextConfigIndex) (set! configIndex (+ configIndex 1)))
(define (prevConfigIndex) (set! configIndex (max 0 (- configIndex 1))))
(define (update-config)
  (define traits (sql (sql-compile "select speed, speed-air, jump-height, gravity, restitution, friction, max-angleup, max-angledown, mass, jump-sound, land-sound, dash, dash-sound from traits")))
  (define settings (list-ref (sql (sql-compile "select xsensitivity, ysensitivity from settings")) 0))
  (if (= (length traits) 0)
    (display "no traits in config\n")
    (if (>= configIndex (length traits))
      (set! configIndex (max 0 (- (length traits) 1)))
      (let ((targettrait (list-ref traits configIndex)))
        (format #t "Settings config to index: ~a\n" configIndex)
        (set-movement-speed (string->number (list-ref targettrait 0)) (string->number (list-ref targettrait 1)))
        (set-jump-config 
          (string->number (list-ref targettrait 2))
          (list-ref targettrait 9)
          (list-ref targettrait 10)
          (string->number (list-ref targettrait 11))
          (list-ref targettrait 12)
        )
        (set-object-values 
          (string->number (list-ref targettrait 3))
          (string->number (list-ref targettrait 4))
          (string->number (list-ref targettrait 5))
          (string->number (list-ref targettrait 8))
        )
        (set-max-lookangles (string->number (list-ref targettrait 6)) (string->number (list-ref targettrait 7)))
      )
    )
  )
  (set-sensitivity (string->number (car settings)) (string->number (cadr settings)))  
)

(define is-grounded #f)
(define grounded-id #f)
(define (onCollideEnter obj hitpoint normal) 
  (define ycomponent (cadr normal))
  (if (< ycomponent 0)
    (begin
      (if (not is-grounded) (land))
      (set! is-grounded #t)
      (set! grounded-id (gameobj-id obj))
    )
  )
)
(define (onCollideExit obj)
  (format #t "onCollideExit ~a" obj)
  (if (equal? (gameobj-id obj) grounded-id)
    (begin
      (set! is-grounded #f)
      (set! grounded-id #f)
    )
  )
)

(define (jump)
  (define impulse (list 0 (* jump-height) 0))
  (format #t "impulse is: ~a\n" impulse)
  (if is-grounded
    (begin
      (applyimpulse mainobj impulse)
      (if jump-obj-name (playclip jump-obj-name))  
    )
  )
)
(define (land) (if land-obj-name (playclip land-obj-name)))

; Would be cool to make this change velocity into the direction of the normal of the surface'
; Allowing change of direction

(define dull-factor 0.4)
(define (dull-vel velocity) (list 
  (* dull-factor (car velocity)) 
  (* dull-factor (cadr velocity)) 
  (* dull-factor (caddr velocity))
))

(define (set-velocity gameobj velocity) (gameobj-setattr! gameobj (list (list "physics_velocity" (dull-vel velocity)))))
(define (apply-turn oldvelocity hitpoint pos)
  (define newVelocity (reflect oldvelocity (move-relative (list 0 0 0) (caddr hitpoint) 1)))
  (set-velocity mainobj newVelocity)
  (format #t "should apply turn about: ~a\n" hitpoint)
  (format #t "velocity in: ~a\n" oldvelocity)
  (format #t "new angle out: ~a\n" newVelocity)
  (if debugmode
    (draw-line pos (move-relative pos (orientation-from-pos (list 0 0 0) newVelocity) 10) #t)
  )
)

(define debugmode (args "debug"))
(define dashline #f)
(define dashwindow 10)
(define (dashturn pos rot)
  (if (not is-grounded)
    (let* (
        (movingDirection (orientation-from-pos (list 0 0 0) velocity))
        (hitpoints (raycast pos movingDirection dashwindow))
      )
      (format #t "hitpoints: ~a\n" hitpoints)
      (if (> (length hitpoints) 0) (apply-turn velocity (car hitpoints) pos))
      (if debugmode 
        (begin
          (if dashline (free-line dashline))
          (set! dashline (draw-line pos (move-relative pos movingDirection 10) #t))
        )
      )
    )
  )
)

(define last-dash-time -1000000)
(define (dash)
  (define now (time-seconds))
  (define rot (gameobj-rot mainobj))
  (define impulse (move-relative (list 0 0 0) rot jump-height))
  (format #t "dashimpulse: ~a\n" impulse)
  (if (and can-dash (> (- now last-dash-time) dash-delay))
    (begin
      (applyimpulse mainobj impulse)
      (set! last-dash-time now)
      (dashturn lastpos rot)
      (if dash-obj-name (playclip dash-obj-name))
    )
  )
)

(define xrot 0)  ; up and down
(define yrot 0)  ; left and right
(define look-velocity-x 0)
(define look-velocity-y 0)
(define forwardvec (orientation-from-pos (list 0 0 0) (list 0 0 -1)))
(define (look elapsedTime ironsight ironsight_turn) 
  (define raw_deltax (* look-velocity-x xsensitivity elapsedTime))
  (define raw_deltay (* -1 look-velocity-y ysensitivity elapsedTime))
  (define deltax (if ironsight (* raw_deltax ironsight_turn) raw_deltax))
  (define deltay (if ironsight (* raw_deltay ironsight_turn) raw_deltay))
  (define targetxrot (clamp-pi (+ xrot deltax)))
  (define targetyrot (clamp-pi (+ yrot deltay)))
  (set! xrot targetxrot)
  (set! yrot (min max-angledown (max max-angleup targetyrot)))
  (gameobj-setrot! mainobj (setfrontdelta forwardvec xrot yrot 0))
  (reset-look-velocity)
)
(define (reset-look-velocity) 
  (set! look-velocity-x 0)
  (set! look-velocity-y 0)
)
(define (onMouseMove x y)
  (set! look-velocity-x x)
  (set! look-velocity-y y)
)

(define (onKey key scancode action mods) 
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
  (if (and (= key 264) (= action 1)) (nextConfigIndex)) ; down arrow
  (if (and (= key 265) (= action 1)) (prevConfigIndex)) ; up arrow
  (if (and (= key 345) (= action 1)) (update-config))   ; right ctrl
  (if (and (= key 32) (= action 1)) (jump)) ; space

  (if (and (= key 331) (= action 1)) (begin  ; /
    (set! xrot 0)
    (set! yrot 0)
  ))
  (if (and (= key 332) (= action 1)) (begin  ; *
    (set! xrot 1.57079632679) ; 90degrees more or less
    (set! yrot 0) 
  ))
  (if (and (= key 340) (= action 1)) (dash))  ; shift
)

(define (move x y z) 
  (applyimpulse-rel mainobj 
    (list 
      (* (time-elapsed) x) 
      (* (time-elapsed) y) 
      (* (time-elapsed) z)
    )
  )
)

(define lastpos (gameobj-pos mainobj))
(define velocity (list 0 0 0))
(define (calc-velocity elapsedTime newpos oldpos)
  (define time (/  1 elapsedTime))
  (list 
    (* time (- (car newpos) (car oldpos))) 
    (* time (- (cadr newpos) (cadr oldpos))) 
    (* time (- (caddr newpos) (caddr oldpos)))
  ) 
)
(define (update-velocity elapsedTime)
  (define currpos (gameobj-pos mainobj))
  (set! velocity (calc-velocity elapsedTime currpos lastpos))
  (set! lastpos currpos)
  ; todo, sendnotify should be able to send any type
  ;(format #t "velocity is: ~a\n" velocity)
  (sendnotify "velocity" (
    string-join 
    (list 
      (number->string (car velocity))
      (number->string (cadr velocity))
      (number->string (caddr velocity))
    ) 
    " "
  ))
)

(define ironsight-mode #f)
(define ironsight-speed 0.2)
(define ironsight-turn 1)
(define (onMessage key value)
  (if (equal? key "ironsight")
    (set! ironsight-mode (equal? value "on"))
  )
)

(define (get-move-speed)
  (define basespeed (if is-grounded movement-speed movement-speed-air))
  (if ironsight-mode (* ironsight-speed basespeed) basespeed)
)

(define go-forward #f)
(define go-left #f)
(define go-backward #f)
(define go-right #f)
(define (onFrame)
  (define elapsedTime (time-elapsed))
  (if using-fpscam
    (begin
      (if (equal? go-forward  #t) (move 0 0 (* -1 (get-move-speed))))
      (if (equal? go-left     #t) (move (* -0.8 (get-move-speed)) 0 0))
      (if (equal? go-right    #t) (move (* 0.8 (get-move-speed)) 0 0))
      (if (equal? go-backward #t) (move 0 0 (get-move-speed)))
      (look elapsedTime ironsight-mode ironsight-turn)
      (update-velocity elapsedTime)
    )
  )
)

(update-config)

