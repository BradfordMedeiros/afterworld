
(define (rm-self) (rm-obj (gameobj-id mainobj)))
(define (onMessage topic value)
  (if (and (equal? topic "bullet") (equal? (string->number value) (gameobj-id mainobj))) 
    (rm-self)
  )
)

(define (filterMainObj id obj1 obj2 fn)
  (define otherObject #f)
  (if (equal? (gameobj-id obj1) id) (set! otherObject obj2))
  (if (equal? (gameobj-id obj2) id) (set! otherObject obj1))
  (if otherObject (fn otherObject))
)

(define (is-bullet obj) (string-contains (gameobj-name obj) "name"))            ; TODO -> better if used labels instead, not good to rely on name
(define (process-hit obj) (if (is-bullet obj) (rm-self)))

(define current-target #f)
(define (is-target obj) (string-contains (gameobj-name obj) "main"))            ; TODO -> better if used labels instead, not good to rely on name
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
(define (set-target obj)
  (display "==============found a target!============\n")
  (set! current-target obj)
  (set-machine enemy "attacking")
)
(define (process-detection obj) 
  (if (is-target obj) 
    (if (is-visible obj)  
      (set-target obj)
      (display "it is not visibile\n")
    )
  )
)

(define (onCollideEnter obj1 obj2 x normal) 
  (filterMainObj (gameobj-id mainobj) obj1 obj2 process-hit)
  (filterMainObj (gameobj-id (lsobj-name "enemy_detection")) obj1 obj2 process-detection)
)

(define enemy (machine
  (list
    (state "just-chilling"
      (list
        (create-track "doing-nothing"
          (list
            (lambda () (display "ENEMY, just kind of chilling yo\n"))
          )
        )
      )
    )
    (state "attacking"
      (list
        (create-track "attack"
          (list
            (lambda () (display "ENEMY, about to attack!\n"))
          )
        )
      )
    )
  )
))

(define (move-toward target)
  (define currPos (gameobj-pos mainobj))
  (gameobj-setpos! mainobj (move-relative currPos (orientation-from-pos currPos (gameobj-pos target)) 0.01))
)
(define (distance point1 point2)
  (define xdelta (- (car  point1) (car  point2)))
  (define ydelta (- (cadr point1) (cadr point2)))
  (define zdelta (- (caddr point1) (caddr point2)))
  (sqrt (+ (* xdelta xdelta) (* ydelta ydelta) (* zdelta zdelta)))
)
(define (is-close-enough target)
  (< (distance (gameobj-pos target) (gameobj-pos mainobj)) 10)
)

(define (onFrame)
  (if (not (equal? current-target #f))
    (if (not (is-close-enough current-target))
      (move-toward current-target)
    )
  )
)

(play-machine enemy)


