; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
(declare-fun b () Bool)
(declare-fun c () Bool)
(assert (or a (not b)))
(assert (=> c a))
; a occurs directly, b and c occur as subterms of the assertions
(prefer a)
(prefer (not b))
(prefer c)
(check-sat)
