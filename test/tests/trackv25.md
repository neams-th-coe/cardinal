# Cases Tested with v25

| Case Directory                            | Test Name                                                                                    | Pass / Fail                       |
|-------------------------------------------|----------------------------------------------------------------------------------------------|-----------------------------------|
| nek_mesh/exact                            | boundary_exact<br>volume_exact<br>second_exact                                               | ✅ Pass<br>✅ Pass<br>✅ Pass   |
| nek_mesh/first_order                      | first_order_mesh<br>first_order_volume_mesh                                                  | ✅ Pass<br>✅ Pass               |
| nek_mesh/second_order                     | second_order_mesh<br>second_order_volume_mesh                                                | ✅ Pass<br>✅ Pass               |
| nek_mesh/sidesets/pyramid                 | pyramid_sidesets<br>pyramid_sidesets_exact                                                   | ✅ Pass<br>✅ Pass               |
| nek_mesh/sidesets/cube                    | cube_sidesets<br>cube_sidesets_exact                                                         | ✅ Pass<br>✅ Pass               |
| transfers/nek_temperature                 | temperature_input<br>multiple_field_transfers<br>too_many_slots                              | ✅ Pass<br>✅ Pass<br>✅ Pass    |
| transfers/nek_temperature/volume          | temperature_input                                                                            | ✅ Pass<br>                      |
| transfers/nek_source                      | multiple_source_transfers<br>multiple_flux_transfers                                         | ✅ Pass<br>✅ Pass<br>           |
| transfers/nek_scalar_value                | controls                                                                                     | ✅ Pass<br>                      |
| transfers/nek_flux                        | volume<br>flux                                                                               | ⏳ Pending Conversion             |
| transfers/nek_postprocessor_value         | nek                                                                                          | ✅ Pass<br>                      |
| transfers/nearest_point                   | nearest_point_receiver                                                                       | ✅ Pass<br>                      |
