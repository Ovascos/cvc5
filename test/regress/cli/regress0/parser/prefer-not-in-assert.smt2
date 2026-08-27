; EXPECT: (error "preferred term (not b) does not occur in any assertion, ignoring")
; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
(declare-fun b () Bool)
(assert (or a b))
; b occurs in an assertion, but (not b) does not
(prefer a)
(prefer (not b))
(check-sat)
