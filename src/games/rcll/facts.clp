
;---------------------------------------------------------------------------
;  facts.clp - LLSF RefBox CLIPS - facts specification
;
;  Created: Mon Feb 11 13:11:45 2013
;  Copyright  2013  Tim Niemueller [www.niemueller.de]
;  Licensed under BSD license, cf. LICENSE file
;---------------------------------------------------------------------------

(deftemplate machine
  (slot name (type SYMBOL)
	(allowed-values C-BS C-DS C-RS1 C-RS2 C-CS1 C-CS2 M-BS M-DS M-RS1 M-RS2 M-CS1 M-CS2))
  (slot team (type SYMBOL) (allowed-values CYAN MAGENTA))
  (slot mtype (type SYMBOL) (allowed-values BS DS RS CS))
  (multislot actual-lights (type SYMBOL)
	     (allowed-values RED-ON RED-BLINK YELLOW-ON YELLOW-BLINK GREEN-ON GREEN-BLINK)
	     (default) (cardinality 0 3))
  (multislot desired-lights (type SYMBOL)
	     (allowed-values RED-ON RED-BLINK YELLOW-ON YELLOW-BLINK GREEN-ON GREEN-BLINK)
	     (default GREEN-ON YELLOW-ON RED-ON) (cardinality 0 3))
  (slot productions (type INTEGER) (default 0))
  ; Overall refbox machine state
  (slot state (type SYMBOL) (allowed-values IDLE BROKEN PREPARED PROCESSING
					    PROCESSED READY-AT-OUTPUT WAIT-IDLE DOWN))
  ; Set on processing a state change
  (slot proc-state (type SYMBOL) (default IDLE))
  (slot prev-state (type SYMBOL) (default IDLE))
  ; This is the state indicated by the MPS
  (slot mps-state-deferred (type SYMBOL) (default NONE))
  (slot mps-state (type SYMBOL) (default IDLE))
  (slot proc-time (type INTEGER))
  (slot proc-start (type FLOAT))
  (multislot down-period (type FLOAT) (cardinality 2 2) (default -1.0 -1.0))
  (slot broken-since (type FLOAT))
  (slot broken-reason (type STRING))
   ; x y theta (meters and rad)
  (multislot pose (type FLOAT) (cardinality 3 3) (default 0.0 0.0 0.0))
  (multislot pose-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (slot zone (type SYMBOL) (default TBD)
	(allowed-values TBD Z1 Z2 Z3 Z4 Z5 Z6 Z7 Z8 Z9 Z10 Z11 Z12
			Z13 Z14 Z15 Z16 Z17 Z18 Z19 Z20 Z21 Z22 Z23 Z24))
  (slot exploration-light-code (type INTEGER) (default 0))
  (slot exploration-type (type STRING))

  (slot prep-blink-start (type FLOAT))
  (slot retrieved-at (type FLOAT))
  (slot bases-added (type INTEGER) (default 0))
  (slot bases-used (type INTEGER) (default 0))

  ; machine type specific slots
  (slot bs-side (type SYMBOL) (allowed-values INPUT OUTPUT))
  (slot bs-color (type SYMBOL) (allowed-values BASE_RED BASE_BLACK BASE_SILVER))

  (slot ds-gate (type INTEGER))
  (slot ds-last-gate (type INTEGER))

  (slot rs-ring-color (type SYMBOL)
	(allowed-values RING_BLUE RING_GREEN RING_ORANGE RING_YELLOW))
  (multislot rs-ring-colors (type SYMBOL) (default RING_GREEN RING_BLUE)
	     (allowed-values RING_BLUE RING_GREEN RING_ORANGE RING_YELLOW))

  (slot cs-operation (type SYMBOL) (allowed-values RETRIEVE_CAP MOUNT_CAP))
  (slot cs-retrieved (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
)

(deftemplate machine-mps-state
  (slot name (type SYMBOL))
  (slot state (type SYMBOL))
  (slot num-bases (type INTEGER) (default 0))
)

(deftemplate machine-light-code
  (slot id (type INTEGER))
  (multislot code (type SYMBOL) (default)
	     (allowed-values RED-ON RED-BLINK YELLOW-ON YELLOW-BLINK GREEN-ON GREEN-BLINK))
)

(deftemplate robot
  (slot number (type INTEGER))
  (slot state (type SYMBOL) (allowed-values ACTIVE MAINTENANCE DISQUALIFIED) (default ACTIVE))
  (slot team (type STRING))
  (slot team-color (type SYMBOL) (allowed-values nil CYAN MAGENTA))
  (slot name (type STRING))
  (slot host (type STRING))
  (slot port (type INTEGER))
  (multislot last-seen (type INTEGER) (cardinality 2 2))
  (slot warning-sent (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
   ; x y theta (meters and rad)
  (slot has-pose (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
  (multislot pose (type FLOAT) (cardinality 3 3) (default 0.0 0.0 0.0))
  (multislot pose-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (multislot vision-pose (type FLOAT) (cardinality 3 3) (default 0.0 0.0 0.0))
  (multislot vision-pose-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (slot maintenance-start-time (type FLOAT))
  (slot maintenance-cycles (type INTEGER) (default 0))
  (slot maintenance-warning-sent (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
)

(deftemplate robot-beacon
  (multislot rcvd-at (type INTEGER) (cardinality 2 2))

  (slot seq (type INTEGER))
  (multislot time (type INTEGER) (cardinality 2 2))

  (slot number (type INTEGER))
  (slot team-name (type STRING))
  (slot team-color (type SYMBOL) (allowed-values nil CYAN MAGENTA))
  (slot peer-name (type STRING))
  (slot host (type STRING))
  (slot port (type INTEGER))
  (slot has-pose (type SYMBOL) (allowed-values TRUE FALSE) (default FALSE))
  (multislot pose (type FLOAT) (cardinality 3 3) (default 0.0 0.0 0.0))
  (multislot pose-time (type INTEGER) (cardinality 2 2) (default 0 0))
)

(deftemplate signal
  (slot type)
  (multislot time (type INTEGER) (cardinality 2 2) (default (create$ 0 0)))
  (slot seq (type INTEGER) (default 1))
  (slot count (type INTEGER) (default 1))
)

(deftemplate network-client
  (slot id (type INTEGER))
  (slot host (type STRING))
  (slot port (type INTEGER))
  (slot is-slave (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
)

(deftemplate network-peer
  (slot group (type SYMBOL) (allowed-values PUBLIC CYAN MAGENTA))
  (slot id (type INTEGER))
  (slot network-prefix (type STRING))
)

(deftemplate attention-message
  (slot team (type SYMBOL) (allowed-values nil CYAN MAGENTA) (default nil))
  (slot text (type STRING))
  (slot time (type INTEGER) (default 5))
)

(deftemplate order
  (slot id (type INTEGER))

  ; Product specification
  (slot complexity (type SYMBOL) (allowed-values C0 C1 C2 C3))
  ; the following is auto-generated based on the previously defined complexity
  (slot base-color (type SYMBOL) (allowed-values BASE_RED BASE_SILVER BASE_BLACK))
  (multislot ring-colors (type SYMBOL) (cardinality 0 3)
	     (allowed-values RING_BLUE RING_GREEN RING_ORANGE RING_YELLOW))
  (slot cap-color (type SYMBOL) (allowed-values CAP_BLACK CAP_GREY))

  (slot quantity-requested (type INTEGER) (default 1))
  ; Quantity delivered for both teams in the order C,M
  (multislot quantity-delivered (type INTEGER) (cardinality 2 2) (default 0 0))
  ; Time window in which the order should start, used for
  ; randomizing delivery-period's first element (start time)
  (multislot start-range (type INTEGER) (cardinality 2 2))
  ; Time the production should take, used for randomizing the second
  ; element of delivery-period (end time)
  (multislot duration-range (type INTEGER) (cardinality 2 2) (default 60 180))
  ; Time window in which it must be delivered, set during initial randomization
  (multislot delivery-period (type INTEGER) (cardinality 2 2) (default 0 900))
  (slot delivery-gate (type INTEGER))
  (slot active (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
  (slot activate-at (type INTEGER) (default 0))
  (multislot activation-range (type INTEGER) (cardinality 2 2) (default 120 240))
  (slot allow-overtime (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
)

(deftemplate workpiece
	(slot id (type INTEGER))
	(slot rtype (type SYMBOL) (allowed-values INCOMING RECORD))
	(slot at-machine (type SYMBOL)
				(allowed-values C-BS C-DS C-RS1 C-RS2 C-CS1 C-CS2 M-BS M-DS M-RS1 M-RS2 M-CS1 M-CS2))
  (slot team (type SYMBOL) (allowed-values nil CYAN MAGENTA))
  (slot visible (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
)

(deftemplate ring-spec
  (slot color (type SYMBOL) (allowed-values RING_BLUE RING_GREEN RING_ORANGE RING_YELLOW))
  (slot req-bases (type INTEGER) (default 0))
)

(deftemplate delivery-period
  (multislot delivery-gates (type SYMBOL) (allowed-values D1 D2 D3 D4 D5 D6) (cardinality 2 2))
  (multislot period (type INTEGER) (cardinality 2 2))
)  
 
(deftemplate product-delivered
  (slot game-time (type FLOAT))
	(slot order (type INTEGER) (default 0))
  (slot team (type SYMBOL) (allowed-values nil CYAN MAGENTA))
	(slot delivery-gate (type INTEGER))
  (slot base-color (type SYMBOL) (allowed-values BASE_RED BASE_SILVER BASE_BLACK))
  (multislot ring-colors (type SYMBOL) (cardinality 0 3)
	     (allowed-values RING_BLUE RING_GREEN RING_ORANGE RING_YELLOW))
  (slot cap-color (type SYMBOL) (allowed-values CAP_BLACK CAP_GREY))
)

(deftemplate gamestate
  (slot refbox-mode (type SYMBOL) (allowed-values STANDALONE) (default STANDALONE))
  (slot state (type SYMBOL)
	(allowed-values INIT WAIT_START RUNNING PAUSED) (default INIT))
  (slot prev-state (type SYMBOL)
	(allowed-values INIT WAIT_START RUNNING PAUSED) (default INIT))
  (slot phase (type SYMBOL)
	(allowed-values PRE_GAME SETUP EXPLORATION PRODUCTION POST_GAME
			OPEN_CHALLENGE NAVIGATION_CHALLENGE WHACK_A_MOLE_CHALLENGE)
	(default PRE_GAME))
  (slot prev-phase (type SYMBOL)
	(allowed-values NONE PRE_GAME SETUP EXPLORATION PRODUCTION POST_GAME
			OPEN_CHALLENGE NAVIGATION_CHALLENGE WHACK_A_MOLE_CHALLENGE)
	(default NONE))
  (slot game-time (type FLOAT) (default 0.0))
  (slot cont-time (type FLOAT) (default 0.0))
  ; cardinality 2: sec msec
  (multislot start-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (multislot end-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (multislot last-time (type INTEGER) (cardinality 2 2) (default 0 0))
  ; cardinality 2: team cyan and magenta
  (multislot points (type INTEGER) (cardinality 2 2) (default 0 0))
  (multislot teams (type STRING) (cardinality 2 2) (default "" ""))

  (slot over-time (type SYMBOL) (allowed-values FALSE TRUE) (default FALSE))
)

(deftemplate exploration-report
	(slot rtype (type SYMBOL) (allowed-values INCOMING RECORD))
  (slot name (type SYMBOL)
	(allowed-values C-BS C-DS C-RS1 C-RS2 C-CS1 C-CS2 M-BS M-DS M-RS1 M-RS2 M-CS1 M-CS2))
  (slot team (type SYMBOL) (allowed-values CYAN MAGENTA))
  (slot type (type STRING))
  (slot zone (type SYMBOL)
	(allowed-values NOT-REPORTED Z1 Z2 Z3 Z4 Z5 Z6 Z7 Z8 Z9 Z10 Z11 Z12
			Z13 Z14 Z15 Z16 Z17 Z18 Z19 Z20 Z21 Z22 Z23 Z24))
  (slot host (type STRING))
  (slot port (type INTEGER))
  (slot game-time (type FLOAT))
  (slot correctly-reported (type SYMBOL) (allowed-values UNKNOWN TRUE FALSE) (default UNKNOWN))
  (slot zone-state (type SYMBOL) (allowed-values NO_REPORT CORRECT_REPORT WRONG_REPORT) (default NO_REPORT))
  (slot type-state (type SYMBOL) (allowed-values NO_REPORT CORRECT_REPORT WRONG_REPORT) (default NO_REPORT))
)

(deftemplate points
  (slot points (type INTEGER))
  (slot team (type SYMBOL) (allowed-values CYAN MAGENTA))
  (slot game-time (type FLOAT))
  (slot phase (type SYMBOL) (allowed-values EXPLORATION PRODUCTION WHACK_A_MOLE_CHALLENGE))
  (slot reason (type STRING))
)

(deftemplate zone-swap
	(slot m1-name (type SYMBOL))
	(slot m1-new-zone (type SYMBOL))
	(slot m2-name (type SYMBOL))
	(slot m2-new-zone (type SYMBOL))
)

(deftemplate sim-time
  (slot enabled (type SYMBOL) (allowed-values false true) (default false))
  (slot estimate (type SYMBOL) (allowed-values false true) (default false))
  (multislot now (type INTEGER) (cardinality 2 2) (default 0 0))
  (multislot last-recv-time (type INTEGER) (cardinality 2 2) (default 0 0))
  (slot real-time-factor (type FLOAT) (default 0.0))
)

; Machine directions in LLSF arena frame when looking from bird's eye perspective
(defglobal
  ?*M-EAST*   = (* (/ 3.0 2.0) (pi))   ; 270 deg or -90 deg
  ?*M-NORTH*  = 0                      ;   0 deg
  ?*M-WEST*   = (/ (pi) 2.0)           ;  90 deg
  ?*M-SOUTH*  = (pi)                   ; 180 deg
)

(deffacts startup
  (gamestate (phase PRE_GAME))
  (signal (type beacon) (time (create$ 0 0)) (seq 1))
  (signal (type gamestate) (time (create$ 0 0)) (seq 1))
  (signal (type robot-info) (time (create$ 0 0)) (seq 1))
  (signal (type bc-robot-info) (time (create$ 0 0)) (seq 1))
  (signal (type machine-info) (time (create$ 0 0)) (seq 1))
  (signal (type machine-info-bc) (time (create$ 0 0)) (seq 1))
  (signal (type ring-info-bc) (time (create$ 0 0)) (seq 1))
  (signal (type order-info) (time (create$ 0 0)) (seq 1))
  (signal (type machine-report-info) (time (create$ 0 0)) (seq 1))
  (signal (type version-info) (time (create$ 0 0)) (seq 1))
  (signal (type workpiece-info) (time (create$ 0 0)) (seq 1))
  (signal (type exploration-info) (time (create$ 0 0)) (seq 1))
  (signal (type setup-light-toggle) (time (create$ 0 0)) (seq 1))
  (setup-light-toggle CS2)
  (whac-a-mole-light NONE)

  (machine (name C-BS)  (team CYAN) (mtype BS) (zone Z9))
  (machine (name C-DS)  (team CYAN) (mtype DS) (zone Z4))
  (machine (name C-RS1) (team CYAN) (mtype RS))
  (machine (name C-RS2) (team CYAN) (mtype RS))
  (machine (name C-CS1) (team CYAN) (mtype CS))
  (machine (name C-CS2) (team CYAN) (mtype CS))

  (machine (name M-BS)  (team MAGENTA) (mtype BS) (zone Z21))
  (machine (name M-DS)  (team MAGENTA) (mtype DS) (zone Z16))
  (machine (name M-RS1) (team MAGENTA) (mtype RS))
  (machine (name M-RS2) (team MAGENTA) (mtype RS))
  (machine (name M-CS1) (team MAGENTA) (mtype CS))
  (machine (name M-CS2) (team MAGENTA) (mtype CS))

  (ring-spec (color RING_BLUE))
  (ring-spec (color RING_GREEN) (req-bases 1))
  (ring-spec (color RING_ORANGE))
  (ring-spec (color RING_YELLOW))
)

(deffacts light-codes
  (machine-light-code (id 1) (code GREEN-ON))
  (machine-light-code (id 2) (code YELLOW-ON))
  (machine-light-code (id 3) (code GREEN-ON YELLOW-ON))
  (machine-light-code (id 4) (code RED-ON))
  (machine-light-code (id 5) (code RED-ON GREEN-ON))
  (machine-light-code (id 6) (code RED-ON YELLOW-ON))
  (machine-light-code (id 7) (code RED-ON YELLOW-ON GREEN-ON))
  ;(machine-light-code (id 8) (code GREEN-BLINK))
  ;(machine-light-code (id 9) (code YELLOW-BLINK))
  ;(machine-light-code (id 10) (code RED-BLINK))
)

(deffacts orders
  (order (id  1) (complexity C0) (quantity-requested 1) (start-range 0 0)
	       (activation-range 900 900) (duration-range 900 900))
  (order (id  2) (complexity C0) (quantity-requested 1) (start-range 200 400))
  (order (id  3) (complexity C0) (quantity-requested 2) (start-range 300 700))
  (order (id  4) (complexity C0) (quantity-requested 1) (start-range 700 900))
  (order (id  5) (complexity C1) (quantity-requested 1) (start-range 500 700)
	       (activation-range 300 500) (duration-range 90 150))
  (order (id  6) (complexity C2) (quantity-requested 1) (start-range 600 720)
	       (activation-range 480 780) (duration-range 120 180))
  (order (id  7) (complexity C3) (quantity-requested 1) (start-range 600 720)
	       (activation-range 900 900) (duration-range 120 180))
  (order (id  8) (complexity C0) (quantity-requested 1) (start-range 900 900)
         (activation-range 0 0) (duration-range 300 300) (allow-overtime TRUE))
)

