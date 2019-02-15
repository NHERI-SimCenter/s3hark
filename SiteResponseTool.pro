#---------------------------------------------------------#
#                                                         #
#               Site Response Tool                        #
#                                                         #
#---------------------------------------------------------#

QT       += core gui sql quick qml webenginewidgets uitools

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SiteResponseTool
TEMPLATE = app


unix: {
# You need to modify this section if your blas and lapack is in a different place
INCLUDEPATH += Include \
            FEM \
            SiteResponse \
            UI \
            /usr/local/opt/lapack/include \
            /usr/local/include
}
win32: {
# You must append your lapack include path if it is not in a standard place.
INCLUDEPATH += FEM \
            SiteResponse
}

unix: {
# You need to modify this section if your blas and lapack is in a different place
#LIBS += /usr/local/opt/lapack/lib/libblas.3.8.0.dylib \
#        /usr/local/opt/lapack/lib/liblapack.3.8.0.dylib \
#        /usr/local/opt/lapack/lib/liblapacke.3.8.0.dylib \
#        -L/usr/local/lib -L/usr/lib  -lm -ldl -lgfortran
LIBS += -llapack -lblas
}
win32: {
# You must append your lapack libs if it is not in a standard place.
LIBS += -L"$$_PRO_FILE_PWD_/lib"
}



