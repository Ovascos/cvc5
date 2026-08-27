; REQUIRES: tracing
; COMMAND-LINE: -t prefer
; SCRUBBER: sed -e '/^resolved:/d' -e '/^fixed:/d' -e 's/^SmtSolver: //'
; EXPECT: addPreferTerm (not b)
; EXPECT: addPreferTerm c
; EXPECT: addPreferTerm a
; EXPECT: sat
(set-logic QF_UF)
(declare-fun a () Bool)
(declare-fun b () Bool)
(declare-fun c () Bool)
; c occurs only deep inside the second assertion
(assert (or a (not b)))
(assert (or a (and b (= b c))))
; the preferred terms are recorded in input order, not declaration order
(prefer (not b))
(prefer c)
(prefer a)
(check-sat)
