
(define (amount-to-draw text rate)
  (define currIndex (inexact->exact (floor (* rate (time-seconds)))))
  (substring text 0 (min (string-length text) currIndex))
)

(define maxBufferSize 10)
(define messageBuffer (list))
(define (onMessage key value)
  (format #t "the message is: ~a ~a\n" key value)
  
  (set! messageBuffer (append messageBuffer (list key value)))
  

  (if (> (length messageBuffer) maxBufferSize)
    (set! messageBuffer (cdr messageBuffer))
  )
  (format #t "list size is: ~a\n" (length messageBuffer))
)

(define (onFrame)
  (draw-text (amount-to-draw "hello world" 5) 50 400 4)
)

