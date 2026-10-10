# Fluff

General purpose open-source C++ library with a focus on performance.

## Overview

Fluff is a library made by a single person with the purpose to integrate
common code across different projects. Furthermore, some of the code is
meant to be as optimized as possible.

Some of the code may be useful for other people and thus this repo
was created under that idea.

## Disclaimer

The code may have bugs, as well as not being particularly clean. The code
may also not be fully tested.
It is still in development and constant reiteration.

## Noteworthy functionality

- `BitArray` An extremely efficient fully customizeable bit set, providing:
    - Logical operators (>>, &, |, ...)
    - Numerical operators (++, +)
    - Unaligned object insertion/retrieval
    where most of the work went towards the last point, being possible to very efficiently
    insert or retrieve objects at any **bit** (not byte) offset, of any length.
- `ContiguousArray` A container that efficiently stores objects contiguously in memory,
    potentially saving memory for a small performance cost.

## Motivation

The motivation was to unite common code under a library I can use for my own projects.

Furthermore, a lot of the code in it either:
- is too simple, to the point everyone just writes their own version.
- was not found anywhere with the same capabilities.
- was not found anywhere with the same efficiency.
- was not found anywhere with clean code/interface.

those are all the possible reasons for why a certain piece of code in this library exists.
Note that the code does not always fulfill the requirement it tried to, in other words,
it may fail to be more efficient or cleaner. That's because this library is a work in progress.

Some other motivation is to clean up, or provide utility functions to other libraries, that may
be unwelcome in the library itself. Because of this, Fluff offers toggleable extensions to
other libraries.

## How to use

Just add it as a subdirectory in CMake and link the library as `fluff` or `fluff::fluff`.

There currently is no guide or documentation on how to use the library, although anyone
is free to learn by exploring.

note the CMake flag `FLUFF_EXTENSION_<lib_name>` where `<lib_name>` is the name a library
it extends. Note that this will also include said library in your project.

## Contributing

Fluff is being worked on without contributions in mind, and as such
the repository isn't prepared for it.

If you have any idea as how to make the code more efficient, algorithmically or
instruction-wise, or would like to discuss about Fluff, feel free to contact! <3

## Todo
- testing (doctest)
- documentation (doxygen)
- BitVector
- ContiguousVector

## Future Ideas
- logging (spdlog) (?)
- package manager (Conan/vcpkg) (?)

## AI Policy

Strict NO-AI policy for this project.

## Thanks :3

Thank you for checking out my library! <3 <3 <3
