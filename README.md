# Cure
Cure is a set of tools for robot programmers 
## Dependencies

Cure has been tested on older linux but should be still robust as it does not use many libs.

Cure was designed as a central part called the toolbox which holds the core functionallity of classes for working with data, file reading and writing, multithreading, inter and inner process communication, dynamic system reconfiguration,...

After installing that one could install optional modules for various specific functionality.  All that managed as one giant CVS repository that one could pick and choose what parts to check out.  Then a complex libtools system would magically figure out what you did and compile the whole thing correctly.

Now we are using git and Cmake so the modules are going to be separate repositories that you can clone in the root Cure folder then tweek the CMakefile.txt in the root to add the new module directory.  Not as powerful but simple and it works.
`
`
## Building

Just clone this repo and when done, create a `build` folder in the Cure root folder, and run
```
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ..
make -j4
make install
```

You should now have a compiled version of cure in the folder
`.../Cure/install`. 
```
For very complete documentation on C++ library usage, see (nada.kth.se/cas/CURE/doc-cure-2.2.2/html/annotated.html).

## Contributors

* John Folkesson johnf@kth.se
* Patric Jensfelt patric@kth.se


