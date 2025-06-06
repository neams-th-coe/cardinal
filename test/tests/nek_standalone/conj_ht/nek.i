[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'conj_ht'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
 [Area_BC3_flow]
   type = NekSideIntegral
   boundary = '3'
   field = unity
   mesh = 'fluid'
 []
 [Area_BC3_all]
   type = NekSideIntegral
   boundary = '3'
   field = unity
   mesh = 'all'
 []
 [SideAverage_T_BC3_flow]
   type = NekSideAverage
   boundary = '3'
   field = temperature
   mesh = 'fluid'
 []
 [SideAverage_T_BC3_all]
   type = NekSideAverage
   boundary = '3'
   field = temperature
   mesh = 'all'
 []
 [HeatFluxIntegral_BC3_flow]
   type = NekHeatFluxIntegral
   boundary = '3'
   mesh = 'fluid'
 []
 [HeatFluxIntegral_BC3_all]
   type = NekHeatFluxIntegral
   boundary = '3'
   mesh = 'all'
 []
 [MassFlowRate_BC1_flow]
   type = NekMassFluxWeightedSideIntegral
   boundary = '1'
   field = unity
   mesh = 'fluid'
 []
 [MassFlowRate_BC1_all]
   type = NekMassFluxWeightedSideIntegral
   boundary = '1'
   field = unity
   mesh = 'all'
 []
 [MflowAvgTemp_BC2_flow]
   type = NekMassFluxWeightedSideAverage
   boundary = '2'
   field = temperature
   mesh = 'fluid'
 []
 [MflowAvgTemp_BC2_all]
   type = NekMassFluxWeightedSideAverage
   boundary = '2'
   field = temperature
   mesh = 'all'
 []
 [Reynolds_BC1_flow]
   type = ReynoldsNumber
   boundary = '1'
   L_ref = 0.5
   mesh = 'fluid'
 []
 [Reynolds_BC1_all]
   type = ReynoldsNumber
   boundary = '1'
   L_ref = 0.5
   mesh = 'all'
 []
 [Peclet_BC1_flow]
   type = PecletNumber
   boundary = '1'
   L_ref = 0.5
   mesh = 'fluid'
 []
 [Peclet_BC1_all]
   type = PecletNumber
   boundary = '1'
   L_ref = 0.5
   mesh = 'all'
 []
#
# Volume post processors
#
 [Vol_flow]
   type = NekVolumeIntegral
   field = unity
   mesh = 'fluid'
 []
 [Vol_all]
   type = NekVolumeIntegral
   field = unity
   mesh = 'all'
 []
 [maxVol_T_flow]
   type = NekVolumeExtremeValue
   field = temperature
   value_type = max
   mesh = 'fluid'
 []
 [maxVol_T_all]
   type = NekVolumeExtremeValue
   field = temperature
   value_type = max
   mesh = 'all'
 []
 [avgVol_T_flow]
   type = NekVolumeAverage
   field = temperature
   mesh = 'fluid'
 []
 [avgVol_T_all]
   type = NekVolumeAverage
   field = temperature
   mesh = 'all'
 []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = final
[]
