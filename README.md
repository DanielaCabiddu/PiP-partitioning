# PiP-partitioning

PiP-partitioning is a C++ tool for efficiently partitioning point clouds using polygonal boundaries.
It is especially suited for workflows where point clouds must be split into manageable subsets while preserving geometric accuracy and processing speed.

The core of PiP-partitioning is a point-in-polygon test based on the Jordan Curve Theorem. According to the theorem, a point lies inside a polygon if a semi-infinite ray cast from the point intersects the polygon boundary an odd number of times. We extend the classic W. Randolph Franklin algorithm to support complex polygonal geometries, including those with holes (e.g., internal courtyards), which are common in building footprints.

The procedure is as follows:

1. The orthogonal projection of the point onto the polygon plane is checked against the polygon’s axis-aligned bounding box (AABB) for an early exclusion test.
2. If the point passes the AABB check, a ray (typically cast horizontally to the right) is used to count intersections with polygon edges.
3. An odd count means the point lies inside the polygon; an even (or zero) count means it lies outside.

When multiple disjoint polygons are provided, the test is applied independently to each polygon, using parallel computation for scalability and performance.
The tool integrates robust geometry handling, efficient spatial indexing, and fast point-in-polygon (PiP) checks, enabling it to process millions of points while maintaining high accuracy and scalability.

Typical applications include:

- Preprocessing LiDAR data for modeling or analysis
- Generating region-specific datasets for distributed processing
- Extracting point cloud subsets matching thematic layers (e.g., city districts, land parcels)

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
