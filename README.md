# SiteResponseTool
<h2>A SimCenter Tool For Site Response Analysis</h2>

![image](resources/images/SRT.png)


### How to build?
#### 1. Install JSON for Modern C++, a header-only library
##### You jsut need to download it from its <a href="https://github.com/nlohmann/json" target="_blank">repo</a>, and copy nlohmann/json to a place where your include system can find. 

#### 2. Install lapack 
##### Make sure the path of your lapack is set correctly in Makefile.in and SiteResponse.pro

#### 3. Install fortran and c++ compilers, if you haven't
Make sure you have fortran and c++ compilers installed. Select your compiler in the Makefile.in

#### 4. Build the library  
##### The first time you open the SiteResponse.pro in Qt Creator, it will call make to build libraries. If you've set everything in Makefile.in correctly, you'll see in Qt Creator's general message winddow: SRT and FEM built successfully.
##### You can also build the libraries from the command line by issuing: make

#### 5. Build the Site Response Tool
##### by clicking Run in Qt Creator.
