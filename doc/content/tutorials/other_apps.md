# Coupling to Other MOOSE Apps

In this tutorial, you will learn how to:

- Couple Cardinal to any arbitrary MOOSE application

To access this tutorial,

```
cd cardinal/tutorials/other_apps
```

For convenience purposes, Cardinal contains several other MOOSE applications
as submodules - Sockeye and SAM, allowing you to simply build those
applications in tandem with Cardinal and even run Sockeye/SAM input files
with Cardinal's executable. But in the general case, you might want to couple
Cardinal to an *arbitrary* MOOSE application. This tutorial will show how to
use dynamic linking in order to couple Cardinal to a generic MOOSE application
named Chickadee - please first clone this repository with

```
git clone https://github.com/aprilnovak/Chickadee.git
```

Chickadee doesn't solve any interesting physics - it only contains one
MOOSE object - a kernel named `SoluteDiffusion`, which is just a diffusion-type
kernel with a user-specified coefficient.

It's a good idea to then set the `MOOSE_DIR` environment variable to the
MOOSE version that you are using in Cardinal; unless you specified something
special when building Cardinal, this will be `cardinal/contrib/moose`:

```
export MOOSE_DIR=$HOME/cardinal/contrib/moose
```

Adjust the above according to wherever you have the Cardinal repository cloned.
Then, you must compile Chickadee just as you would for any other MOOSE application:

```
cd chickadee
make -j8
```

## Running Cardinal as the Main App

You can couple Cardinal to an arbitrary MOOSE application and run Cardinal either
as the main application or as a sub-application. There is a very small difference
between the two, so we will describe both. To run Cardinal as the main application,
we need to create an input file that runs Cardinal. In this file, we include a
[TransientMultiApp](https://mooseframework.inl.gov/source/multiapps/TransientMultiApp.html),
which will run Chickadee at the end of each Cardinal time step; in this particular
example, we run a Nek-wrapped input file for Cardinal.

!listing /tutorials/other_apps/cardinal_master.i

We have intentionally made the Cardinal input file very small, since we're
not trying to teach you anything about the Nek wrapping at the moment. Instead,
the only thing we need to run Chickadee as a sub-application is to provide a
`library_path` that points to the `lib` directory wherever you have the Chickadee
repository. Then, simply specify the `app_type` as a `ChickadeeApp`. We leave
the `[Transfers]` block empty - you will need to fill it out according
to your application to reflect the data to be communicated between Chickadee
and Cardinal.

If we take a quick look at the Chickadee input file, we see a very basic input file
that uses the `SoluteDiffusion` kernel in Chickadee.

!listing /tutorials/other_apps/chickadee_sub.i

To run this input file, use the Cardinal executable (since the main application
is a Cardinal application):

```
cardinal-opt -i cardinal_master.i
```

## Running Cardinal as the Sub App

To instead run Cardinal as the sub-application, we need to put a `CardinalApp`
multi-app within a Chickadee input file. Note that we have changed the
`library_path` to now point to the `lib` directory in Cardinal.

!listing /tutorials/other_apps/chickadee_master.i

The Nek-wrapped sub-application is then just a very small Cardinal input
file.

!listing /tutorials/other_apps/cardinal_sub.i

To run this input, you must run with the Chickadee executable that you built previously -
otherwise, `CardinalApp` will try to find a `SoluteDiffusion` object, which
only exists in `ChickadeeApp`, not `CardinalApp`.

```
chickadee-opt -i chickadee_master.i
```

