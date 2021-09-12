
(define fire-animation "firegun")
(define (set-animation animationname)
  (set! fire-animation animationname)
  (format #t "traits: fire-animation: ~a\n" animationname)
)

(define (get-parent) (lsobj-name ">maincamera"))

(define (spawn-gun mesh)
  (mk-obj-attr "weapon"       
    (list 
      (list "mesh" mesh)
      (list "position" (list 0 0 0))
    )
  )
)

(define gunid #f)
(define (change-gun modelpath)
  (if (not (equal? gunid #f))
    (rm-obj gunid)
  )
  (let ((id (spawn-gun modelpath)))
    (format #t "the gun id is: ~a, modelpath: ~a\n" id modelpath)
    ;(make-parent gunid (gameobj-id (get-parent)))
    (set! gunid id)
  )
)

(define (handleChangeGun gunname)
  (define query (string-append "select modelpath, fire-animation from guns where name = " gunname ))  ;; This line is a sql bug, notice the absence of quotes around the gun name
  (define gunstats (sql (sql-compile query)))
  (if (= (length gunstats) 0)
    (format #t "warning: no gun named: ~a\n" gunname)
    (let ((guninfo (car gunstats)))
      (change-gun (list-ref guninfo 0))
      (set-animation (list-ref guninfo 1))
    )
  )
)

(define (onMessage key value)
  (if (equal? key "changegun")
    (begin
      (format #t "changing gun to: ~a\n" value)
      (handleChangeGun value)
    )
  )
)

(define (fire-gun)
  (gameobj-playanimation (gameobj-by-id gunid) fire-animation)
)
(define (onMouse button action mods) 
  (if (and (= button 0) (= action 1))
    (fire-gun)
  )
)

(onMessage "changegun" "pistol")