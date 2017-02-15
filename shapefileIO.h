#pragma once
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

//#include "_vcclrit.h"
#include "shapefil.h"
#include "ShapeObject.h"
//
using namespace System;
using namespace System::Data;
using namespace System::IO;
using namespace System::Drawing;
//
//
namespace Reclamation 
{
namespace Pn
{

	public ref class ShapeFileIO
	{
	public:
	  ShapeFileIO(){ }
	  ~ShapeFileIO();

	  void Open(String^ filename);
	  DataRow^  Attributes(int iRecord);


	  property int ShapeCount
	  {
		 int get() { return hSHP->nRecords;}
	  }

	  property RectangleF Bounds{ RectangleF get() {return bounds;}}
	  property int ShapeType{int get(){ return shapeType;}}

	 property double MinX { double get(){ return minX;}	 }
	 property double MaxX { double get(){ return maxX;}}

	 property double MinY { double get(){return minY;}}
	 property double MaxY { double get(){return maxY;}}

	 property double MinZ{ double get(){return minZ;}}
	 property double MaxZ{ double get(){return maxZ;}}
	
	 property double MinM{ double get(){return minM;}}
	 property double MaxM{ double get(){return maxM;}}
	

	 ShapeObject^ Shapes(int i);

	 // --- static methods ----
	static DataTable^ ShapeFileIO::DBFTable(String ^filename);
	static DataTable^ ShapeFileIO::IndexTable(String ^filename);
	static DataTable^ ShapeFileIO::ShapesTable(String ^filename);

	static void SaveDBF(String^ filename, DataTable ^table);
	static void SaveShapes(String^ filename,DataTable ^table);

	private:
	static DBFHandle GetDbfHandle(String^ filename);
	static SHPHandle GetShpHandle(String^ filename);
//
	static void CreateDBF(String^ filename, DataTable ^table);
	static void CreateSHP(String^ filename, DataTable ^table);
		void OpenFile();
	    void OpenDbf();
//	
		SHPHandle	hSHP;
		DBFHandle   hDBF;
		DataTable ^table; //template for attributes
		RectangleF bounds;
		String^ filename;
		int shapeType;
		double minX,maxX,minY,maxY,minZ,maxZ,minM,maxM;
	};
//	
	BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID 
lpvReserved) {
   Console::WriteLine("DllMain is called...");
   return TRUE;
}
//
//	
//
}
}
