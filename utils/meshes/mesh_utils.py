# Meshing utilities

def build_pattern(n_rings: int,
                  first_row: int,
                  last_row: int) -> str:
  pattern = []
  for i in range(n_rings):
    pattern.append(' ' .join(['0'] * (first_row + i)))
  for i in range(n_rings - 1):
    pattern.append(' ' .join(['0'] * (last_row - 1 - i)))
  return "'" + '; '.join(pattern) + "'"
