import openmc
import numpy as np
import h5py

def main():
    # DEFINE OPENMC MODEL
    # ...
    TRISO_sphere_cell.fill = function_to_make_fill_list() # make a function to carefully construct this list!
    write_mat_list_to_h5(TRISO_cell.fill, "fuel_material_fill_ids.h5")

def function_to_make_fill_list():
  fill_list = []
  # implement zoning here
  return fill_list

def write_mat_list_to_h5(mat_list, filename):
    """
    Takes a list of openmc.Materials and writes the ids to an hdf5 file
    so that Cardinal can read it in and assign the fill before running
    a multiphysics simulation

    Parameters
    ----------
    mat_list : list of openmc.Material, the list of materials for the zoning
    filename : str, filename to write the information to
    """

    if not filename.endswith(".h5"):
        raise ValueError(f"The specified filename {filename} must end with .h5")

    ids = [mat.id for mat in mat_list]
    with h5py.File(filename, "w") as f:
        f.create_dataset("material_fill_ids", data=ids, dtype=np.int32)

if __name__ == "__main__":
    main()
