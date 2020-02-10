# <i>s<sup>3</sup>hark</i>
<h2>A SimCenter Tool For Site Response Analysis</h2>

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.3463594.svg)](https://doi.org/10.5281/zenodo.3246641)

![image](resources/images/SRT.png)

## What is <i>s<sup>3</sup>hark</i>

<i>s<sup>3</sup>hark</i> is the acronym of site-specific seismic hazard analysis and research kit.
This tool focuses on simulating wave propagation along soil depth using finite element (FE) method.
The intended audience for <i>s<sup>3</sup>hark</i> is researchers and practitioners interested in 
performing site-specific analysis of soil in response to earthquakes, and educators interested in 
teaching site response analysis in their classes. The tool provides a friendly interface for users
to input and modify soil layers using tables, while the built soil profile and the FE mesh being
visualized simultaneously. Results including acceleration, velocity, displacement, pore pressure, spectral acceleration, etc., 
are visualized for the soil profile and for each node as well, from which the user can comprehend the 
wave propagation and liquefaction status along the soil depth.

Features of <i>s<sup>3</sup>hark</i> include:
* 2D and 3D elements for dynamic analysis of fluid saturated porous media
* Advanced linear / nonlinear soil material models
* Total stress / effective stress analysis
* Bi-directional motions
* Flat / slope free field analysis
* Finite rigidity of the bedrock


Available material models:


| Material models        | Usage           | Status  |
| ------------- |:-------------:| -----:|
| ElasticIsotropic     | 2D/3D | ✓ |
| PM4Sand      | 2D      |   ✓|
| PM4Silt | 2D      |    ✓ |
| PressureIndependMultiYield | 2D/3D      |    ✓ |
| PressureDependMultiYield | 2D/3D      |    ✓ |
| PressureDependMultiYield02 | 2D/3D      |    ✓ |
| ManzariDafalias | 2D/3D      |    ✓ |
| Borja-Amies | 3D      |    ✓ |




## Instructions for users

[Quick start](https://nheri-simcenter.github.io/s3hark-Documentation)

## Instructions for developers: how to build

#### 1. Make sure the paths of your blas and lapack are set correctly in s3hark.pri 

#### 2. Make sure you have fortran and c++ compilers installed.

#### 3. Build <i>s<sup>3</sup>hark</i> by clicking Run in Qt Creator

## How to Cite
You can cite this software as follows:

Charles Wang, Frank McKenna, Peter Mackenzie-Helnwein, Adam Zsarnoczay, Wael Elhaddad, Michael Gardner, & Pedro Arduino. (2019, October 11). NHERI-SimCenter/s3hark: Release v1.1.2 (Version 1.1.2). Zenodo. http://doi.org/10.5281/zenodo.3483211

## Acknowledgement
This material is based upon work supported by the <img src="https://raw.githubusercontent.com/NHERI-SimCenter/SURF/master/docs/images/nsf.png"  width="20px"  alt="NSF"/> `National Science Foundation` under Grant No. 1612843.


Long Chen and Andrew Makdisi at University of Washington, Gregor Vilhar at PLAXIS, BV contributed to the verification of PM4Sand model. 

## Contact
Charles Wang, NHERI SimCenter, UC Berkeley, c_w@berkeley.edu