SOURCES += main.cpp\
        UI/MainWindow.cpp \
    UI/DatabaseManager.cpp \
    UI/BonzaTableView.cpp \
    UI/InsertWindow.cpp \
    UI/BonzaTableModel.cpp \
    UI/SiteResponse.cpp \
    UI/TabManager.cpp \
    #SiteResponse/Mesher.cpp \
    UI/JsonManager.cpp \
    UI/ElementModel.cpp \
    FEM/PM4Sand.cpp \
    FEM/PM4Silt.cpp \
    FEM/ZeroLength.cpp \
    FEM/ElasticIsotropicMaterial.cpp \
    FEM/VariableTimeStepDirectIntegrationAnalysis.cpp \
    FEM/SSPquadUP.cpp \
    FEM/SSPquad.cpp \
    FEM/Analysis.cpp \
    FEM/AnalysisModel.cpp \
    FEM/ArrayOfTaggedObjects.cpp \
    FEM/ArrayOfTaggedObjectsIter.cpp \
    FEM/BandGenLinLapackSolver.cpp \
    FEM/BandGenLinSOE.cpp \
    FEM/BandGenLinSolver.cpp \
    FEM/BeamFiberMaterial.cpp \
    FEM/BeamFiberMaterial2d.cpp \
    FEM/BeamIntegration.cpp \
    FEM/BinaryFileStream.cpp \
    FEM/Brick.cpp \
    FEM/Channel.cpp \
    FEM/CompositeResponse.cpp \
    FEM/ConstraintHandler.cpp \
    FEM/ConvergenceTest.cpp \
    FEM/CrdTransf.cpp \
    FEM/CTestNormDispIncr.cpp \
    FEM/DataFileStream.cpp \
    FEM/DataFileStreamAdd.cpp \
    FEM/DirectIntegrationAnalysis.cpp \
    FEM/DispBeamColumn3d.cpp \
    #FEM/DispBeamColumn3dWithSensitivity.cpp \
    FEM/DOF_Group.cpp \
    FEM/DOF_GrpIter.cpp \
    FEM/DOF_Numberer.cpp \
    FEM/Domain.cpp \
    FEM/DomainComponent.cpp \
    FEM/DummyElementAPI.cpp \
    FEM/DummyStream.cpp \
    FEM/EarthquakePattern.cpp \
    #FEM/EigenSOE.cpp \
    #FEM/EigenSolver.cpp \
    FEM/ElasticIsotropicThreeDimensional.cpp \
    FEM/ElasticMaterial.cpp \
    FEM/ElasticSection3d.cpp \
    FEM/Element.cpp \
    FEM/ElementalLoad.cpp \
    FEM/ElementalLoadIter.cpp \
    FEM/ElementRecorder.cpp \
    FEM/ElementResponse.cpp \
    FEM/ElementStateParameter.cpp \
    FEM/EquiSolnAlgo.cpp \
    FEM/FE_Datastore.cpp \
    FEM/FE_EleIter.cpp \
    FEM/FE_Element.cpp \
    FEM/FEM_ObjectBroker.cpp \
    FEM/FiberResponse.cpp \
    FEM/File.cpp \
    FEM/FileIter.cpp \
    FEM/FileStream.cpp \
    FEM/FrictionModel.cpp \
    FEM/FrictionResponse.cpp \
    FEM/Graph.cpp \
    FEM/GraphNumberer.cpp \
    FEM/GroundMotion.cpp \
    FEM/ID.cpp \
    FEM/ImposedMotionSP.cpp \
    FEM/IncrementalIntegrator.cpp \
    FEM/Information.cpp \
    FEM/Integrator.cpp \
    FEM/J2CyclicBoundingSurface.cpp \
    FEM/LegendreBeamIntegration.cpp \
    FEM/LinearCrdTransf3d.cpp \
    FEM/LinearSeries.cpp \
    FEM/LinearSOE.cpp \
    FEM/LinearSOESolver.cpp \
    FEM/Load.cpp \
    FEM/LoadControl.cpp \
    FEM/LoadPattern.cpp \
    FEM/LoadPatternIter.cpp \
    FEM/MapOfTaggedObjects.cpp \
    FEM/MapOfTaggedObjectsIter.cpp \
    FEM/Material.cpp \
    FEM/MaterialResponse.cpp \
    FEM/Matrix.cpp \
    FEM/MatrixUtil.cpp \
    FEM/Message.cpp \
    FEM/MovableObject.cpp \
    FEM/MP_Constraint.cpp \
    FEM/MultiSupportPattern.cpp \
    FEM/NDMaterial.cpp \
    FEM/Newmark.cpp \
    FEM/NewtonRaphson.cpp \
    FEM/NodalLoad.cpp \
    FEM/NodalLoadIter.cpp \
    FEM/Node.cpp \
    FEM/NodeRecorder.cpp \
    FEM/ObjectBroker.cpp \
    FEM/OPS_Stream.cpp \
    FEM/Parameter.cpp \
    FEM/PathSeries.cpp \
    FEM/PathTimeSeries.cpp \
    FEM/PenaltyConstraintHandler.cpp \
    FEM/PenaltyMP_FE.cpp \
    FEM/PenaltySP_FE.cpp \
    FEM/PlainNumberer.cpp \
    FEM/PlaneStrainMaterial.cpp \
    FEM/PlaneStressMaterial.cpp \
    FEM/PlateFiberMaterial.cpp \
    FEM/Pressure_Constraint.cpp \
    FEM/PySimple1.cpp \
    FEM/QzSimple1.cpp \
    FEM/RCM.cpp \
    FEM/Recorder.cpp \
    FEM/Renderer.cpp \
    FEM/Response.cpp \
    FEM/SectionForceDeformation.cpp \
    FEM/shp3d.cpp \
    FEM/SimpsonTimeSeriesIntegrator.cpp \
    FEM/SimulationInformation.cpp \
    FEM/SingleDomAllSP_Iter.cpp \
    FEM/SingleDomEleIter.cpp \
    #FEM/SingleDomLC_Iter.cpp \
    FEM/SingleDomMP_Iter.cpp \
    FEM/SingleDomNodIter.cpp \
    FEM/SingleDomParamIter.cpp \
    FEM/SingleDomPC_Iter.cpp \
    FEM/SingleDomSP_Iter.cpp \
    FEM/SolutionAlgorithm.cpp \
    FEM/SP_Constraint.cpp \
    FEM/SSPbrick.cpp \
    FEM/StandardStream.cpp \
    FEM/StaticAnalysis.cpp \
    FEM/StaticIntegrator.cpp \
    FEM/StringContainer.cpp \
    FEM/Subdomain.cpp \
    FEM/SubdomainNodIter.cpp \
    FEM/TaggedObject.cpp \
    FEM/TimeSeries.cpp \
    FEM/TimeSeriesIntegrator.cpp \
    FEM/TransformationConstraintHandler.cpp \
    FEM/TransformationDOF_Group.cpp \
    FEM/TransformationFE.cpp \
    FEM/TransientAnalysis.cpp \
    FEM/TransientIntegrator.cpp \
    FEM/TrapezoidalTimeSeriesIntegrator.cpp \
    FEM/TzSimple1.cpp \
    FEM/UniaxialMaterial.cpp \
    FEM/UniformExcitation.cpp \
    FEM/Vector.cpp \
    FEM/Vertex.cpp \
    FEM/VertexIter.cpp \
    FEM/ViscousMaterial.cpp \
    #FEM/win32Functions.cpp \
    #FEM/ErrorHandler.cpp \
    FEM/ElasticIsotropicPlaneStrain2D.cpp \
    #FEM/ElasticIsotropicPlaneStress2D.cpp \
    SiteResponse/EffectiveFEModel.cpp \
    SiteResponse/Mesher.cpp \
    SiteResponse/soillayer.cpp \
    SiteResponse/siteLayering.cpp \
    SiteResponse/outcropMotion.cpp \
    #SiteResponse/FEModel3D.cpp
    UI/ProfileManager.cpp \
    UI/PostProcessor.cpp \
    UI/RockOutcrop1D.cpp

