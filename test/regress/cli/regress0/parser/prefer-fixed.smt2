; EXPECT: sat
(set-logic QF_LIA)
(declare-fun p () Bool)
(declare-fun q () Bool)
(declare-fun x () Int)
; Preprocessing fixes p to true, so no decision is ever made on it and the hint
; on it is vacuous. It is dropped silently, and q is still honored.
(assert p)
(assert (or q (> x 0)))
(prefer p)
(prefer q)
(check-sat)
