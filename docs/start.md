
## Download and Install <i>s<sup>3</sup>hark</i>

The app is [here](https://www.designsafe-ci.org/data/browser/public/designsafe.storage.community/SimCenter/Software/s3hark) if you haven't got it.

## Install OpenSees

The current version of <i>s<sup>3</sup>hark</i> utilizes OpenSees as its backend. Its internal backend is being developed and will be available in next release. 

If you don't have OpenSees installed, go to the [download page](https://opensees.berkeley.edu/OpenSees/user/download.php) to register.

After registration, you can proceed to the download page by entering your email address and clicking the Submit button. 

Read through that page, then download and install the correct versions of OpenSees and Tcl depending on your system (Windows or Mac).

Remember the installation path of OpenSees. 

## Download input motion files

Click [this](https://nheri-simcenter.github.io/s3hark/DemoGM.zip) link to download demos for input rock motion files. 

Unzip the downloaded file you'll see two json demos: one is for 1D shaking, the other is for bi-directional shaking.


## Add soil layers

Click the '+' button on the top of the table to add a soil layer.


## Run the analysis

Make sure the following two paths have been set correctly in the Configure tab:
* OpenSees executable path 
* Ground motion file path 

Click the 'Analyze' button to start the simulation.