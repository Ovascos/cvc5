; EXPECT: (error "preferred variable c does not occur in any assertion, ignoring")
; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
(declare-fun b () Bool)
(declare-fun c () Bool)
(assert (or a b))
; (not b) is accepted: only the variable b has to occur, not the negation.
; c does not occur in any assertion at all.
(prefer (not b))
(prefer c)
(check-sat)
