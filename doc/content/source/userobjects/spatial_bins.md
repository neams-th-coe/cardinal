The bins are specified by providing a list of binning user objects
with the `bin` parameter.
Available user objects for specifying spatial bins are:

- [HexagonalSubchannelBin](HexagonalSubchannelBin.md)
- [HexagonalSubchannelGapBin](HexagonalSubchannelGapBin.md)
- [LayeredBin](LayeredBin.md)
- [LayeredGapBin](LayeredGapBin.md)
- [RadialBin](RadialBin.md)

If more than one bin is provided, then the bins are taken as the
product of each individual bin distribution.
