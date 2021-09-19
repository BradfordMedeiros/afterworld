
(define (onMessage key value)
  (if (and (equal? key "hit") (equal? (string->number value) (gameobj-id mainobj)))
    (begin
      (format #t "wants to delete itself: ~a " (gameobj-name mainobj))
      (rm-obj (gameobj-id mainobj))
    )
  )
)