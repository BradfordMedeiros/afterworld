
(define (amount-to-draw text createTime rate)
  (define currIndex (inexact->exact (floor (* rate (- (time-seconds) createTime)))))
  (substring text 0 (min (string-length text) currIndex))
)

(define maxBufferSize 10)
(define messageBuffer (list))

(define displayMessage (args "alert"))
(define (onMessage key value)
  (if (equal? key "alert")
    (begin
      (set! messageBuffer (reverse (cons (list value (time-seconds)) (reverse messageBuffer)))) 
      (if (> (length messageBuffer) maxBufferSize)
        (set! messageBuffer (cdr messageBuffer))
      )
      (format #t "list is: ~a\n" messageBuffer)
    )
  )
  (if (equal? key displayMessage) 
    (onMessage "alert" (string-append "message: " key value))
  )
)

(define (render-alerts yoffset buffer)
  (if (> (length buffer) 0)
    (begin
      (format #t "rendering alerts: ~a ~a\n" yoffset buffer)
      (draw-text (amount-to-draw (car (car buffer)) (cadr (car buffer)) 100) 50 yoffset 4)
      (render-alerts (+ yoffset 10) (cdr buffer))
    )
  )
)

(define (onFrame)
  (render-alerts 400 messageBuffer)
)

(define crosshairobjname "crosshair")
(define (set-crosshair image)
  (format #t "placeholder set crosshair: ~a\n" image)
  (gameobj-setattr! 
    (lsobj-name "crosshair")
    (list
      (list "texture" image)
    )
  )
)
(define crosshair (args "crosshair"))
(if (string? crosshair) 
  (set-crosshair crosshair)
)
