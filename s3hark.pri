

INCLUDEPATH += $$PWD/ \
               $$PWD/Include \
               $$PWD/FEM \
               $$PWD/SiteResponse \
               $$PWD/UI \
               #/Users/simcenter/Codes/Research/OpenSees/OpenSees/OTHER/SuperLU_5.1.1/SRC

unix {
# You need to modify this section if your blas and lapack is in a different place
INCLUDEPATH += /usr/local/opt/lapack/include \
               /usr/local/include \


# You need to modify this section if your blas and lapack is in a different place
#LIBS += /usr/local/opt/lapack/lib/libblas.3.8.0.dylib \
#        /usr/local/opt/lapack/lib/liblapack.3.8.0.dylib \
#        /usr/local/opt/lapack/lib/liblapacke.3.8.0.dylib \
#        -L/usr/local/lib -L/usr/lib  -lm -ldl -lgfortran
LIBS += -llapack
}

win32: {
# You must append your lapack include path if it is not in a standard place.
INCLUDEPATH += FEM \
            += SiteResponse

LIBS += "The path to the file lapack.lib"
LIBS += "The path to the file blas.lib"
LIBS += "The path to the file libifcoremt.lib"
LIBS += "The path to the file libirc.lib"
LIBS += "The path to the file ifconsol.lib"
LIBS += "The path to the file libifport.lib"
LIBS += "The path to the file libmmt.lib"
LIBS += "The path to the file svml_dispmt.lib"
# You must append your lapack include path if it is not in a standard place.
LIBS += -L"$$_PRO_FILE_PWD_/lib"
}


