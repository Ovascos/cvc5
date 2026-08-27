; EXPECT: (error "preferred term (> x 0) is not a Boolean variable, ignoring")
; EXPECT: (error "preferred term x is not a Boolean variable, ignoring")
; EXPECT: sat
(set-logic QF_LIA)
(declare-fun a () Bool)
(declare-fun x () Int)
(assert (or a (> x 0)))
; (> x 0) is a Boolean term but not a variable; x is a variable but not Boolean
(prefer a)
(prefer (> x 0))
(prefer x)
(check-sat)
