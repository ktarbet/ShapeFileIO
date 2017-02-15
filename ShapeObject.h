//#pragma once

using namespace System;
using namespace cli;
namespace Reclamation{
namespace Pn {
public ref class ShapeObject
{
public:
	ShapeObject(void);
	~ShapeObject(void);

   int		nSHPType;

    int		nShapeId; /* -1 is unknown/unassigned */

    int		nParts;
	array< int >^ panPartStart; // = gcnew array< Int32 >(ARRAY_SIZE);
	array< int >^ panPartType;
    int		nVertices;
	array<double >^	padfX;
    array<double >^	padfY;
    array<double >^	padfZ;
    array<double >^	padfM;

    double	dfXMin;
    double	dfYMin;
    double	dfZMin;
    double	dfMMin;

    double	dfXMax;
    double	dfYMax;
    double	dfZMax;
    double	dfMMax;
};

}}