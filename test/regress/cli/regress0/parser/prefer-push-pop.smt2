; COMMAND-LINE: --incremental
; EXPECT: sat
; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
(declare-fun c () Bool)
(assert (or a c))
(push 1)
(declare-fun b () Bool)
(assert (or a b))
; b only exists inside this scope, so this hint must not outlive the pop
(prefer b)
(check-sat)
(pop 1)
(prefer a)
(check-sat)
