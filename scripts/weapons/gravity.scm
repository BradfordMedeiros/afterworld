


(define (id-from-hitpoints hitpoints)
  (if (< (length hitpoints) 1)
    #f
    (gameobj-by-id (car (list-ref hitpoints 0)))
  )
)

(define (is-dynamic obj)
  (define objattr (gameobj-attr obj))
  (define physicsType (assoc "physics_type" objattr))
  (format #t "objattr: ~a\n" objattr)
  (if physicsType
    (equal? (cadr physicsType) "dynamic")
    #f
  )
)

(define heldobj #f)
(define (release-object)
  (format #t "release object placeholder\n")
  (set! heldobj #f)
)

(define (update-obj-position)
  (if heldobj
    (begin
      (format #t "updating obj position!\n")
      (gameobj-setpos! heldobj (move-relative (gameobj-pos-world mainobj) (gameobj-rot-world mainobj) 3))
    )
  )
)

(define (onFrame)
  (update-obj-position)
)

(define (set-hold-object obj)
  (format #t "should hold: ~a\n" (gameobj-name obj))
  (set! heldobj obj)
)

(define grab-length 20)
(define (grab-object)
  (define hitpoints (raycast (gameobj-pos-world mainobj) (gameobj-rot-world mainobj) grab-length)) 
  (define hitobj (id-from-hitpoints hitpoints))  
  (define isdynamic (and hitobj (is-dynamic hitobj)))
  (if hitobj
    (format #t "hitobj name: ~a\n" (gameobj-name hitobj))
    (format #t "did not hit an object\n")
  )
  (if (and isdynamic hitobj)
    (begin
      (format #t "is dynamic: ~a\n" isdynamic)
      (set-hold-object hitobj)
    )
  )
;  gameobj-attr
  
  (format #t  "mainobj name: ~a\n" (gameobj-name mainobj))
  (draw-line 
    (gameobj-pos-world mainobj) 
    (move-relative 
      (gameobj-pos-world mainobj) 
      (gameobj-rot-world mainobj) 
      grab-length
    )
    #t
  )

)

(define (onMouse button action mods) 
  (if (and (= button 0) (= action 0))
    (release-object)
  )
  (if (and (= button 0) (= action 1))
    (grab-object)
  )
)

