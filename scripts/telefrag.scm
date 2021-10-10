
(define (teleport)
  (define mainpos (gameobj-pos mainobj))
  (define newpos (list (car mainpos) (+ (cadr mainpos) 5) (caddr mainpos)))
  (gameobj-setpos! (lsobj-name ">maincamera") newpos)
)

(define (onKeyChar key)
  (if (= key 116)
    (begin
      (format #t "should teleport now!\n")
      (teleport)
      (rm-obj (gameobj-id mainobj))
    )
  )
)
