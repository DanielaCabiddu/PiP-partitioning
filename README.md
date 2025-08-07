# PBF-FR-partitioning

The identification and recognition of urban features are essential for creating accurate and comprehensive digital representations of cities. In particular, the automatic characterization of fac ̧ade elements plays a key role in enabling semantic enrichment and 3D reconstruction. It also supports urban analysis and underpins various applications, including planning, simulation, and visualization. 

This repository icludes the partitioning approach presented and described in the paper titles *PBF-FR: Partitioning Beyond Footprints for Façade Recognition in Urban Point Clouds*. to appear in Computers \& Graphics - Special Issue 3DOR 2025. 

![Representative Image](images/representative-image.png)

## Clone
We provide the commands to install EWoPe. 
The repository includes a submodule necessary to make the code work. Please, clone it recursively:

- Clone recursively the repository into your local machine:
```
git clone --recursive https://github.com/DanielaCabiddu/PBF-FR-partitioning.git
```

## Content of the repository
- `src`: source code 
- `external`: external libraries
- `data`: data necessary to replicate experiments described in the paper
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

## Author & Copyright
Daniela Cabiddu

## Citing us
```bibtex
@article{pbf-fr,
  author       = {Chiara Romanengo, Daniela Cabiddu, Michela Mortara},
  title        = {{PBF-FR: Partitioning Beyond Footprints for Façade Recognition in Urban Point Clouds}},
  year         = {2025 (to appear)},
  journal      = {Computers \& Graphics - Special Issue 3DOR 2025}
}
```

## Acknowledgment

