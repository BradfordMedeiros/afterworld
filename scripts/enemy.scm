
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