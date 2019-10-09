<img src="https://raw.githubusercontent.com/NHERI-SimCenter/s3hark/master/docs/images/configure-1d.png" alt="configure"/>

In the configure tab, two paths need to be specified. 

You can either type them or click the '+' button to select them from your computer.

If you don't have OpenSees install, [here](start) is the instruction.
If you don't have a ground motion file, [here](https://nheri-simcenter.github.io/s3hark/DemoGM.zip) are demos to download.

The first demo is SRT-GM-Input-Style3.json, which contains the shaking motion in one direction (1D shaking). 
If you select this files as the input motion, your tab will look like the one shown above. 
You can edit the slope degree α. For flat ground the value should be set as 0. 
If 1D shaking motion provided, <i>s<sup>3</sup>hark</i> automatically treat the problem as a 2D plane strain problem. 
2D elements will be used. The slope diagram:
<img src="https://raw.githubusercontent.com/NHERI-SimCenter/s3hark/master/docs/images/slope2d.png" alt="slope"/>


The second demo is SRT-GM-Input-Style3-2D.json, which contains the shaking motion in two directions (2D shaking). 
If you select this file as the input motion, your tab will look like the one shown below.

<img src="https://raw.githubusercontent.com/NHERI-SimCenter/s3hark/master/docs/images/configure-2d.png" alt="configure"/>

You can see <i>s<sup>3</sup>hark</i> detected it's 2D shaking, 
<i>s<sup>3</sup>hark</i> automatically treat the problem as a 3D problem. 
3D elements will be used. The slope diagram:

<img src="https://raw.githubusercontent.com/NHERI-SimCenter/s3hark/master/docs/images/slope3d.png" alt="slope"/>

For flat ground α and β should be set as 0. 



