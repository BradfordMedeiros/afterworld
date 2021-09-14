(define onmenu #f)
(define menu-index 0)
(define levels (sql (sql-compile "select filepath, name from levels")))

(define (next-level) (set! menu-index (min (- (length levels) 1) (+ menu-index 1))))
(define (prev-level) (set! menu-index (max 0 (- menu-index 1))))
(define (selected-level) (car (list-ref levels menu-index)))

(define (find-index searchin value index)
  (if (>= index (length searchin))
    #f
    (if (equal? (cadr (list-ref searchin index)) value)
      index
      (find-index searchin value (+ index 1))
    )
  )
)
(define (set-level-by-name levelname) 
  (define index (find-index levels levelname 0))
  (if index
    (begin
      (set! menu-index index)
      #t
    )
    #f
  )
)

(define levelselect (machine
  (list
    (state "main-menu"
      (list
        (create-track "load-menu"
          (list
            (lambda () (display "LEVEL SELECT: loading menu\n"))
            (lambda () (set! onmenu #t))
            (lambda () 
              (let ((sceneId (load-scene "../afterworld/scenes/menu.rawscene")))
                (begin
                  (display (string-append "scene id: " (number->string sceneId)))
                  (set-camera (gameobj-id (lsobj-name ">maincamera" sceneId)))
                )
              )
            )
          )
        )
        (on-exit
          (list
            (lambda () (display "on exit main menu\n"))
            (lambda () (set! onmenu #f))
            (lambda () (unload-all-scenes))
          )
        )
      )
    )
    (state "playing-level"
      (list
        (create-track "load-level"
          (list
            (lambda () (display "LEVEL SELECT: loading level\n"))
            (lambda () 
              (let ((sceneId (load-scene (selected-level))))
                (begin
                  (display (string-append "scene id: " (number->string sceneId)))
                  (set-camera (gameobj-id (lsobj-name ">maincamera" sceneId)))
                )
              )
            )
          )
        )
        (on-exit
          (list
            (lambda () (display "on exit playing level\n"))
            (lambda () (unload-all-scenes))
          )
        )
      )
    )
  )
))

(define (onMessage topic value)
  (if (equal? topic "reset") 
    (set-machine levelselect "main-menu")
  )
)

(define (playlevel) (set-machine levelselect "playing-level"))
(define (onKey key scancode action mods)
  (if (unlock "input")
    (begin
      (if (and (equal? key 259) (equal? action 1)) (set-machine levelselect "main-menu"))
      (if (and (equal? key 257) (equal? action 1)) (playlevel))
      (if (and (equal? key 265) (equal? action 1)) (prev-level))
      (if (and (equal? key 264) (equal? action 1)) (next-level))  
    )
  )
)

(define (text current-index)
  (define level  (car (list-ref levels current-index)))
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

(define level (args "level"))
(if (string? level) 
  (if (set-level-by-name level)
    (playlevel)
    (display (string-append "ERROR: no level named: " level "\n"))
  )
)
