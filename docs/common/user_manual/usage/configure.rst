
.. _configure:

Configure
================================



.. _configure-1d:

.. figure:: ../../../images/configure-1d.png
	:align: center
	:figclass: align-center

	Configuration with a 1D shaking motion

In the configure tab, two paths need to be specified. 

You can either type them or click the '+' button to select them from your computer.

If you don't have OpenSees install, the instruction can be found :ref:`here <quickstart>`.
If you don't have a ground motion file, demos can be downloaded :download:`here <../../../_static/files/DemoGM.zip>`.

.. note:: 

   - Variables are assumed to have m, kPa, and kN units in |s3harkName|.    

The first demo is SRT-GM-Input-Style3.json, which contains the shaking motion in one direction (1D shaking). 
If you select this files as the input motion, your tab will look like the one shown in :numref:`configure-1d`. 
You can edit the slope degree :math:`\alpha`. For flat ground the value should be set as 0. 
If 1D shaking motion provided, |s3harkName| automatically treat the problem as a 2D plane strain problem. 
2D elements will be used. The slope diagram is plotted in :numref:`slope-2d`.

.. _slope-2d:

.. figure:: ../../../images/slope2d.png
	:align: center
	:figclass: align-center

	Slope definition for 2D Column

The second demo is SRT-GM-Input-Style3-2D.json, which contains the shaking motion in two directions (2D shaking). 
If you select this file as the input motion, your tab will look like the one shown in :numref:`configure-2d`.


.. _configure-2d:

.. figure:: ../../../images/configure-2d.png
	:align: center
	:figclass: align-center

	Configuration with a bi-directional shaking motion

You can see |s3harkName| detected the file you provided is a 2D shaking, 
|s3harkName| automatically treat the problem as a 3D problem. 
3D elements will be used. The slope diagram is plotted in :numref:`slope-3d`:


.. _slope-3d:

.. figure:: ../../../images/slope3d.png
	:align: center
	:figclass: align-center

	Slope definition for 3D Column

For flat ground :math:`\alpha` and :math:`\beta` should be set as 0. 
