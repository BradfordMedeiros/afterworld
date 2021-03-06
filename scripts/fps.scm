; Movement parameters
(define movement-speed 1)
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

;; Movement code
(define (jump amount) (display "jump placeholder\n"))
(define (move x y) (display "move placeholder\n"))
(define (set-gravity vec3) (display "set-gravity placeholder\n"))
(define (play-jump-sound) (display "play jump sound placeholder\n"))

;; Gunplay code
(define (set-gun-model) (display "set gun model placeholder\n"))
(define (set-hud-element) (display "set hud element placeholder\n"))
(define (set-gun-offset x y) (display "set gun offset placeholder \n"))
(define (fire-gun) (display "fire gun placeholder\n"))