SOURCES += $$PWD/UI/RockOutcrop.cpp \
    $$PWD/UI/BonzaTableView.cpp \
    $$PWD/UI/InsertWindow.cpp \
    $$PWD/UI/BonzaTableModel.cpp \
    $$PWD/UI/SiteResponse.cpp \
    $$PWD/UI/TabManager.cpp \
    #$$PWD/SiteResponse/Mesher.cpp \
    $$PWD/UI/JsonManager.cpp \
    $$PWD/UI/ElementModel.cpp \
    $$PWD/FEM/PM4Sand.cpp \
    $$PWD/FEM/PM4Silt.cpp \
    $$PWD/FEM/ZeroLength.cpp \
    $$PWD/FEM/ElasticIsotropicMaterial.cpp \
    $$PWD/FEM/VariableTimeStepDirectIntegrationAnalysis.cpp \
    $$PWD/FEM/SSPquadUP.cpp \
    $$PWD/FEM/SSPquad.cpp \
    $$PWD/FEM/Analysis.cpp \
    $$PWD/FEM/AnalysisModel.cpp \
    $$PWD/FEM/ArrayOfTaggedObjects.cpp \
    $$PWD/FEM/ArrayOfTaggedObjectsIter.cpp \
    $$PWD/FEM/BandGenLinLapackSolver.cpp \
    $$PWD/FEM/BandGenLinSOE.cpp \
    $$PWD/FEM/BandGenLinSolver.cpp \
    $$PWD/FEM/BeamFiberMaterial.cpp \
    $$PWD/FEM/BeamFiberMaterial2d.cpp \
    $$PWD/FEM/BeamIntegration.cpp \
    $$PWD/FEM/BinaryFileStream.cpp \
    $$PWD/FEM/Brick.cpp \
    $$PWD/FEM/Channel.cpp \
    $$PWD/FEM/CompositeResponse.cpp \
    $$PWD/FEM/ConstraintHandler.cpp \
    $$PWD/FEM/ConvergenceTest.cpp \
    $$PWD/FEM/CrdTransf.cpp \
    $$PWD/FEM/CTestNormDispIncr.cpp \
    $$PWD/FEM/DataFileStream.cpp \
    $$PWD/FEM/DataFileStreamAdd.cpp \
    $$PWD/FEM/DirectIntegrationAnalysis.cpp \
    $$PWD/FEM/DispBeamColumn3d.cpp \
    #$$PWD/FEM/DispBeamColumn3dWithSensitivity.cpp \
    $$PWD/FEM/DOF_Group.cpp \
    $$PWD/FEM/DOF_GrpIter.cpp \
    $$PWD/FEM/DOF_Numberer.cpp \
    $$PWD/FEM/Domain.cpp \
    $$PWD/FEM/DomainComponent.cpp \
    $$PWD/FEM/DummyElementAPI.cpp \
    $$PWD/FEM/DummyStream.cpp \
    $$PWD/FEM/EarthquakePattern.cpp \
    #$$PWD/FEM/EigenSOE.cpp \
    #$$PWD/FEM/EigenSolver.cpp \
    $$PWD/FEM/ElasticIsotropicThreeDimensional.cpp \
    $$PWD/FEM/ElasticMaterial.cpp \
    $$PWD/FEM/ElasticSection3d.cpp \
    $$PWD/FEM/Element.cpp \
    $$PWD/FEM/ElementalLoad.cpp \
    $$PWD/FEM/ElementalLoadIter.cpp \
    $$PWD/FEM/ElementRecorder.cpp \
    $$PWD/FEM/ElementResponse.cpp \
    $$PWD/FEM/ElementStateParameter.cpp \
    $$PWD/FEM/EquiSolnAlgo.cpp \
    $$PWD/FEM/FE_Datastore.cpp \
    $$PWD/FEM/FE_EleIter.cpp \
    $$PWD/FEM/FE_Element.cpp \
    $$PWD/FEM/FEM_ObjectBroker.cpp \
    $$PWD/FEM/FiberResponse.cpp \
    $$PWD/FEM/File.cpp \
    $$PWD/FEM/FileIter.cpp \
    $$PWD/FEM/FileStream.cpp \
    $$PWD/FEM/FrictionModel.cpp \
    $$PWD/FEM/FrictionResponse.cpp \
    $$PWD/FEM/Graph.cpp \
    $$PWD/FEM/GraphNumberer.cpp \
    $$PWD/FEM/GroundMotion.cpp \
    $$PWD/FEM/ID.cpp \
    $$PWD/FEM/ImposedMotionSP.cpp \
    $$PWD/FEM/IncrementalIntegrator.cpp \
    $$PWD/FEM/Information.cpp \
    $$PWD/FEM/Integrator.cpp \
    $$PWD/FEM/J2CyclicBoundingSurface.cpp \
    $$PWD/FEM/ManzariDafalias.cpp \
    $$PWD/FEM/ManzariDafalias3D.cpp \
    $$PWD/FEM/ManzariDafaliasPlaneStrain.cpp \
    $$PWD/FEM/LegendreBeamIntegration.cpp \
    $$PWD/FEM/LinearCrdTransf3d.cpp \
    $$PWD/FEM/LinearSeries.cpp \
    $$PWD/FEM/LinearSOE.cpp \
    $$PWD/FEM/LinearSOESolver.cpp \
    $$PWD/FEM/Load.cpp \
    $$PWD/FEM/LoadControl.cpp \
    $$PWD/FEM/LoadPattern.cpp \
    $$PWD/FEM/LoadPatternIter.cpp \
    $$PWD/FEM/MapOfTaggedObjects.cpp \
    $$PWD/FEM/MapOfTaggedObjectsIter.cpp \
    $$PWD/FEM/Material.cpp \
    $$PWD/FEM/MaterialResponse.cpp \
    $$PWD/FEM/Matrix.cpp \
    $$PWD/FEM/MatrixUtil.cpp \
    $$PWD/FEM/Message.cpp \
    $$PWD/FEM/MovableObject.cpp \
    $$PWD/FEM/MP_Constraint.cpp \
    $$PWD/FEM/MultiSupportPattern.cpp \
    $$PWD/FEM/NDMaterial.cpp \
    $$PWD/FEM/Newmark.cpp \
    $$PWD/FEM/NewtonRaphson.cpp \
    $$PWD/FEM/NodalLoad.cpp \
    $$PWD/FEM/NodalLoadIter.cpp \
    $$PWD/FEM/Node.cpp \
    $$PWD/FEM/NodeRecorder.cpp \
    $$PWD/FEM/ObjectBroker.cpp \
    $$PWD/FEM/OPS_Stream.cpp \
    $$PWD/FEM/Parameter.cpp \
    $$PWD/FEM/PathSeries.cpp \
    $$PWD/FEM/PathTimeSeries.cpp \
    $$PWD/FEM/PenaltyConstraintHandler.cpp \
    $$PWD/FEM/PenaltyMP_FE.cpp \
    $$PWD/FEM/PenaltySP_FE.cpp \
    $$PWD/FEM/PlainNumberer.cpp \
    $$PWD/FEM/PlaneStrainMaterial.cpp \
    $$PWD/FEM/PlaneStressMaterial.cpp \
    $$PWD/FEM/PlateFiberMaterial.cpp \
    $$PWD/FEM/Pressure_Constraint.cpp \
    $$PWD/FEM/PySimple1.cpp \
    $$PWD/FEM/QzSimple1.cpp \
    $$PWD/FEM/RCM.cpp \
    $$PWD/FEM/Recorder.cpp \
    $$PWD/FEM/Renderer.cpp \
    $$PWD/FEM/Response.cpp \
    $$PWD/FEM/SectionForceDeformation.cpp \
    $$PWD/FEM/shp3d.cpp \
    $$PWD/FEM/SimpsonTimeSeriesIntegrator.cpp \
    $$PWD/FEM/SimulationInformation.cpp \
    $$PWD/FEM/SingleDomAllSP_Iter.cpp \
    $$PWD/FEM/SingleDomEleIter.cpp \
    #$$PWD/FEM/SingleDomLC_Iter.cpp \
    $$PWD/FEM/SingleDomMP_Iter.cpp \
    $$PWD/FEM/SingleDomNodIter.cpp \
    $$PWD/FEM/SingleDomParamIter.cpp \
    $$PWD/FEM/SingleDomPC_Iter.cpp \
    $$PWD/FEM/SingleDomSP_Iter.cpp \
    $$PWD/FEM/SolutionAlgorithm.cpp \
    $$PWD/FEM/SP_Constraint.cpp \
    $$PWD/FEM/SSPbrick.cpp \
    $$PWD/FEM/SSPbrickUP.cpp \
    $$PWD/FEM/StandardStream.cpp \
    $$PWD/FEM/StaticAnalysis.cpp \
    $$PWD/FEM/StaticIntegrator.cpp \
    $$PWD/FEM/StringContainer.cpp \
    $$PWD/FEM/Subdomain.cpp \
    $$PWD/FEM/SubdomainNodIter.cpp \
    $$PWD/FEM/TaggedObject.cpp \
    $$PWD/FEM/TimeSeries.cpp \
    $$PWD/FEM/TimeSeriesIntegrator.cpp \
    $$PWD/FEM/TransformationConstraintHandler.cpp \
    $$PWD/FEM/TransformationDOF_Group.cpp \
    $$PWD/FEM/TransformationFE.cpp \
    $$PWD/FEM/TransientAnalysis.cpp \
    $$PWD/FEM/TransientIntegrator.cpp \
    $$PWD/FEM/TrapezoidalTimeSeriesIntegrator.cpp \
    $$PWD/FEM/TzSimple1.cpp \
    $$PWD/FEM/UniaxialMaterial.cpp \
    $$PWD/FEM/UniformExcitation.cpp \
    $$PWD/FEM/Vector.cpp \
    $$PWD/FEM/Vertex.cpp \
    $$PWD/FEM/VertexIter.cpp \
    $$PWD/FEM/ViscousMaterial.cpp \
    #$$PWD/FEM/win32Functions.cpp \
    #$$PWD/FEM/ErrorHandler.cpp \
    $$PWD/FEM/ElasticIsotropicPlaneStrain2D.cpp \
    #$$PWD/FEM/ElasticIsotropicPlaneStress2D.cpp \
    $$PWD/FEM/MultiYieldSurface.cpp \
    $$PWD/FEM/PressureDependMultiYield02.cpp \
    $$PWD/FEM/PressureDependMultiYield.cpp \
    $$PWD/FEM/PressureIndependMultiYield.cpp \
    $$PWD/SiteResponse/EffectiveFEModel.cpp \
    $$PWD/SiteResponse/Mesher.cpp \
    $$PWD/SiteResponse/soillayer.cpp \
    $$PWD/SiteResponse/siteLayering.cpp \
    $$PWD/SiteResponse/outcropMotion.cpp \
    #$$PWD/SiteResponse/FEModel3D.cpp
    $$PWD/UI/ProfileManager.cpp \
    $$PWD/UI/PostProcessor.cpp \
    $$PWD/UI/SSSharkThread.cpp \
    $$PWD/FEM/T2Vector.cpp \
    $$PWD/FEM/PlainHandler.cpp \
    #$$PWD/FEM/SuperLU.cpp \
    #$$PWD/FEM/SparseGenColLinSOE.cpp \
    #$$PWD/FEM/Accelerator.cpp \
    #$$PWD/FEM/AcceleratedNewton.cpp \
    #$$PWD/FEM/KrylovAccelerator.cpp \
    #$$PWD/FEM/KrylovNewton.cpp \
    #$$PWD/FEM/SparseGenColLinSolver.cpp


