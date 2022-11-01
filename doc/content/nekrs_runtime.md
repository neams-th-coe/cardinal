# Troubleshooting NekRS at Runtime

This page collects some common issues encountered when running NekRS
models. Generally, if you are encountering issues it is a good idea to
*clear the cache* directory so that you're absolutely sure there aren't
old case build files lingering around that are causing conflicts. Try
the following first:

```
rm -rf .cache/
```

and then re-run your case. If the issue persists, check out these other
potential causes.

## Insufficient Memory

If you encounter an error like the following

```
obj/libnek5000.a(navier1.o): In function `expl_strs_e_':
navier1.f:(.text+0x11a3): relocation truncated to fit: R_X86_64_PC32 against `.bss'
```

this means that you have likely run out of memory. Please increase the number of nodes.
In some cases, you might need to delete the `.cache` directory for the new
pre-compilation to occur correctly.

## Threading

NekRS does not support threading. On some systems, you might see an error like

```
libgomp: Thread creation failed: Resource temporarily unavailable
```

This error can usually be addressed by explicitly telling NekRS to use just 1 OpenMC
thread, with `export OMP_NUM_THREADS=1`.

## parRSB

If you see an error just after NekRS prints `running parRSB ...`, such as this:

```
running parRSB ...
cardinal-opt: ../../ccmi/protocols/barrier/MultiLeaderBarrierT.h:256: static void CCMI::Protocols::Barrier::MultiLeaderBarrierFactoryT<T_Composite, T_Conn>::cb_async(libcoll_context_t, void*, const void*, unsigned int, unsigned int, size_t, LibColl::PipeWorkQueue**, void (**)(libcoll_context_t, void*, libcoll_result_t), void**) [with T_Composite = CCMI::Protocols::Barrier::MultiLeaderBarrierT<LibColl::Interfaces::NativeInterface, (LibColl::topologyIndex_t)0>; T_Conn = CCMI::ConnectionManager::SimpleConnMgr; libcoll_context_t = void*; size_t = long unsigned int; libcoll_event_function = void (*)(void*, void*, libcoll_result_t)]: Assertion `composite != __null' failed.
```

there may be issues with parRSB. You can get around these by using `gencon` and `genmap`
(tools that [ship with Nek5000](nek_tools.md)).
