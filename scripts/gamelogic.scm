
; when the player dies, we can maybe respawn them 
; maybe implement a basic life system here? 
; basic session info b/c whatever why not 
; mainly just have example here of how we can have different "game modes"

; for example, in a mp style session this would handle respawning things at respawn points or whatever

(define (onMessage topic value)
  (if (and (equal? topic "killed") (equal? value "player")) 
    (begin
      (set! num-times-killed (+ 1 num-times-killed))
      (if (equal? num-times-killed 10)
        (sendnotify "reset" "")
      )
    )
  )
)


(define num-times-killed 0)
(define (render-score)
  (draw-text "test mode" 400 10 4)
  (draw-text (string-append "num times killed: " (number->string num-times-killed) "\n") 400 20 4)
)

(define (onFrame)
  (render-score)
)