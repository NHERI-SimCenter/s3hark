#------------------------------------------------------------------------------#
#             Site-specific Seismic Hazard Analysis & Research Kit             #
#                      A Site Response Analysis Tool                           #
#                                                                              #
#------------------------------------------------------------------------------#

QT       += core gui quick qml webenginewidgets uitools webengine webchannel

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = S3hark
TEMPLATE = app




include(../SimCenterCommon/Common/Common.pri)

include(./s3hark.pri)




# Notes:
# /usr/local/lib/libJPEG.dylib -> /usr/local/Cellar/jpeg/9c/lib/libjpeg.dylib
# /System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/libJPEG.dylib

# /usr/local/lib/libGIF.dylib -> /usr/local/Cellar/giflib/5.1.4_1/lib/libgif.dylib
# /System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/libGIF.dylib

# /usr/local/lib/libTIFF.dylib -> /usr/local/Cellar/libtiff/4.0.9_4/lib/libtiff.dylib
# /System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/libTIFF.dylib

# /usr/local/lib/libPng.dylib -> /usr/local/Cellar/libpng/1.6.35/lib/libpng.dylib
# /System/Library/Frameworks/ImageIO.framework/Versions/A/Resources/libPng.dylib
