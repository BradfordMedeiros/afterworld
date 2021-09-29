
(define (is-bouncepad) #t)
(define (approach-from-above) #t)
(define (launch-object) (format #t "placeholder launch object\n"))

(define (handle-bounce-attr)
  (if (is-bouncepad)
    (if (approach-from-above)
      (launch-object)
    )
  )
)

(define (onGlobalCollideEnter obj1 obj2 hitpoint normal) 
  (handle-bounce-attr)
)
