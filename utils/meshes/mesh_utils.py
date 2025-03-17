# Meshing utilities

def build_pattern(n_rings: int) -> str:
  """Build 'pattern' input parameter for PatternedHexMeshGenerator, assuming single input assembly type"""
  pattern = []
  # Build top half of pattern vector, including middle row
  for i in range(n_rings):
    pattern.append(' ' .join(['0'] * (n_rings + i)))

  # Build bottom half of pattern vector by reversing top half, excluding middle row
  pattern += list(reversed(pattern[:-1]))

  return "'" + '; '.join(pattern) + "'"
