



Theory and Implementation
==========================


Site-Response Analysis in Geotechnical Earthquake Engineering
--------------------------------------------------------------


Available Material Models
--------------------------------------------------------------

.. list-table:: Available material models
   :widths: 25 25 50
   :header-rows: 1

   * - Material models
     - Usage
     - Development Status
   * - ElasticIsotropic
     - 2D/3D
     - ✓
   * - PM4Sand
     - 2D
     - ✓
   * - PM4Silt
     - 2D
     - ✓
   * - PressureIndependMultiYield
     - 2D/3D
     - ✓
   * - PressureDependMultiYield
     - 2D/3D
     - ✓
   * - PressureDependMultiYield02
     - 2D/3D
     - ✓
   * - ManzariDafalias
     - 2D/3D
     - ✓
   * - Borja-Amies
     - 3D
     - ✓

## ElasticIsotropic

Choosing `Elastic` as the material model will bring up the editing tab for this model.


## PM4Sand

Choosing `PM4Sand` as the material model will bring up the editing tab for this model.

The sand plasticity model PM4Sand (version 3) [[3]](#references) follows the basic framework of the stress-ratio controlled, critical state compatible, bounding surface plasticity model for sand presented by [[4]](#references). 
Modifications to the model were developed by [[3]](#references) to improve its ability
to approximate the stress-strain responses important to geotechnical earthquake engineering applications; 
in essence, the model was calibrated at the equation level to provide for better approximation of the trends observed across a set of experimentally- and case history-based design correlations. 
The model is shown to provide reasonable approximations of desired behaviors and to be relatively easy to calibrate.

## PM4Silt
Choosing `PM4Silt` as the material model will bring up the editing tab for this model.

PM4Silt [[2]](#references) is a plasticity model for representing low-plasticity silts and clays in geotech- nical earthquake engineering applications . The PM4Silt model builds on the framework of the stress-ratio controlled, critical state compatible, bounding surface plasticity PM4Sand model. Modifications to the model were developed and implemented to improve its abil- ity to approximate undrained monotonic and cyclic loading responses of low-plasticity silts and clays, as opposed to those for purely nonplastic silts or sands. Emphasis was given to obtaining reasonable approximations of undrained monotonic shear strengths, undrained cyclic shear strengths, and shear modulus reduction and hysteretic damping responses across a range of initial static shear stress and overburden stress conditions. The model does not include a cap, and therefore is not suited for simulating consolidation settlements or strength evolution with consolidation stress history. The model is cast in terms of the state parameter relative to a linear critical state line in void ratio versus logarithm of mean effective stress. The primary input parameters are the undrained shear strength ratio (or undrained shear strength), the shear modulus coefficient, the contraction rate parameter, and an optional post-strong-shaking shear strength reduction factor.

## PressureIndependMultiYield
Choosing `PIMY` as the material model will bring up the editing tab for this model.
PressureIndependMultiYield material is an elastic-plastic material in which plasticity exhibits only in the deviatoric stress-strain response. The volumetric stress-strain response is linear-elastic and is independent of the deviatoric response. This material is implemented to simulate monotonic or cyclic response of materials whose shear behavior is insensitive to the confinement change. Such materials include, for example, organic soils or clay under fast (undrained) loading conditions.

## PressureDependMultiYield and PressureDependMultiYield02
Choosing `PDMY` or `PDMY02` as the material model will bring up the editing tab for these models.

PressureDependMultiYield and PressureDependMultiYield02 materials are elastic-plastic material for simulating the essential response characteristics of pressure sensitive soil mate- rials under general loading conditions. Such characteristics include dilatancy (shear-induced volume contraction or dilation) and non-flow liquefaction (cyclic mobility), typically exhibited in sands or silts during monotonic or cyclic loading.



 ## Manzari-Dafalias
 Choosing `Manzari-Dafalias` as the material model will bring up the editing tab for this model.

Manzari Dafalias [[4]](#references) is a stress-ratio controlled, critical state compatible, sand plasticity model. A fabric-dilatancy related quantity, scalar valued in the triaxial and tensor valued in generalized stress space, which is instrumental in modeling macroscopically the effect of fabric changes during the dilatant phase of deformation on the subsequent contractant response upon load increment reversals, and the ensuing realistic simulation of the sand behavior under undrained cyclic loading. The dependence of the plastic strain rate direction on a modified Lode angle in the multiaxial generalization enables it to produce realistic stress-strain simulations in nontriaxial conditions. A very systematic connection between the simple triaxial and the general multiaxial formulation makes it possible to use correctly the model parameters of the former in the implementation of the latter.




## Borja-Amies
Choosing `J2Bounding` as the material model will bring up the editing tab for this model.

Borja-Amies [[1]](#references) is a total stress-based bounding surface plasticity model for clays developed to accommodate multiaxial stress reversals. The model is constructed based on the idea of a vanishing elastic region undergoing pure translation inside a bound- ing surface, and an interpolation function for hardening modulus which varies with stress distance of the elastic region from the unloading point. Central to the development of the model are the general criteria for loading and unloading, which are phrased based upon the simple argument that with continued loading the hardening modulus should decrease monotonically with deformation. Combined with numerical integration of the elastoplastic constitutive equations in a form suitable for a robust computer implementation, the model is applied to cohesive soils undergoing undrained stress reversals and cyclic loading. With a suitable choice of the interpolation function for the hardening modulus, it is shown that existing one-dimensional nonlinear laws for soils can be replicated, such as the hyperbolic, exponential, the Davidenkov, and even the Ramberg-Osgood models. Specifically, the ap- propriateness of the exponential hardening function for cohesive soils is investigated and its parameters determined for some clays and silts for use in dynamic soil-structure interaction modeling.


## References
---

[1] Ronaldo I Borja and Alexander P Amies. “Multiaxial cyclic plasticity model for clays”. In: Journal of geotechnical engineering 120.6 (1994), pp. 1051–1070.

[2] Ross W Boulanger and Katerina Ziotopoulou. “PM4Silt (Version 1): a silt plastic- ity model for earthquake engineering applications”. In: Report No. UCD/CGM-18/01, Center for Geotechnical Modeling, Department of Civil and Environmental Engineer- ing, University of California, Davis, CA, 108 pp. (2018).

[3] RW Boulanger and K Ziotopoulou. PM4Sand (Version 3): A sand plasticity model for earthquake engineering applications. Center for Geotechnical Modeling Report No. UCD/CGM-15/01, Department of Civil and Environmental Engineering, University of California, Davis, Calif, 2015.

[4] Yannis F Dafalias and Majid T Manzari. “Simple plasticity sand model accounting for fabric change effects”. In: Journal of Engineering mechanics 130.6 (2004), pp. 622–634.



Verification and Validation 
=============================



2D Column 1D Shaking
----------------------


3D Column Bidirectional Shaking
---------------------------------