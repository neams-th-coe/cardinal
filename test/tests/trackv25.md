# Cases Tested with v25

| Case Directory                            | Test Name                                                                                    | Pass / Fail                       | Comment                     |
|-------------------------------------------|----------------------------------------------------------------------------------------------|-----------------------------------|-----------------------------|
| nek_mesh/exact                            | boundary_exact<br>volume_exact<br>second_exact                                               | ✅ Pass<br>✅ Pass<br>✅ Pass   |                             |
| nek_mesh/first_order                      | first_order_mesh<br>first_order_volume_mesh                                                  | ✅ Pass<br>✅ Pass               |                             |                  
| nek_mesh/second_order                     | second_order_mesh<br>second_order_volume_mesh                                                | ✅ Pass<br>✅ Pass               |                             |                  
| nek_mesh/sidesets/pyramid                 | pyramid_sidesets<br>pyramid_sidesets_exact                                                   | ✅ Pass<br>✅ Pass               |                             |
| nek_mesh/sidesets/cube                    | cube_sidesets<br>cube_sidesets_exact                                                         | ✅ Pass<br>✅ Pass               |                             |
| transfers/nek_temperature                 | temperature_input<br>multiple_field_transfers<br>too_many_slots                              | ✅ Pass<br>✅ Pass<br>✅ Pass   |                             |
| transfers/nek_temperature/volume          | temperature_input                                                                            | ✅ Pass                          |                             |
| transfers/nek_source                      | multiple_source_transfers<br>multiple_flux_transfers                                         | ✅ Pass<br>✅ Pass              |                             |
| transfers/nek_scalar_value                | controls                                                                                     | ✅ Pass                          |                             |
| transfers/nek_flux                        | volume<br>flux                                                                               | ✅ Pass<br>✅ Pass              |                             |
| transfers/nek_postprocessor_value         | nek                                                                                          | ✅ Pass                          |                             |
| transfers/nearest_point                   | nearest_point_receiver                                                                       | ✅ Pass                          |                             |
| conduction/identical_interface/cube       | slab_conduction                                                                              | ✅ Pass                          |                             |
| conduction/identical_interface/pyramid    | pyramid_conduction                                                                           | ✅ Pass                          | run using `--heavy`         |
| conduction/boundary_and_volume/prism      | pyramid_exact<br>duplicate_temp<br>pyramid                                                   | ❌ Fail<br>✅ Pass<br>❌ Fail   | `gold` update required<br>PR #10 |
| conduction/identical_volume/cube          | slab_heat_source                                                                             | ✅ Pass                          |                             |
| conduction/zero_flux                      | zero_flux_total<br>zero_flux_total_vpp<br>vpp_disjoint<br> vpp_disjoint_zero<br>mismatch_length<br>nodes_on_shared  | ✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass |                     |
| conduction/nonidentical_interface/cylinders | cylinder_conduction<br>cylinder_conduction_subcycle<br>cylinder_conduction_reversed<br>cylinder_conduction_mini<br>cylinder_conduction_exact  | ✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass | |
| conduction/nonidentical_volume/nondimensional | cylinder_heat_source                                                                     | ✅ Pass             | |
| conduction/nonidentical_volume/cylinder   | cylinder_heat_source<br>cylinder_exact                                                       | ✅ Pass<br>✅ Pass             |     |
| conduction/reverse_cht                    | reverse_cht                                                                                  | ✅ Pass             |     |
| nek_errors/usrwrk_transfers               | duplicate_scratch<br>exceed_allocated_field<br>exceed_allocated_scalar<br>field_duplicated_by_field<br>scalar_duplicated_by_field<br>flux_no_boundary<br>source_no_volume<br>flux_slots<br>source_slots<br>problem_field<br>problem_scalar   |  ✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass<br>✅ Pass            |     |
