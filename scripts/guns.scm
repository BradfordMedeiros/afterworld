(define debugmode (args "debug"))
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
(define is-ironsight #f)
(define ironsight-offset (list 0 0 0))
(define recoilLength 0.1)
(define recoilPitchRadians 0)  
(define recoilTranslate (list 0 0 20))
(define recoilZoomTranslate (list 0 0 15))

(define gunid #f)
(define initial-gun-pos (list 0 0 0))
(define initial-gun-rot (orientation-from-pos (list 0 0 0) (list 0 0 -1)))

(define (change-gun modelpath xoffset yoffset zoffset xrot yrot zrot xscale yscale zscale)
  (define gunpos (list xoffset yoffset zoffset))
  (if (not (equal? gunid #f)) (rm-obj gunid))
  (let ((id (mk-obj-attr "weapon"       
    (list 
      (list "mesh" modelpath)
      (list "position" gunpos)
      (list "scale" (list xscale yscale zscale))
      (list "physics" "disabled")
      (list "layer" "no_depth")
    )
  )))
    (set! gunid id)
    (set! initial-gun-pos gunpos)
    (set! initial-gun-rot 
      (orientation-from-pos 
        (list 0 0 0) 
        (list 
          (string->number xrot) 
          (string->number yrot)
          (string->number zrot)
        )
      )
    )
    (format #t "the gun id is: ~a, modelpath: ~a\n" id modelpath)
    (gameobj-setrot! (gameobj-by-id id) initial-gun-rot) ; mk-obj-attr has funky format so this for now
    (make-parent gunid (gameobj-id (get-parent)))
  )
)

(define soundid #f)
(define (change-sound clip)
  (if (not (equal? soundid #f))
    (rm-obj soundid)
  )
  (let ((id  (mk-obj-attr "&weapon-sound" (list (list "clip" clip) (list "physics" "disabled")))))
    (set! soundid id)
    (format #t "the sound id is: ~a: ~a\n" soundid clip)
    (make-parent soundid (gameobj-id (get-parent)))    
  )
)

(define defaultParticleAttrs (list
  (list "position" (list 0 -0.1 0))
  (list "physics" "disabled")
  (list "state" "disabled")
))

(define (template sourceString template templateValue)
  (define index (string-contains sourceString template))
  (if index
    (string-replace sourceString templateValue index (+ index (string-length template)))
    sourceString
  )
)

(define reserved-emitter-chars (list "!" "?" "+"))
(define (reserved-emitter-name value) (member (substring value 0 1) reserved-emitter-chars))
(define (template-emit-line line) 
  (define keyname (car line))
  (define value (template (cadr line) "$MAINOBJ" parent-name))
  (define reservedKeyName (reserved-emitter-name keyname))
  (if reservedKeyName
    (list keyname value)
    (list keyname (parse-attr value))
  )
)
(define (split-emit-line emitLine) (template-emit-line (string-split emitLine #\:)))
(define (is-not-whitespace val) (not (equal? (string-trim val) "")))
(define (emitterOptsToList emitterOptions) (map split-emit-line (filter is-not-whitespace (string-split emitterOptions #\;))))

(define emitterid #f)
(define hit-emitterid #f)
(define projectile-emitterid #f)
(define (get-hit-particle-id) hit-emitterid)

(define (create-emitter name attrs oldemitterid)
  (format #t (string-append name " options: [~a]\n") attrs)
  (format #t "attr length ~a\n" (length attrs))
  (if (> (length attrs) 0)
    (begin
      (if (not (equal? (lsobj-name name) #f)) (rm-obj oldemitterid))
      (let 
        ((id (mk-obj-attr name (append defaultParticleAttrs attrs))))
        (format #t "the emitter id is: ~a\n" id)
        (make-parent id gunid)
        id
      )
    )
    #f
  )
)

(define (change-emitter emitterOptions hitParticleOptions projectileOptions)
  (set! emitterid            (create-emitter "+particles"     (emitterOptsToList emitterOptions)     emitterid           ))
  (set! hit-emitterid        (create-emitter "+hit-particles" (emitterOptsToList hitParticleOptions) hit-emitterid       ))
  (set! projectile-emitterid (create-emitter "+projectile"    (emitterOptsToList projectileOptions)  projectile-emitterid))
)

(define (parse-stringvec str) (map string->number (string-split str #\ )))
(define (handleChangeGun gunname)
  (define query (string-append "
    select 
      modelpath, fire-animation, fire-sound, xoffset-pos, yoffset-pos, zoffset-pos, 
      xrot, yrot, zrot, xscale, yscale, zscale, firing-rate, hold, raycast, 
      ironsight, iron-xoffset-pos, iron-yoffset-pos, iron-zoffset-pos, particle, hit-particle,
      recoil-length, recoil-angle, recoil, recoil-zoom, projectile
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
      (set-animation     (list-ref guninfo 1))

      ; Firing Parameters
      (set! firing-rate  (string->number (list-ref guninfo 12)))
      (set! can-hold     (if (equal? (list-ref guninfo 13) "TRUE") #t #f))
      (set! is-raycast   (if (equal? (list-ref guninfo 14) "TRUE") #t #f))
      (set! is-ironsight (if (equal? (list-ref guninfo 15) "TRUE") #t #f))
      (set! ironsight-offset (list 
        (string->number (list-ref guninfo 16))
        (string->number (list-ref guninfo 17))
        (string->number (list-ref guninfo 18))
      ))
      (set! recoilLength (string->number (list-ref guninfo 21)))
      (set! recoilPitchRadians (string->number (list-ref guninfo 22)))
      (set! recoilTranslate (parse-stringvec (list-ref guninfo 23)))
      (set! recoilZoomTranslate (parse-stringvec (list-ref guninfo 24)))
      (format #t "traits: firing-params - rate: ~a, can-hold: ~a, raycast: ~a, ironsight: ~a, ironsight-offset: ~a\n" firing-rate can-hold is-raycast is-ironsight ironsight-offset)
      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      (set! last-shooting-time initFiringTime)
      (change-sound (list-ref guninfo 2))
      (change-emitter 
        (list-ref guninfo 19)
        (list-ref guninfo 20)
        (list-ref guninfo 25)
      )
    )
  )
)

(define hitlength 3)
(define (debug-hitmarker loc)
  (draw-line 
    (list (+ (car loc)) (+ (cadr loc) (* -1 hitlength)) (+ (caddr loc)))
    (list (+ (car loc)) (+ (cadr loc) hitlength) (+ (caddr loc)))
    #t
  )
  (draw-line 
    (list (+ (car loc) (* -1 hitlength)) (+ (cadr loc)) (+ (caddr loc)))
    (list (+ (car loc) hitlength) (+ (cadr loc)) (+ (caddr loc)))
    #t
  )
  (draw-line 
    (list (+ (car loc)) (+ (cadr loc)) (+ (caddr loc) (* -1 hitlength)))
    (list (+ (car loc)) (+ (cadr loc)) (+ (caddr loc) hitlength))
    #t
  )
)

(define (zfighting-pos pos normal) (move-relative pos normal 0.01))
(define (sendhit hitpoint)
  (define hitemitter (get-hit-particle-id))
  (define hitloc (cadr hitpoint))
  (define hitnormal (caddr hitpoint))
  (sendnotify "hit" (number->string (car hitpoint)))
  (format #t "hitpoint: ~a\n" hitpoint)
  (if hitemitter
    (begin
      (emit hitemitter (zfighting-pos hitloc hitnormal) hitnormal)
      (if debugmode
        (debug-hitmarker hitloc)
      )
    )
  )
)

; TODO -> Consider including translation/rotational sway
; probably just the delta(rot) added to the gun
; but then consider the ads is inaccurate, so I probably should introduce ADs adjustment for raycast centering 
; but maybe just visually make the model line up to the crosshair?
(define raycastline #f)
(define (fire-raycast)
  (define mainobjpos (gameobj-pos (get-parent)))
  (define hitpoints (raycast mainobjpos (gameobj-rot (get-parent)) 500))   
  (format #t "hitpoints: ~a\n" hitpoints)
  (if debugmode
    (begin
      (if raycastline (free-line raycastline))
      (set! raycastline (draw-line mainobjpos (move-relative mainobjpos (gameobj-rot (get-parent)) 500) #t))

    )
  )
  (for-each sendhit hitpoints)
)

(define recoilStart -100)
(define (calc-recoil-slerpamount) 
  (define amount (/ (- (time-seconds) recoilStart) recoilLength))
  (if (> amount 1) 0 amount)
)
(define (recoil-finished) (> (- (time-seconds) recoilStart) recoilLength))
(define (start-recoil) (set! recoilStart (time-seconds)))

(define (fire-gun)
  (if (not (equal? fire-animation #f))
    (gameobj-playanimation (gameobj-by-id gunid) fire-animation)
  )
  (if soundid (playclip "&weapon-sound"))
  (if emitterid (emit emitterid))
  (if projectile-emitterid (emit projectile-emitterid (gameobj-pos (lsobj-name parent-name)) (gameobj-rot (lsobj-name parent-name))))
  (if is-raycast (fire-raycast))
  (start-recoil)
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
(define is-holding-left #f)
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
  (if (and (= button 1) (= action 1))
    (begin
      (set! is-holding-left #t)
      (sendnotify "ironsight" "on")
    )
  )
  (if (and (= button 1) (= action 0))
    (begin
      (set! is-holding-left #f)
      (sendnotify "ironsight" "off")
    )
  )
)

(define max-mag-sway-x 1)
(define max-mag-sway-y 1)
(define max-mag-sway-z 1)
(define sway-velocity 3)

(define (locationWithRecoil targetpos zoomgun)
  (define recoilAmount (if zoomgun recoilZoomTranslate recoilTranslate))
  (define targetposWithRecoil (list 
    (+ (car targetpos)   (car recoilAmount))
    (+ (cadr targetpos)  (cadr recoilAmount)) 
    (+ (caddr targetpos) (caddr recoilAmount))
  ))
  ;(format #t "targetpos: ~a\n" targetposWithRecoil)
  (lerp targetpos targetposWithRecoil (calc-recoil-slerpamount))
)

(define (sway-gun-translation relvelocity zoomgun)
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

  (if zoomgun
    (gameobj-setpos-rel! 
      (gameobj-by-id gunid) 
      (lerp (gameobj-pos (gameobj-by-id gunid)) (locationWithRecoil ironsight-offset #t) (* (time-elapsed) 10))
    ) 
    (gameobj-setpos-rel! 
      (gameobj-by-id gunid) 
      (lerp (gameobj-pos (gameobj-by-id gunid)) (locationWithRecoil targetpos #f) (* (time-elapsed) sway-velocity))
    ) 
  )
)

(define max-mag-sway-x-rot 0.1)
(define max-mag-sway-y-rot 0.1)
(define (sway-gun-rotation relvelocity zoomgun)
  (define sway-amount-x (list-ref relvelocity 0))
  (define limited-sway-x (min max-mag-sway-x-rot (max sway-amount-x (* -1 max-mag-sway-x-rot))))
  (define sway-amount-y (list-ref relvelocity 1))
  (define limited-sway-y (min max-mag-sway-y-rot (max sway-amount-y (* -1 max-mag-sway-y-rot))))
  (define recoilAmount (cadr (lerp (list 0 0 0) (list 0 recoilPitchRadians 0) (calc-recoil-slerpamount))))
  (define targetrot (setfrontdelta initial-gun-rot limited-sway-x (+ recoilAmount (* -1 limited-sway-y)) 0))  ; yaw, pitch, roll .  
  ;(format #t "sway x: ~a, sway y: ~a\n" limited-sway-x limited-sway-y)
  (gameobj-setrot! 
    (gameobj-by-id gunid) 
    (slerp (gameobj-rot (gameobj-by-id gunid)) targetrot 0.01)
  )  
)


(define mouse-velocity (list 0 0 0))
(define reset-mouse-velocity #t)
(define (onMouseMove xpos ypos) (set! mouse-velocity (list xpos ypos 0)))

(define velocity (list 0 0 0))
(define sway-from-mouse #f)
(define (get-sway-velocity)
  (if sway-from-mouse
    mouse-velocity
    (move-relative (list 0 0 0) (invquat (gameobj-rot (get-parent))) velocity)
  )
)

(define sway-rotation #t)
(define (sway-gun zoomgun)
  (sway-gun-translation (get-sway-velocity) zoomgun)
  (if sway-rotation
    (sway-gun-rotation mouse-velocity zoomgun)
  )
)

(define (onFrame)
  (if (and can-hold is-holding) (fire-gun-limited))
  ;(format #t "velocity is: ~a\n" velocity)
  (if gunid (sway-gun (and is-holding-left is-ironsight)))
  (if reset-mouse-velocity (set! mouse-velocity (list 0 0 0)))
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
  (if (equal? key 53) (onMessage "changegun" "upistol"))
)
