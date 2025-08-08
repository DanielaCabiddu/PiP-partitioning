# PiP-partitioning

## Clone
The repository includes the submodules necessary to make the code work. Please, clone it recursively:

- Clone recursively the repository into your local machine:
```
git clone --recursive https://github.com/DanielaCabiddu/PiP-partitioning.git
```

In the following, assume the **${ROOT}** folder to be the one whee this *README* lies.

## Content of the repository
- `src`: source code 
- `external`: external libraries
- `scripts`: scripts necessary to replicate experiments described in the paper
- `CMakeLists.txt`: build configuration file


## Build the source code
To build the source code, use the following pipeline:

```
cd ${ROOT}
mkdir build
cd build
cmake --DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

Binaries will be available in the **${ROOT}/bin** folder

## Author & Copyright
Daniela Cabiddu (CNR-IMATI). Contact Email: daniela.cabiddu@cnr.it
