ecto doc
================================
ecto is...

Contents:

.. toctree::
   :maxdepth: 2

Building
==================
Using a standard cmake build system:

.. code-block:: sh
  
  mkdir build
  cd build
  cmake ..
  make
  

The libs will be located in source under lib

Assuming you're in the top level ecto directory, try the following in a shell:

.. code-block:: sh

  export PYTHONPATH=`pwd`/lib:`pwd`/python
  python test/python/doit.py
  python test/python/test_plasm.py
  
Structure
==================
  - include
      c++ include files
  - src
      c++ library
  - src/ecto_py
      ecto boost::python wrappers
  - python
      ecto python library
  - test/modules
      ecto c++ modules
  - test/python
      python tests of ecto
  
libraries:
  - libecto.so
      c++ library, clients must link against this and the 
  - _ecto.so
      python bindings lib
  - buster.so
      a sample client python exposed module

Usage
=================
to make a module use the macro in cmake:

.. code-block:: cmake

  #this takes care of linking against python and ecto
  ectomodule(buster
    test/modules/buster.cpp
  )
  ecto_link(buster
    ${MY_EXTRA_LIBS}
  )
