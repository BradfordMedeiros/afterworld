(define (get-target-pos) (gameobj-pos-world (lsobj-name ">maincamera")))

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

(define (onFrame)
  (define targetpos (get-target-pos))
  (if (not (is-close-enough targetpos))
    (move-toward (get-target-pos))
    (fire-bullet)
  )
)