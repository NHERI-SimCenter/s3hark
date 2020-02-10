
.. _lbl-how-to-build:

How to Build
=============


To build the tool, follow the below instructions:

#. Install `Qt <https://www.qt.io/>`_. Qt is free for open source developers. 

#. Download and install the BLAS library following instructions `here <http://www.netlib.org/blas/>`_.

#. Download and install the LAPACK library following instructions `here <http://www.netlib.org/lapack/>`_.

#. Download this repository from `Github <https://github.com/NHERI-SimCenter/s3hark>`_.

#. Open the file s3hark.pri using any text editor, and make sure the path to your BLAS and LAPACK files are set correctly if they are not located at standard folders.

#. Open the file s3hark.pro using Qt Creator, build |s3harkName| by clicking the Run button.
