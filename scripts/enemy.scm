
(define (rm-self) (rm-obj (gameobj-id mainobj)))

(define (onMessage topic)
  (if (equal? topic "bulletraycast") (rm-self))
)

(define (filterMainObj obj1 obj2 fn)
  (define id (gameobj-id mainobj))
  (define otherObject #f)
  (if (equal? (gameobj-id obj1) id) (set! otherObject obj2))
  (if (equal? (gameobj-id obj2) id) (set! otherObject obj1))
  (if otherObject (fn otherObject))
)

(define (is-bullet obj) (string-contains (gameobj-name obj) "name"))            ; TODO -> better if used labels instead, not good to rely on name
(define (process-hit obj) (if (is-bullet obj) (rm-self)))
(define (onCollideEnter obj1 obj2 x) (filterMainObj obj1 obj2 process-hit))

