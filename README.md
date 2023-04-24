# Forth
a forth 

Written in C - C++ test project, Visual studio 2019 - x86 or x64

Forth core project has no std library dependencies. Upon initialising vm "put char" and "get char" IO function pointers are passed in and these are the only external dependencies the forth VM has, although it is possible to register C functions for it to call in a similar manner to lua.

Compiled code is a list of forth word header addresses, contains a disassembler to view the contents of the dictionary using the word "showWords" which disassembles the whole dictionary printing the names of words it finds and printing the info contained in the word headers it encounters.

The VM itself is encapsulated in a struct and is passed a pointer to and size of a pool of memory to use when it is initialised, and separate pointers and sizes for the data and return stacks to use. Multiple forth VMs can therefore easily be created, this approach also takes inspiration from lua and will hopefully make it versatile as a game scripting language and also good for use in multithreaded situations. Links can be set up between the dictionaries of different VMs to provide namespace like functionality and avoid having to repeat the same compiled code enabling small VMs for use in game scripting scenarios.

I think it implements most of forth including:
  - words
  - user definable control flow words implemented as immediate forth words (pre-written library of several common ones)
  - pre-written forth library of several common forth words such as print, constant, variable, ",", ect (to be expanded)
  - string literals
  - defining words with user defined runtime behavior using create and does>
  - C interop
  - code comments

three projects:
  Forth     - the core forth. VM struct definition, Functions to initialise VM and do string of forth source code, register C function as callable from forth
  ForthRepl - uses Forth to implement a forth repl
  ForthTest - C++ google test test project 

TODO 
(short term):
- add ability to pass text files to the repl exe. It should preprocess the files to ensure one space between each token and no non space whitespace (but be smart enough not to remove whitespace from inside string literals (DONE)
- write more useful words, in forth where possible
- get tests to run on new (old) pc and update tests to reflect new word additions
- Create C interop helper library, including linking between vms for a namespace like ability

(long term):
- write custom repl terminal / ide with dictionary inspector. Try to use non windows specific gui library
- add breakpoints somehow
- add float stack
- systematically go through forth standard implementing all the words
- create build pipeline with running tests. Improve repo, remove visual studio dependency convert project to CMAKE or similar

Future port targets (boards I own):
- raspberry pi pico
- stm32 f303k8
- atmega 32
