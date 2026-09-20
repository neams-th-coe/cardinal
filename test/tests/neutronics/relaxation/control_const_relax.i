!include control_dufek_gudowski.i

# This test computes the values of alpha, which are deterministic based on the number
# of iterations. The value of alpha should be 0.5 on iterations 2, 3, and 6.
# Otherwise, alpha should be 1.0 as relaxation is disabled.
[Problem]
  relaxation := 'constant'
[]

[Outputs]
  hide := 'heat_source p1 p1_prev p1_raw p2 p2_prev p2_raw p3 p3_prev p3_raw particles particles_total'
[]
