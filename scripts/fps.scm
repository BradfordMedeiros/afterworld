; Movement parameters
(define movement-speed 0.1)
(define jump-height 1)
(define gravity 1)
(define tilt (list 0 0))
(define jump-sound-effect #f)

; Gun parameters
(define gun-name "default-gun")
(define gun-model "")
(define hud-element "")
(define x-offset-percentage 0)
(define y-offset-percentage 0)
(define firing-rate 400)
(define can-hold #f)
(define spread 0)
(define max-ammo 10)
;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Movement code
(define (jump amount) (display "jump placeholder\n"))
(define (move x y z) (applyimpulse-rel mainobj (list x y (* -1 z))))
(define (look x y) (gameobj-setrotd! mainobj (* 0.1 x) (* -0.1 y) 0))

(define (set-gravity vec3) (display "set-gravity placeholder\n"))
(define (play-jump-sound) (display "play jump sound placeholder\n"))

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
)


