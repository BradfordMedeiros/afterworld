

(define look-velocity-x 0)
(define look-velocity-y 0)
(define (onMouseMove x y)
  (set! look-velocity-x x)
  (set! look-velocity-y y)
)

(define xrot 0)  ; up and down
(define yrot 0)  ; left and right
(define forwardvec (orientation-from-pos (list 0 0 0) (list 0 0 -1)))
(define (look) 
  (define elapsedTime (time-elapsed))
  (set! xrot (+ xrot (* look-velocity-x elapsedTime)))
  (set! yrot (+ yrot (* -1 look-velocity-y elapsedTime)))
  (gameobj-setrot! mainobj (setfrontdelta forwardvec xrot yrot 0))
  (set! look-velocity-y 0)
  (set! look-velocity-x 0)
  (gameobj-setattr! mainobj 
    (list 
      (list "physics_velocity" (move-relative (list 0 0 0) (gameobj-rot mainobj) 50))
    )
  )
)

(define (onCollideEnter obj hitpoint normal) 
  (format #t "on collision!\n")
  (reset-camera)
)

(define (onFrame) 
  (look)
  (draw-text "////////MISSLE MODE////////" 50 900 5)
)

(define smartcam 
  (mk-obj-attr 
    ">smartcamera" 
    (list 
      (list "position" (list 0 0 10))
      (list "physics" "disabled")
    )
  )
)

(make-parent smartcam (gameobj-id mainobj))
(set-camera smartcam 2)

(define (reset-camera) (set-camera (gameobj-id (lsobj-name ">maincamera")) 2))
(define (beforeUnload) (reset-camera))

