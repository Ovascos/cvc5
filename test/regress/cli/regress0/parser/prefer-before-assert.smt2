; EXPECT: (error "preferred variable a does not occur in any assertion, ignoring")
; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
; the assertion mentioning a comes after the prefer, so it is not seen yet
(prefer a)
(assert a)
(check-sat)
