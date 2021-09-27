(define parent-name ">maincamera")
(define (get-parent) (lsobj-name parent-name))

(define fire-animation #f)
(define (set-animation animationname)
  (define animation (if (> (string-length animationname) 0) animationname #f))
  (set! fire-animation animation)
  (format #t "traits: fire-animation: ~a\n" animation)
)

(define firing-rate 1000)
(define initFiringTime -10000)
(define last-shooting-time initFiringTime)
(define can-hold #f)
(define is-raycast #f)
(define (set-firing-params rate hold raycast)
  (set! firing-rate rate)
  (set! can-hold hold)
  (set! is-raycast raycast)
  (format #t "traits: firing-params - rate: ~a, can-hold: ~a, raycast: ~a\n" firing-rate can-hold is-raycast)
)

(define gunid #f)
(define initial-gun-pos (list 0 0 0))

(define (change-gun modelpath xoffset yoffset zoffset xrot yrot zrot xscale yscale zscale)
  (define gunpos (list xoffset yoffset zoffset))
  (if (not (equal? gunid #f)) (rm-obj gunid))
  (let ((id (mk-obj-attr "weapon"       
    (list 
      (list "mesh" modelpath)
      (list "position" gunpos)
      (list "rotation" (string-join (list xrot yrot zrot) "")) ; rotation is strings...needs to change in engine..
      (list "scale" (list xscale yscale zscale))
      (list "physics" "disabled")
      (list "layer" "no_depth")
    )
  )))
    (set! gunid id)
    (set! initial-gun-pos gunpos)
    (format #t "the gun id is: ~a, modelpath: ~a\n" id modelpath)
    (make-parent gunid (gameobj-id (get-parent)))
  )
)

(define soundid #f)
(define (change-sound clip)
  (if (not (equal? soundid #f))
    (rm-obj soundid)
  )
  (let ((id  (mk-obj-attr "&weapon-sound" (list (list "clip" clip)))))
    (set! soundid id)
    (format #t "the sound id is: ~a: ~a\n" soundid clip)
    (make-parent soundid (gameobj-id (get-parent)))    
  )
)

(define emitterid #f)
(define (change-emitter)
  ;(if (not (equal? emitterid #f))
  ;  (rm-obj emitterid)
  ;)
  ;(let 
  ;  ((id (mk-obj-attr "+particles"
  ;    (list 
  ;      (list "position" (list 0 -2 0))
  ;      (list "+mesh" "../gameresources/build/primitives/plane_xy_1x1.gltf")
  ;      (list "+physics_type" "dynamic")
  ;      (list "+physics" "enabled")
  ;      (list "+physics_collision" "nocollide")
  ;      (list "+texture" "../gameresources/textures/iguana.jpg")
  ;      ;(list "state" "disabled")
  ;      (list "rate" 0.5)
  ;      (list "duration" 1)
  ;      (list "limit" 3)
  ;      (list "+lookat" parent-name)
  ;    )
  ;  )))
  ;  (set! emitterid id)  
  ;  (format #t "the emitter id is: ~a\n" emitterid)
  ;  (make-parent emitterid (gameobj-id (get-parent)))
  ;)
  (format #t "change emitter\n")
)

(define (handleChangeGun gunname)
  (define query (string-append "
    select 
      modelpath, fire-animation, fire-sound, xoffset-pos, yoffset-pos, zoffset-pos, 
      xrot, yrot, zrot, xscale, yscale, zscale, firing-rate, hold, raycast
    from guns where name = " gunname)) 
  (define gunstats (sql (sql-compile query)))
  (if (= (length gunstats) 0)
    (format #t "warning: no gun named: ~a\n" gunname)
    (let ((guninfo (car gunstats)))
      (change-gun 
        (list-ref guninfo 0) 
        (string->number (list-ref guninfo 3)) 
        (string->number (list-ref guninfo 4))
        (string->number (list-ref guninfo 5))
        (list-ref guninfo 6)
        (list-ref guninfo 7)
        (list-ref guninfo 8)
        (string->number (list-ref guninfo 9))
        (string->number (list-ref guninfo 10))
        (string->number (list-ref guninfo 11))
      )
      (set-animation (list-ref guninfo 1))
      (set-firing-params 
        (string->number (list-ref guninfo 12))
        (if (equal? (list-ref guninfo 13) "TRUE") #t #f)
        (if (equal? (list-ref guninfo 14) "TRUE") #t #f)
      )
      (set! last-shooting-time initFiringTime)
      (change-sound (list-ref guninfo 2))
      (change-emitter)
    )
  )
)

(define (sendhit hitpoint)
  (sendnotify "hit" (number->string (car hitpoint)))
)

(define (fire-raycast)
  (define mainobjpos (gameobj-pos (get-parent)))
  (define hitpoints (raycast mainobjpos (gameobj-rot (get-parent)) 500))   
  (format #t "hitpoints: ~a\n" hitpoints)
  (draw-line mainobjpos (move-relative mainobjpos (gameobj-rot (get-parent)) 500))
  (for-each sendhit hitpoints)
)
(define (fire-gun)
  (if (not (equal? fire-animation #f))
    (gameobj-playanimation (gameobj-by-id gunid) fire-animation)
  )
  (if soundid (playclip "&weapon-sound"))
  (if emitterid (emit emitterid))
  (if is-raycast (fire-raycast))
)


(define (fire-gun-limited)
  (define elapsedMilliseconds (* 1000 (time-seconds)))
  (define timeSinceLastShot (- elapsedMilliseconds last-shooting-time))
  (define lessThanFiringRate (> timeSinceLastShot firing-rate))
  (if lessThanFiringRate 
    (begin
      (fire-gun)
      (set! last-shooting-time elapsedMilliseconds)
    )
  )
  (format #f "fire gun limited placeholder\n")
)

(define is-holding #f)
(define (onMouse button action mods) 
  (if (and (= button 0) (= action 1))
    (begin
      (set! is-holding #t)
      (if gunid (fire-gun-limited))
    )
  )
  (if (and (= button 0) (= action 0))
    (set! is-holding #f)
  )
)

(define velocity (list 0 0 0))
(define max-mag-sway-x 0)
(define max-mag-sway-y 0)
(define max-mag-sway-z 10)
(define sway-velocity 1)
(define (sway-gun-translation)
  (define relvelocity (move-relative (list 0 0 0) (gameobj-rot (get-parent)) velocity))
  ;; relative velocity is wrong, going forward is negative when look right

  (define sway-amount-x (list-ref relvelocity 0))
  (define limited-sway-x (min max-mag-sway-x (max sway-amount-x (* -1 max-mag-sway-x))))
  (define sway-amount-y (list-ref relvelocity 1))
  (define limited-sway-y (min max-mag-sway-y (max sway-amount-y (* -1 max-mag-sway-y))))
  (define sway-amount-z (list-ref relvelocity 2))
  (define limited-sway-z (min max-mag-sway-z (max sway-amount-z (* -1 max-mag-sway-z))))
  (define targetpos 
    (list
      (+ (* -1 limited-sway-x) (list-ref initial-gun-pos 0))
      (+ (* -1 limited-sway-y) (list-ref initial-gun-pos 1))
      (+ (* -1 limited-sway-z) (list-ref initial-gun-pos 2))
    )
  )
  (gameobj-setpos-rel! 
    (gameobj-by-id gunid) 
    (lerp (gameobj-pos (gameobj-by-id gunid)) targetpos (* (time-elapsed) sway-velocity))
  ) 
  (format #t "relative velocity ~a\n" relvelocity)
)

(define (onFrame)
  (if (and can-hold is-holding) (fire-gun-limited))
  ;(format #t "velocity is: ~a\n" velocity)
  (if gunid (sway-gun-translation))
)

(define (onMessage key value)
  (if (equal? key "hit")
    (format #t "key: ~a, value: ~a\n" key value)
  )
  (if (equal? key "changegun")
    (begin
      (format #t "changing gun to: ~a\n" value)
      (handleChangeGun value)
    )
  )
  (if (equal? key "velocity")
    (set! velocity (map string->number (string-split value #\ )))
  )
)

(define (onKeyChar key)
  (if (equal? key 49) (onMessage "changegun" "pistol"))
  (if (equal? key 50) (onMessage "changegun" "leftypistol"))
  (if (equal? key 51) (onMessage "changegun" "nailgun"))
  (if (equal? key 52) (onMessage "changegun" "machinegun"))
)
