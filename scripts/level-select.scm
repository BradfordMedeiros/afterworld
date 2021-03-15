(define onmenu #f)
(define menu-index 0)
(define levels (list 
  "../../afterworld/scenes/sandbox.rawscene"
  "../../afterworld/scenes/sandbox2.rawscene"
))

(define (next-level) (set! menu-index (min (- (length levels) 1) (+ menu-index 1))))
(define (prev-level) (set! menu-index (max 0 (- menu-index 1))))
(define (selected-level) (list-ref levels menu-index))

(define levelselect (machine
  (list
    (state "main-menu"
      (list
        (create-track "load-menu"
          (list
            (lambda () (display "LEVEL SELECT: loading menu\n"))
            (lambda () (set! onmenu #t))
            (lambda () (unload-all-scenes))
            (lambda () (load-scene "../../afterworld/scenes/menu.rawscene"))
            (lambda () (set-camera (gameobj-id (lsobj-name ">maincamera"))))
          )
        )
      )
    )
    (state "playing-level"
      (list
        (create-track "load-level"
          (list
            (lambda () (display "LEVEL SELECT: loading level\n"))
            (lambda () (set! onmenu #f))
            (lambda () (unload-all-scenes))
            (lambda () (load-scene (selected-level)))
            (lambda () (set-camera (gameobj-id (lsobj-name ">maincamera"))))
          )
        )
      )
    )
  )
))

(define (onKey key scancode action mods)
  (display (string-append "key is: " (number->string key) "\n"))
  (if (and (equal? key 259) (equal? action 1)) (set-machine levelselect "main-menu"))
  (if (and (equal? key 257) (equal? action 1)) (set-machine levelselect "playing-level"))
  (if (and (equal? key 265) (equal? action 1)) (prev-level))
  (if (and (equal? key 264) (equal? action 1)) (next-level))  
)

(define (text current-index)
  (define level  (list-ref levels current-index))
  (if (= current-index menu-index) (string-append "-> " level " <-") level)
)
(define (render-menu menu index)
  (if (> (length menu) 0)
    (begin
      (draw-text (text index) 50 (+ 400 (* index 10)) 4)
      (render-menu (cdr menu) (+ index 1))
    )
  )
)

(define (onFrame)
  (if onmenu (render-menu levels 0))
)

(play-machine levelselect)
