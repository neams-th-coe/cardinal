import numpy as np
import h5py
from argparse import ArgumentParser

def write_ids(mat_ids, filename):
    """
    Takes a list of openmc.Materials and writes the ids to an hdf5 file
    so that Cardinal can read it in and assign the fill before running
    a multiphysics simulation

    Parameters
    ----------
    mat_list : list int, the list of materials for the zoning
    filename : str, filename to write the information to
    """

    if not filename.endswith(".h5"):
        raise ValueError(f"The specified filename {filename} must end with .h5")

    with h5py.File(filename, "w") as f:
        f.create_dataset("material_fill_ids", data=mat_ids, dtype=np.int32)

if __name__ == "__main__":
    ap = ArgumentParser()
    ap.add_argument(
        "-i",
        "--mat_ids",
        dest="mat_ids",
        type=int,
        nargs="+",
        default = [1,2,1],
        help="List of material IDs",
    )
    ap.add_argument(
        "-f",
        "--file_name",
        dest="file_name",
        type=str,
        default = "material_fill_ids.h5",
        help="The name of the file to save data in. Must be an .h5 file.",
    )
    args = ap.parse_args()
    write_ids(args.mat_ids, args.file_name)