HEADERS  += UI/MainWindow.h \
    UI/DatabaseManager.h \
    GlobalConstances.h \
    UI/BonzaTableView.h \
    UI/InsertWindow.h \
    UI/BonzaTableModel.h \
    UI/SiteResponse.h \
    UI/TabManager.h \
    #SiteResponse/Mesher.h \
    UI/JsonManager.h \
    UI/ElementModel.h \
    FEM/PM4Sand.h \
    FEM/PM4Silt.h \
    FEM/Analysis.h \
    FEM/AnalysisModel.h \
    FEM/ArrayOfTaggedObjects.h \
    FEM/ArrayOfTaggedObjectsIter.h \
    FEM/BandGenLinLapackSolver.h \
    FEM/BandGenLinSOE.h \
    FEM/BandGenLinSolver.h \
    FEM/BeamFiberMaterial.h \
    FEM/BeamFiberMaterial2d.h \
    FEM/BeamIntegration.h \
    FEM/BinaryFileStream.h \
    FEM/Brick.h \
    FEM/Channel.h \
    FEM/classTags.h \
    FEM/ColorMap.h \
    FEM/CompositeResponse.h \
    FEM/ConstraintHandler.h \
    FEM/ConvergenceTest.h \
    FEM/CrdTransf.h \
    FEM/CTestNormDispIncr.h \
    FEM/DataFileStream.h \
    FEM/DataFileStreamAdd.h \
    FEM/DirectIntegrationAnalysis.h \
    FEM/DispBeamColumn3d.h \
    #FEM/DispBeamColumn3dWithSensitivity.h \
    FEM/DOF_Group.h \
    FEM/DOF_GrpIter.h \
    FEM/DOF_Numberer.h \
    FEM/Domain.h \
    FEM/DomainComponent.h \
    FEM/DomainDecompositionAnalysis.h \
    FEM/DummyStream.h \
    FEM/EarthquakePattern.h \
    #FEM/EigenSOE.h \
    #FEM/EigenSolver.h \
    FEM/ElasticIsotropicMaterial.h \
    FEM/ElasticIsotropicThreeDimensional.h \
    FEM/ElasticMaterial.h \
    FEM/ElasticSection3d.h \
    FEM/Element.h \
    FEM/ElementalLoad.h \
    FEM/ElementalLoadIter.h \
    FEM/elementAPI.h \
    FEM/ElementIter.h \
    FEM/ElementRecorder.h \
    FEM/ElementResponse.h \
    FEM/ElementStateParameter.h \
    FEM/EquiSolnAlgo.h \
    FEM/FE_Datastore.h \
    FEM/FE_EleIter.h \
    FEM/FE_Element.h \
    FEM/FEM_ObjectBroker.h \
    FEM/Fiber.h \
    FEM/FiberResponse.h \
    FEM/File.h \
    FEM/FileIter.h \
    FEM/FileStream.h \
    FEM/FrictionModel.h \
    FEM/FrictionResponse.h \
    FEM/G3Globals.h \
    FEM/Graph.h \
    FEM/GraphNumberer.h \
    FEM/GroundMotion.h \
    FEM/ID.h \
    FEM/ImposedMotionSP.h \
    FEM/IncrementalIntegrator.h \
    FEM/Information.h \
    FEM/Integrator.h \
    FEM/J2CyclicBoundingSurface.h \
    FEM/LegendreBeamIntegration.h \
    FEM/LinearCrdTransf3d.h \
    FEM/LinearSeries.h \
    FEM/LinearSOE.h \
    FEM/LinearSOESolver.h \
    FEM/Load.h \
    FEM/LoadControl.h \
    FEM/LoadPattern.h \
    FEM/LoadPatternIter.h \
    FEM/MapOfTaggedObjects.h \
    FEM/MapOfTaggedObjectsIter.h \
    FEM/Material.h \
    FEM/MaterialResponse.h \
    FEM/Matrix.h \
    FEM/MatrixUtil.h \
    FEM/MeshRegion.h \
    FEM/Message.h \
    FEM/MovableObject.h \
    FEM/MP_Constraint.h \
    FEM/MP_ConstraintIter.h \
    FEM/MultiSupportPattern.h \
    FEM/NDMaterial.h \
    FEM/Newmark.h \
    FEM/NewtonRaphson.h \
    FEM/NodalLoad.h \
    FEM/NodalLoadIter.h \
    FEM/Node.h \
    FEM/NodeIter.h \
    FEM/NodeRecorder.h \
    FEM/ObjectBroker.h \
    FEM/OPS_Globals.h \
    FEM/OPS_Stream.h \
    FEM/Parameter.h \
    FEM/ParameterIter.h \
    FEM/PathSeries.h \
    FEM/PathTimeSeries.h \
    FEM/PenaltyConstraintHandler.h \
    FEM/PenaltyMP_FE.h \
    FEM/PenaltySP_FE.h \
    FEM/PlainMap.h \
    FEM/PlainNumberer.h \
    FEM/PlaneStrainMaterial.h \
    FEM/PlaneStressMaterial.h \
    FEM/PlateFiberMaterial.h \
    FEM/Pressure_Constraint.h \
    FEM/Pressure_ConstraintIter.h \
    FEM/PySimple1.h \
    FEM/QzSimple1.h \
    FEM/RCM.h \
    FEM/Recorder.h \
    FEM/Renderer.h \
    FEM/Response.h \
    FEM/SectionForceDeformation.h \
    FEM/shp3d.h \
    FEM/SimpsonTimeSeriesIntegrator.h \
    FEM/SimulationInformation.h \
    FEM/SingleDomAllSP_Iter.h \
    FEM/SingleDomEleIter.h \
    #FEM/SingleDomLC_Iter.h \
    FEM/SingleDomMP_Iter.h \
    FEM/SingleDomNodIter.h \
    FEM/SingleDomParamIter.h \
    FEM/SingleDomPC_Iter.h \
    FEM/SingleDomSP_Iter.h \
    FEM/SolutionAlgorithm.h \
    FEM/SP_Constraint.h \
    FEM/SP_ConstraintIter.h \
    FEM/SSPbrick.h \
    FEM/StandardStream.h \
    FEM/StaticAnalysis.h \
    FEM/StaticIntegrator.h \
    FEM/StringContainer.h \
    FEM/Subdomain.h \
    FEM/SubdomainNodIter.h \
    FEM/TaggedObject.h \
    FEM/TaggedObjectIter.h \
    FEM/TaggedObjectStorage.h \
    FEM/TimeSeries.h \
    FEM/TimeSeriesIntegrator.h \
    FEM/TransformationConstraintHandler.h \
    FEM/TransformationDOF_Group.h \
    FEM/TransformationFE.h \
    FEM/TransientAnalysis.h \
    FEM/TransientIntegrator.h \
    FEM/TrapezoidalTimeSeriesIntegrator.h \
    FEM/TzSimple1.h \
    FEM/UniaxialMaterial.h \
    FEM/UniformExcitation.h \
    FEM/VariableTimeStepDirectIntegrationAnalysis.h \
    FEM/Vector.h \
    FEM/Vertex.h \
    FEM/VertexIter.h \
    FEM/ViscousMaterial.h \
    FEM/ZeroLength.h \
    #FEM/ErrorHandler.h \
    FEM/ElasticIsotropicPlaneStrain2D.h \
    #FEM/ElasticIsotropicPlaneStress2D.h \
    FEM/SSPquad.h \
    FEM/SSPquadUP.h \
    SiteResponse/Mesher.h \
    SiteResponse/EffectiveFEModel.h \
    SiteResponse/soillayer.h \
    SiteResponse/outcropMotion.h \
    SiteResponse/siteLayering.h \
    UI/ProfileManager.h \
    UI/PostProcessor.h \
    UI/RockOutcrop1D.h


FORMS    += UI/MainWindow.ui \
    UI/InsertWindow.ui \
    UI/PM4Sand.ui \
    UI/ElasticIsotropic.ui \
    UI/DefaultMatTab.ui \
    UI/GroundMotion.ui \
    UI/FEM.ui \
    UI/RockOutcrop1D.ui

RESOURCES += \
    resources.qrc