HEADERS  += $$PWD/UI/RockOutcrop.h \
    $$PWD/GlobalConstances.h \
    $$PWD/UI/BonzaTableView.h \
    $$PWD/UI/InsertWindow.h \
    $$PWD/UI/BonzaTableModel.h \
    $$PWD/UI/SiteResponse.h \
    $$PWD/UI/TabManager.h \
    #$$PWD/SiteResponse/Mesher.h \
    $$PWD/UI/JsonManager.h \
    $$PWD/UI/ElementModel.h \
    $$PWD/FEM/PM4Sand.h \
    $$PWD/FEM/PM4Silt.h \
    $$PWD/FEM/Analysis.h \
    $$PWD/FEM/AnalysisModel.h \
    $$PWD/FEM/ArrayOfTaggedObjects.h \
    $$PWD/FEM/ArrayOfTaggedObjectsIter.h \
    $$PWD/FEM/BandGenLinLapackSolver.h \
    $$PWD/FEM/BandGenLinSOE.h \
    $$PWD/FEM/BandGenLinSolver.h \
    $$PWD/FEM/BeamFiberMaterial.h \
    $$PWD/FEM/BeamFiberMaterial2d.h \
    $$PWD/FEM/BeamIntegration.h \
    $$PWD/FEM/BinaryFileStream.h \
    $$PWD/FEM/Brick.h \
    $$PWD/FEM/Channel.h \
    $$PWD/FEM/classTags.h \
    $$PWD/FEM/ColorMap.h \
    $$PWD/FEM/CompositeResponse.h \
    $$PWD/FEM/ConstraintHandler.h \
    $$PWD/FEM/ConvergenceTest.h \
    $$PWD/FEM/CrdTransf.h \
    $$PWD/FEM/CTestNormDispIncr.h \
    $$PWD/FEM/DataFileStream.h \
    $$PWD/FEM/DataFileStreamAdd.h \
    $$PWD/FEM/DirectIntegrationAnalysis.h \
    $$PWD/FEM/DispBeamColumn3d.h \
    #$$PWD/FEM/DispBeamColumn3dWithSensitivity.h \
    $$PWD/FEM/DOF_Group.h \
    $$PWD/FEM/DOF_GrpIter.h \
    $$PWD/FEM/DOF_Numberer.h \
    $$PWD/FEM/Domain.h \
    $$PWD/FEM/DomainComponent.h \
    $$PWD/FEM/DomainDecompositionAnalysis.h \
    $$PWD/FEM/DummyStream.h \
    $$PWD/FEM/EarthquakePattern.h \
    #$$PWD/FEM/EigenSOE.h \
    #$$PWD/FEM/EigenSolver.h \
    $$PWD/FEM/ElasticIsotropicMaterial.h \
    $$PWD/FEM/ElasticIsotropicThreeDimensional.h \
    $$PWD/FEM/ElasticMaterial.h \
    $$PWD/FEM/ElasticSection3d.h \
    $$PWD/FEM/Element.h \
    $$PWD/FEM/ElementalLoad.h \
    $$PWD/FEM/ElementalLoadIter.h \
    $$PWD/FEM/elementAPI.h \
    $$PWD/FEM/ElementIter.h \
    $$PWD/FEM/ElementRecorder.h \
    $$PWD/FEM/ElementResponse.h \
    $$PWD/FEM/ElementStateParameter.h \
    $$PWD/FEM/EquiSolnAlgo.h \
    $$PWD/FEM/FE_Datastore.h \
    $$PWD/FEM/FE_EleIter.h \
    $$PWD/FEM/FE_Element.h \
    $$PWD/FEM/FEM_ObjectBroker.h \
    $$PWD/FEM/Fiber.h \
    $$PWD/FEM/FiberResponse.h \
    $$PWD/FEM/File.h \
    $$PWD/FEM/FileIter.h \
    $$PWD/FEM/FileStream.h \
    $$PWD/FEM/FrictionModel.h \
    $$PWD/FEM/FrictionResponse.h \
    $$PWD/FEM/G3Globals.h \
    $$PWD/FEM/Graph.h \
    $$PWD/FEM/GraphNumberer.h \
    $$PWD/FEM/GroundMotion.h \
    $$PWD/FEM/ID.h \
    $$PWD/FEM/ImposedMotionSP.h \
    $$PWD/FEM/IncrementalIntegrator.h \
    $$PWD/FEM/Information.h \
    $$PWD/FEM/Integrator.h \
    $$PWD/FEM/J2CyclicBoundingSurface.h \
    $$PWD/FEM/ManzariDafalias.h \
    $$PWD/FEM/ManzariDafalias3D.h \
    $$PWD/FEM/ManzariDafaliasPlaneStrain.h \
    $$PWD/FEM/LegendreBeamIntegration.h \
    $$PWD/FEM/LinearCrdTransf3d.h \
    $$PWD/FEM/LinearSeries.h \
    $$PWD/FEM/LinearSOE.h \
    $$PWD/FEM/LinearSOESolver.h \
    $$PWD/FEM/Load.h \
    $$PWD/FEM/LoadControl.h \
    $$PWD/FEM/LoadPattern.h \
    $$PWD/FEM/LoadPatternIter.h \
    $$PWD/FEM/MapOfTaggedObjects.h \
    $$PWD/FEM/MapOfTaggedObjectsIter.h \
    $$PWD/FEM/Material.h \
    $$PWD/FEM/MaterialResponse.h \
    $$PWD/FEM/Matrix.h \
    $$PWD/FEM/MatrixUtil.h \
    $$PWD/FEM/MeshRegion.h \
    $$PWD/FEM/Message.h \
    $$PWD/FEM/MovableObject.h \
    $$PWD/FEM/MP_Constraint.h \
    $$PWD/FEM/MP_ConstraintIter.h \
    $$PWD/FEM/MultiSupportPattern.h \
    $$PWD/FEM/NDMaterial.h \
    $$PWD/FEM/Newmark.h \
    $$PWD/FEM/NewtonRaphson.h \
    $$PWD/FEM/NodalLoad.h \
    $$PWD/FEM/NodalLoadIter.h \
    $$PWD/FEM/Node.h \
    $$PWD/FEM/NodeIter.h \
    $$PWD/FEM/NodeRecorder.h \
    $$PWD/FEM/ObjectBroker.h \
    $$PWD/FEM/OPS_Globals.h \
    $$PWD/FEM/OPS_Stream.h \
    $$PWD/FEM/Parameter.h \
    $$PWD/FEM/ParameterIter.h \
    $$PWD/FEM/PathSeries.h \
    $$PWD/FEM/PathTimeSeries.h \
    $$PWD/FEM/PenaltyConstraintHandler.h \
    $$PWD/FEM/PenaltyMP_FE.h \
    $$PWD/FEM/PenaltySP_FE.h \
    $$PWD/FEM/PlainMap.h \
    $$PWD/FEM/PlainNumberer.h \
    $$PWD/FEM/PlaneStrainMaterial.h \
    $$PWD/FEM/PlaneStressMaterial.h \
    $$PWD/FEM/PlateFiberMaterial.h \
    $$PWD/FEM/Pressure_Constraint.h \
    $$PWD/FEM/Pressure_ConstraintIter.h \
    $$PWD/FEM/PySimple1.h \
    $$PWD/FEM/QzSimple1.h \
    $$PWD/FEM/RCM.h \
    $$PWD/FEM/Recorder.h \
    $$PWD/FEM/Renderer.h \
    $$PWD/FEM/Response.h \
    $$PWD/FEM/SectionForceDeformation.h \
    $$PWD/FEM/shp3d.h \
    $$PWD/FEM/SimpsonTimeSeriesIntegrator.h \
    $$PWD/FEM/SimulationInformation.h \
    $$PWD/FEM/SingleDomAllSP_Iter.h \
    $$PWD/FEM/SingleDomEleIter.h \
    #$$PWD/FEM/SingleDomLC_Iter.h \
    $$PWD/FEM/SingleDomMP_Iter.h \
    $$PWD/FEM/SingleDomNodIter.h \
    $$PWD/FEM/SingleDomParamIter.h \
    $$PWD/FEM/SingleDomPC_Iter.h \
    $$PWD/FEM/SingleDomSP_Iter.h \
    $$PWD/FEM/SolutionAlgorithm.h \
    $$PWD/FEM/SP_Constraint.h \
    $$PWD/FEM/SP_ConstraintIter.h \
    $$PWD/FEM/SSPbrick.h \
    $$PWD/FEM/SSPbrickUP.h \
    $$PWD/FEM/StandardStream.h \
    $$PWD/FEM/StaticAnalysis.h \
    $$PWD/FEM/StaticIntegrator.h \
    $$PWD/FEM/StringContainer.h \
    $$PWD/FEM/Subdomain.h \
    $$PWD/FEM/SubdomainNodIter.h \
    $$PWD/FEM/TaggedObject.h \
    $$PWD/FEM/TaggedObjectIter.h \
    $$PWD/FEM/TaggedObjectStorage.h \
    $$PWD/FEM/TimeSeries.h \
    $$PWD/FEM/TimeSeriesIntegrator.h \
    $$PWD/FEM/TransformationConstraintHandler.h \
    $$PWD/FEM/TransformationDOF_Group.h \
    $$PWD/FEM/TransformationFE.h \
    $$PWD/FEM/TransientAnalysis.h \
    $$PWD/FEM/TransientIntegrator.h \
    $$PWD/FEM/TrapezoidalTimeSeriesIntegrator.h \
    $$PWD/FEM/TzSimple1.h \
    $$PWD/FEM/UniaxialMaterial.h \
    $$PWD/FEM/UniformExcitation.h \
    $$PWD/FEM/VariableTimeStepDirectIntegrationAnalysis.h \
    $$PWD/FEM/Vector.h \
    $$PWD/FEM/Vertex.h \
    $$PWD/FEM/VertexIter.h \
    $$PWD/FEM/ViscousMaterial.h \
    $$PWD/FEM/ZeroLength.h \
    #$$PWD/FEM/ErrorHandler.h \
    $$PWD/FEM/ElasticIsotropicPlaneStrain2D.h \
    #$$PWD/FEM/ElasticIsotropicPlaneStress2D.h \
    $$PWD/FEM/SSPquad.h \
    $$PWD/FEM/SSPquadUP.h \
    $$PWD/FEM/MultiYieldSurface.h \
    $$PWD/FEM/PressureDependMultiYield02.h \
    $$PWD/FEM/PressureDependMultiYield.h \
    $$PWD/FEM/PressureIndependMultiYield.h \
    $$PWD/SiteResponse/Mesher.h \
    $$PWD/SiteResponse/EffectiveFEModel.h \
    $$PWD/SiteResponse/soillayer.h \
    $$PWD/SiteResponse/outcropMotion.h \
    $$PWD/SiteResponse/siteLayering.h \
    $$PWD/UI/ProfileManager.h \
    $$PWD/UI/PostProcessor.h \
    $$PWD/UI/SSSharkThread.h \
    $$PWD/FEM/T2Vector.h \
    $$PWD/FEM/PlainHandler.h \

    #$$PWD/FEM/SuperLU.h \
    #$$PWD/FEM/SparseGenColLinSOE.h \
    #$$PWD/FEM/Accelerator.h \
    #$$PWD/FEM/AcceleratedNewton.h \
    #$$PWD/FEM/KrylovAccelerator.h \
    #$$PWD/FEM/KrylovNewton.h \
    #$$PWD/FEM/SparseGenColLinSolver.h


FORMS    += $$PWD/UI/MainWindow.ui \
    $$PWD/UI/RockOutcrop.ui \
    $$PWD/UI/InsertWindow.ui \
    $$PWD/UI/PM4Sand.ui \
    $$PWD/UI/PM4Silt.ui \
    $$PWD/UI/ElasticIsotropic.ui \
    $$PWD/UI/DefaultMatTab.ui \
    $$PWD/UI/GroundMotion.ui \
    $$PWD/UI/FEM.ui

RESOURCES += \
    $$PWD/resources.qrc

