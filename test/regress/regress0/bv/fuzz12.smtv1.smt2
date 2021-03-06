(set-option :incremental false)
(set-info :status sat)
(set-logic QF_BV)
(declare-fun v1 () (_ BitVec 9))
(declare-fun v2 () (_ BitVec 10))
(declare-fun v0 () (_ BitVec 3))
(check-sat-assuming ( (let ((_let_0 (bvneg v1))) (let ((_let_1 (bvlshr v2 ((_ zero_extend 1) v1)))) (let ((_let_2 (ite (bvsge (bvadd v1 (_ bv1 9)) ((_ zero_extend 6) v0)) (_ bv1 1) (_ bv0 1)))) (=> (= (_ bv1 3) v0) (xor (or (= (_ bv0 9) (ite (= (_ bv1 1) ((_ extract 9 9) ((_ sign_extend 2) v2))) _let_0 ((_ zero_extend 8) (ite (bvsgt (_ bv1 11) ((_ zero_extend 8) v0)) (_ bv1 1) (_ bv0 1))))) (= (ite (bvult (_ bv1 12) ((_ zero_extend 3) v1)) (_ bv1 1) (_ bv0 1)) (ite (= ((_ zero_extend 2) _let_1) (_ bv0 12)) (_ bv1 1) (_ bv0 1)))) (ite (= (_ bv0 12) ((_ sign_extend 9) v0)) (= (_ bv1 1) (bvcomp (_ bv0 10) (bvmul _let_1 ((_ zero_extend 1) ((_ rotate_left 3) _let_0))))) (= (_ bv0 10) (ite (= (_ bv1 1) (bvnor _let_2 _let_2)) (_ bv0 10) _let_1)))))))) ))
