import unittest
from stochastic_lock_audit import ALL,B,V,flood,lock,wave

class LockToy(unittest.TestCase):
    def setUp(self):
        # Top edge: target pen cells 0..3, exit gate 4, external guard 5.
        self.inner=sum(B[:4])
        self.grid=ALL^sum(B[30+x] for x in range(4))
    def test_bit_edges(self):
        self.assertEqual(wave(B[29]),B[28]|B[59])
        self.assertEqual(wave(B[30]),B[0]|B[31]|B[60])
    def test_cow_has_one_successful_step(self):
        evaluator=lock(self.inner,(4,),self.grid,((5,),(0,)),[],1,False)
        self.assertEqual(evaluator(0),[1.]*4)
        self.assertEqual(evaluator(3),[0.,.5,.5,.5])
    def test_pig_two_substeps(self):
        evaluator=lock(self.inner,(4,),self.grid,((5,),(0,)),[],2,False)
        self.assertEqual(evaluator(3)[:2],[0.,.25])
        # Return excursions through the mouth in the same phase are included.
        self.assertGreater(evaluator(3)[2],.25)
    def test_bounded_interference(self):
        evaluator=lock(self.inner,(4,),self.grid,((5,),(0,)),[(6,1)],1,True)
        self.assertEqual(evaluator(3),[0.]*4)
    def test_only_one_cell_missing(self):
        self.assertEqual(flood(B[0],self.grid^B[4]),self.inner)

if __name__=='__main__':
    unittest.main()
