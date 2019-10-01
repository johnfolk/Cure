# Cure
Cure is a set of tools for robot programmers 
## Dependencies

Cure has been tested on older linux but should be still robust as it does not use many libs.

The git repository is not complete yet but we will be transfering it over bit by bit.  

```
sudo apt-get install libcereal-dev libglfw3-dev libceres-dev libtinyxml2-dev
```
On 18.04 the dependencies should be similar. Note that you may need to provide
the flags `-DAUVLIB_USE_LIBIGL_TINYXML=ON -DAUVLIB_USE_LIBIGL_GLFW=ON` to cmake.
In that case, ignore the errors about missing install targets.

## Building



When done, create a `build` folder in the repo root, and run
```
cd build
cmake -DCMAKE_INSTALL_PREFIX=../install ..
make -j4
make install
```

You should now have a compiled version of auvlib in the folder
`/path/to/Cure/install`. 
```

## Using as a python library

Ha Ha, Not yet

## Using as a c++ library

First, initialize the submodules, same as for the previous section. For using auvlib as a library in an external project, Being transfered to git soon:)


For very complete documentation on C++ library usage, see [the overview document](nada.kth.se/cas/CURE/doc-cure-2.2.2/html/annotated.html).

## Contributors

* John Folkesson johnf@kth.se
* Patric Jensfelt patric@kth.se


