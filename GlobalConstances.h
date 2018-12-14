#ifndef GLOBALCONSTANCES_H
#define GLOBALCONSTANCES_H

#include <QString>
#include <QMap>
static const QString g_dbFileName = "data.dat";
static const QString g_dbTableName = "soil";
static const QString g_dbFEMTableName = "FEM";

static const QString g_layerNameLabel = "LayerName";
static const QString g_thicknessLabel = "Thickness";
static const QString g_densityLabel = "Density";
static const QString g_vsLabel = "Vs";
static const QString g_colorLabel = "Color";


#define CHECKED         1
#define LAYERNAME       2
#define THICKNESS       3
#define DENSITY        4
#define VS          5
#define MATERIAL 6
#define COLOR       7
#define FEM 8

#define MAXLAYERS 20

#endif // GLOBALCONSTANCES_H

