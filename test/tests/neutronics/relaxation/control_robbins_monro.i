!include control_dufek_gudowski.i

# This test computes the values of alpha, which are deterministic based on the number
# of iterations. The first value of alpha is 1.0 (iteration 2), the second value
# of alpha is ~0.5 (iteration 3), and the final value of alpha is ~0.333333
# (iteration 6). Otherwise, alpha should be 1.0 as relaxation is disabled.
[Problem]
  relaxation := 'robbins_monro'
[]

[Outputs]
  hide := 'heat_source p1 p1_prev p1_raw p2 p2_prev p2_raw p3 p3_prev p3_raw particles'
[]
