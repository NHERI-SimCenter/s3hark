#ifndef GLOBALCONSTANCES_H
#define GLOBALCONSTANCES_H

#include <QString>
#include <QMap>
static const QString g_dbFileName = "database.dat";
static const QString g_dbTableName = "soil";
static const QString g_dbFEMTableName = "FEM";

static const QString g_layerNameLabel = "LayerName";
static const QString g_thicknessLabel = "Thickness";
static const QString g_densityLabel = "Density";
static const QString g_vsLabel = "Vs";
static const QString g_colorLabel = "Color";

#define LayerID 0
#define CHECKED         1
#define LAYERNAME       2
#define THICKNESS       3
#define DENSITY        4
#define VS          5
#define MATERIAL 6
#define ESIZE 7
#define COLOR 8
#define FEM 9
#define LASTCOL 9


#define DefaultThickness 3
#define DefaultDensity 2.0
#define DefaultVs 180
#define DefaultEType "Elastic"
#define DefaultESize 1

#define MAXLAYERS 1000
#define PAGESIZE 1000

#define useJSONasInput true


#endif // GLOBALCONSTANCES_H

