# Forth
a forth 

    
16/09/23 
  - premake build system set up, to enable generation of project files for many different IDEs
  - google test added as a git sub module, test project builds googletest from source
  - to build on windows, run one of the build project files batch files

Written in C - C++ test project, Visual studio 2019 - x86 or x64

Forth core project has no std library dependencies and no dynamic memory allocation. Upon initialising vm "put char" and "get char" IO function pointers are passed in and these are the only external dependencies the forth VM has, although it is possible to register C functions for it to call in a similar manner to lua.

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
  - Cooperative multitasking
      - inter-task communication through Mail boxes, semaphore 

three projects:
  Forth     - the core forth. VM struct definition, Functions to initialise VM and do string of forth source code, register C function as callable from forth
  ForthRepl - uses Forth to implement a forth repl
  ForthTest - C++ google test test project 

TODO 
(short term):
- add ability to pass text files to the repl exe. It should preprocess the files to ensure one space between each token and no non space whitespace (but be smart enough not to remove whitespace from inside string literals (DONE)
- Create C interop helper library, including linking between vms for a namespace like ability - (Some of this has been done in MrDo clone project - changes need porting over to this one. Changes also include an explicitly 32 bit fetch and store word)
- an easy one that's been overlooked and is crucial in any real forth programming scenario - implement "forget" - reset the dictionary top pointer to directly before a given word
- when the return stack is printed with "show", it should maybe print the name of the function (word) that the return addresses are within. It would still need to show a numeric value too to be compatible with using the return stack to implement local variables (or in fact using the return stack just for loop counters like I do now).

(medium term):
- implement REPL in forth - will probably require some new words - getchar is done but not really tested
- create Raspberry pi pico build - basic shell that runs through serial and can create words in RAM
- change build system from premake to CMake - it's just better I think and more amenable to embedded platforms - needed, I think for pico build


(long term):
- write custom repl terminal / ide with dictionary inspector. Try to use non windows specific gui library
- add breakpoints somehow
- add float stack
- systematically go through forth standard implementing all the words
- create build pipeline with running tests. Improve repo, remove visual studio dependency convert project to CMAKE or similar
- implement local variables https://forth-standard.org/standard/locals
- words to save dictionary items to flash memory in pico build, and select a forth word to run on bootup
- assembler words in pico build - write assembly code RPN style in forth, with labels, (from a repl for example) and have it assemble on the fly. If it has reached this point and has saving to flash memory, it'd actually be good and other people might want to use it

(unknown-term)
  - make tests run through visual studio test runner
  - consider the possibility that there should be some checking for stack over and underflows, whether optional or not
  - make the inner interpreter faster - but it must remain portable C and preferably not have goto's!!! Also, maybe make it more portable to ancient versions of C.

Future port targets (boards I own):
- raspberry pi pico
- stm32 f303k8
- atmega 32
