NekRS uses [OCCA](https://libocca.org/#/) to provide an API for device programming. Available
backends include CPU (i.e. [!ac](MPI) parallelism), CUDA, HIP, OpenCL, and OpenMP.
There are several different ways that you can set the backend; in order of *decreasing* priority,

- Pass via the command line, like

  ```
  cardinal-opt -i nek.i --nekrs-backend=CPU
  ```
- Set in the `[OCCA]` block of the NekRS `.par` file to control the backend for a specific model, like

  ```
  [OCCA]
    backend = CPU
  ```
- Set the `NEKRS_OCCA_MODE_DEFAULT` environment variable to one of `CPU`, `CUDA`, `HIP`, `OPENCL`, or
  `OPENMP` to control the backend for all models, like

  ```
  export NEKRS_OCCA_MODE_DEFAULT=CPU
  ```

!alert! note title=Compiling for GPU?
If you plan to use a GPU backend, you will also need to
set the correct threading API in the `Makefile` by setting
the values of the `OCCA_CUDA_ENABLED`, `OCCA_HIP_ENABLED`, or `OCCA_OPENCL_ENABLED` variables to 1,
respectively.
!alert-end!

