(define cyclecolors (list 
  (list 0.1 0.1 0.1)
  (list 0.8 0.8 0.8)
  (list 0.8 0.8 0.8)
  (list 0.8 0.8 0.8)
  (list 1 0.8 0.8)
  (list 0.8 0.8 0.8)
  (list 0.1 0.1 0.3)
  (list 0.1 0.1 0.3)
  (list 0.1 0.1 0.1)
))

(define cycleLengthSecs 20)
(define unitCycleLength (/ cycleLengthSecs (length cyclecolors)))

(define (cycle-index) 
  (modulo (inexact->exact (floor (/ (time-seconds) unitCycleLength))) (length cyclecolors))
)
(define (calcLerpAmount)
  (define currtime (time-seconds))
  (define intamount (* unitCycleLength (floor (/ (time-seconds) unitCycleLength))))
  (define lerpamount (/ (- currtime intamount) unitCycleLength)) 
  lerpamount
)

(define (onFrame)
  (define currCycle (cycle-index))
  (define nextCycle (if (>= (+ 1 currCycle) (length cyclecolors)) 0 (+ 1 currCycle)))
  (define lerpamount (calcLerpAmount))
  (define color (lerp (list-ref cyclecolors currCycle) (list-ref cyclecolors nextCycle) lerpamount))

  (format #t "color: ~a, a: ~a\n" color lerpamount)
  (set-wstate (list 
    (list "skybox" "color" color)
  ))
)