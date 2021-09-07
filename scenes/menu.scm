
(define ypos 0)
(define redness 0.2)
(define gettingred #t)
(define lastTime (time-seconds))
(define elapsedTime 0)

(if (not (args "silent"))
  (playclip "&music")
)

(define (onFrame)
  (define currTime (time-seconds))
  (define delta (* elapsedTime 0.01))
  (define colordelta (* elapsedTime 0.1))

  (set! elapsedTime (- currTime lastTime))
  (set! lastTime currTime)

  (set! ypos (+ ypos delta))
  (if (> ypos 1)
    (set! ypos 0)
  )
  (if gettingred
    (begin 
      (set! redness (+ redness colordelta))
      (if (> redness 1)
        (set! gettingred #f)
      )
    )
    (begin 
      (set! redness (- redness colordelta))
      (if (< redness 0.1)
        (set! gettingred #t)
      )
    )
  )

  (gameobj-setattr! 
    mainobj
    (list
      (list "textureoffset" (string-join (list "0" (number->string ypos)) " "))
      (list "tint" (list (+ redness 1) 0.4 0.4))
    )
  )
)