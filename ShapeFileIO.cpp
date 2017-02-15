#include "stdafx.h"
#include <stdio.h>
#include "ShapeFileIO.h"
#include <vcclr.h>
using namespace Reclamation::Pn;
using namespace System::Runtime::InteropServices;



void ShapeFileIO::Open(String^ filename)
{
	this->filename = filename;
	OpenFile();
	this->hDBF=ShapeFileIO::GetDbfHandle(filename);
}

//
//
/// get handle to shape file 
void ShapeFileIO::OpenFile()
{
  int	  nShapeType,nEntities; 
  double  adfMinBound[4], adfMaxBound[4]; // x,y,z,m 
	hSHP =GetShpHandle(this->filename);

    if( hSHP == NULL )
    {
		printf( "Error:: Unable to open: file \n" );
    }
	else
	{
	 SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );
	}

	// x,y,z,m
	minX = adfMinBound[0];
	maxX = adfMaxBound[0];

	minY = adfMinBound[1];
	maxY = adfMaxBound[1]; 

	minZ = adfMinBound[2];
	maxZ = adfMaxBound[2];

	minM = adfMinBound[3];
	maxM = adfMaxBound[3];

	shapeType = nShapeType;
	bounds.X = (float)minX;
	bounds.Y = (float)minY;
	bounds.Height =(float)( maxY-minY);
	bounds.Width = (float)( maxX-minX);
	
	
}

ShapeObject^ ShapeFileIO::Shapes(int i)
{
	ShapeObject^ so = gcnew ShapeObject();
	SHPObject*	psShape;

	psShape = SHPReadObject( hSHP, i );
	so->dfMMax = psShape->dfMMax;
	so->dfMMin = psShape->dfMMin;
	so->dfXMax = psShape->dfXMax;
	so->dfXMin = psShape->dfXMin;
	so->dfYMax = psShape->dfYMax;
	so->dfYMin = psShape->dfYMin;
	so->dfZMax = psShape->dfZMax;
	so->dfZMin = psShape->dfZMin;

	so->nParts = psShape->nParts;
	so->nShapeId = psShape->nShapeId;
	so->nSHPType = psShape->nSHPType;
	so->nVertices = psShape->nVertices;
	so->panPartStart = gcnew array< int>(so->nParts);
	for(int i=0; i<so->panPartStart->Length; i++)
		so->panPartStart[i] = psShape->panPartStart[i];

	so->panPartType = gcnew array< int>(so->nParts);
	for(int i=0; i<so->panPartType->Length; i++)
		so->panPartType[i] = psShape->panPartType[i];

	so->padfX  = gcnew array<double> (so->nVertices);
    so->padfY  = gcnew array<double> (so->nVertices);
    so->padfZ  = gcnew array<double> (so->nVertices);
    so->padfM  = gcnew array<double> (so->nVertices);

	int sz = so->padfX->Length;
	for(int i=0; i<sz; i++)
	{
	so->padfX[i]  = psShape->padfX[i];
    so->padfY[i]  = psShape->padfY[i];
    so->padfZ[i]  = psShape->padfZ[i]; 
    so->padfM[i]  = psShape->padfM[i];
	}

	SHPDestroyObject( psShape );
	return so;
}


DataRow^  ShapeFileIO::Attributes(int iRecord)
{

 int		nWidth, nDecimals;
	char	szTitle[12];

	DataRow^ row = table->NewRow();
	String^ s ="";
	int ival = 0;
	double dval = 0;

for(int i = 0; i < DBFGetFieldCount(hDBF); i++ )
	{
       DBFFieldType	eType;
       eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
       switch( eType )
            {
            case FTString:
				s = gcnew String(DBFReadStringAttribute( hDBF, iRecord, i ));
				row[i] = s; 
            break;
            
            case FTInteger:
                   ival = DBFReadIntegerAttribute( hDBF, iRecord, i ) ;
				   row[i] = ival;
            break;
            case FTDouble:
				  dval = DBFReadDoubleAttribute( hDBF, iRecord, i );
				 row[i] = dval;
            break;
            
            default:
            break;
				}
	}
         
return row;
}


void ShapeFileIO::OpenDbf()
{
    int		nWidth, nDecimals;
	char	szTitle[12];

	String^ fn = Path::ChangeExtension(this->filename,".dbf");
	hDBF = ShapeFileIO::GetDbfHandle(fn);
    if( hDBF == NULL )
    {
		throw gcnew Exception("Error opening dbf file");
    }

	int numColumns = DBFGetFieldCount(hDBF);
	this->table = gcnew DataTable();
	for(int i=0; i<numColumns; i++)
	{
	DBFFieldType	eType;
	eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
	
	String^ title = gcnew String(szTitle);
	switch(eType)
	  {
	case  FTString:
		table->Columns->Add(title,System::Type::GetType("System.String"));
		break;
	case  FTInteger:
		table->Columns->Add(title,System::Type::GetType("System.Int32"));
		break;
	case  FTDouble:
		table->Columns->Add(title,System::Type::GetType("System.Double"));
		break;
	case  FTInvalid:
		throw gcnew Exception("Invalid field type in dbf file");
		break;
	  } 

	}
}
//
ShapeFileIO::~ShapeFileIO()
{
	if(hSHP != NULL)
	{
		SHPClose( hSHP );
	}
	if( hDBF != NULL)
	{
		DBFClose( hDBF );
	}
}
//
//
////-------------------------------------------------------------
///******************************************************
//Static Methods ---- Below
//*******************************************************/
////-------------------------------------------------------------
//// private
SHPHandle ShapeFileIO::GetShpHandle(String^ filename)
{
	int length = filename->Length;
	char *in_string = new char[length+1];
	for(unsigned short i = 0; i<length; i++)
	{
		in_string[i] = (char)filename[i];
	}
	in_string[length] ='\0';
	SHPHandle hSHP;

	hSHP = SHPOpen( in_string, "rb" );

	delete[] in_string;
	return hSHP;
}

// private
DBFHandle ShapeFileIO::GetDbfHandle(String^ filename)
{
	String^ fn = Path::ChangeExtension(filename,".dbf");
	int length = fn->Length;

	char *in_string = new char[length+1];
	 for(unsigned short i = 0; i<length; i++)
		{
			in_string[i] = (char)filename[i];
		}
		in_string[length] ='\0';
   DBFHandle handle;
	handle = DBFOpen( in_string, "rb" );
    if( handle == NULL )
    {
		throw gcnew Exception("Error opening dbf file");
    }
	delete[] in_string;
	return handle;

}
// static private.
void ShapeFileIO::CreateDBF(String^ filename, DataTable ^table)
{

   DBFHandle	hDBF;
   int		i;
   char* pChars = (char*)Marshal::StringToHGlobalAnsi(filename).ToPointer(); 
   hDBF = DBFCreate(  pChars );
	Marshal::FreeHGlobal((IntPtr)pChars);

    if( hDBF == NULL )
    {
		Console::Write(filename);
		Console::WriteLine(" DBFCreate failed." );
		throw gcnew System::Exception("dbfcreate failed");
    }
    
	for( i=0; i<table->Columns->Count; i++)
	{
		char* columnName;
		
		columnName = (char*)Marshal::StringToHGlobalAnsi(table->Columns[i]->ColumnName).ToPointer(); 

		if( table->Columns[i]->DataType == System::Type::GetType("System.String") )
		{
			int width = 50;
			if( table->Columns[i]->ExtendedProperties->Contains("DBFWidth") )
			{
				String ^strW = System::Convert::ToString(table->Columns[i]->ExtendedProperties["DBFWidth"]);
				width = System::Convert::ToInt32(strW);
			}
			else
			{
				Console::WriteLine("Warning: using default width of 50 for string column in dbf file");
			}
			
			if( DBFAddField( hDBF, columnName, 
				FTString, width, 0 )
				== -1 )
			{
				throw gcnew System::Exception("error creating string column in dbf");
			}
		}
		else
			if(    table->Columns[i]->DataType == System::Type::GetType("System.Double")
				||table->Columns[i]->DataType == System::Type::GetType("System.Int32") )
			{
				int width =17;
				int decimals = 0; // default for integer..

			if( table->Columns[i]->ExtendedProperties->Contains("DBFWidth") )
			{
				String ^strW = System::Convert::ToString(table->Columns[i]->ExtendedProperties["DBFWidth"]);
				width = System::Convert::ToInt32(strW);
			}
			if( table->Columns[i]->ExtendedProperties->Contains("DBFDecimals") )
			{
				String ^strD = System::Convert::ToString(table->Columns[i]->ExtendedProperties["DBFDecimals"]);
				decimals = System::Convert::ToInt32(strD);
			}
				if( DBFAddField( hDBF,  columnName,
					FTDouble, width, decimals  ) == -1 )
				{
					throw gcnew System::Exception("error creating float column in dbf");
				}

			}
			else
				{
					Console::Write("Error creating column ");
					Console::WriteLine(table->Columns[i]->ColumnName);
						throw gcnew System::Exception("Error creating dbf: column type not implemented");
				}
	Marshal::FreeHGlobal((IntPtr)columnName);
	}
	
    DBFClose( hDBF );
}

void ShapeFileIO::SaveDBF(String^ filename, DataTable ^table)
{

	CreateDBF(filename, table);
    DBFHandle	hDBF;
    int		i, iRecord;

	char* pChars = (char*)Marshal::StringToHGlobalAnsi(filename).ToPointer(); 
	hDBF = DBFOpen( pChars, "r+b" );
	Marshal::FreeHGlobal((IntPtr)pChars);
    if( hDBF == NULL )
    {
		Console::WriteLine("DBFOpen({0},\"rb+\") failed.\n", filename );
		return;
    }

	int sz = table->Rows->Count;
	for( iRecord = DBFGetRecordCount( hDBF ); iRecord<sz; iRecord++)
	{
		for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
		{
			DataRow^ row = table->Rows[iRecord];

			if( row[i] == System::DBNull::Value )
			{
				DBFWriteNULLAttribute(hDBF, iRecord, i );
			}
			else if( DBFGetFieldInfo( hDBF, i, NULL, NULL, NULL ) == FTString )
			{
			char* pChars = (char*)Marshal::StringToHGlobalAnsi(Convert::ToString(row[i])).ToPointer(); 
			DBFWriteStringAttribute(hDBF, iRecord, i, pChars );
			Marshal::FreeHGlobal((IntPtr)pChars);
			}
			else
			{
				double v = Convert::ToDouble(row[i]);
				DBFWriteDoubleAttribute(hDBF, iRecord, i, v);
			}
		}
	}

    DBFClose( hDBF );

}

void ShapeFileIO::CreateSHP(String^ filename, DataTable ^table)
{
	if ( table->Rows->Count==0)
		return;
   int nShapeType = Convert::ToInt32(table->Rows[0]["ShapeType"]);
    SHPHandle	hSHP;


     char* pChars = (char*)Marshal::StringToHGlobalAnsi(filename).ToPointer(); 
    hSHP = SHPCreate( pChars, nShapeType );
		Marshal::FreeHGlobal((IntPtr)pChars);

    if( hSHP == NULL )
    {
		Console::Write( "Unable to create:");
		Console::WriteLine(filename );
    }
    SHPClose( hSHP );
}


void ShapeFileIO::SaveShapes(String^ filename, DataTable ^table)
{
	CreateSHP(filename,table);
	SHPHandle	hSHP;
	int		nShapeType, nVertices, nParts, *panParts; //, i, nVMax;
	double	*padfX, *padfY, *padfZ;
	SHPObject	*psObject;

char* pChars = (char*)Marshal::StringToHGlobalAnsi(filename).ToPointer(); 
	hSHP = SHPOpen( pChars, "r+b" );
Marshal::FreeHGlobal((IntPtr)pChars);

	if( hSHP == NULL )
	{
		Console::WriteLine(System::String::Concat("Error Saving ",filename));
		return;
	}

	SHPGetInfo( hSHP, NULL, &nShapeType, NULL, NULL );


//	nVertices = 0;

	int sz = table->Rows->Count;
	int lastShapeNumber = Convert::ToInt32(table->Rows[sz-1]["ShapeNumber"]);
	int prevShapeNumber = 1;

	for(int shapeNumber=1; shapeNumber<=lastShapeNumber; shapeNumber++)
	{
		// select rows for this shapenumber.
		String^ sql = String::Concat("ShapeNumber = ",shapeNumber.ToString());
		array<DataRow^>^ dr = table->Select(sql);

		 nVertices = dr->Length;
		if( nVertices ==0) //null shape
		{
			// TO DO : handle null shape.
			continue;
		}
		padfX =  new double[nVertices];
		padfY = new double[nVertices];
		padfZ =  new double[nVertices];
		panParts = new int[nVertices]; // this is plenty big

		nParts = 1;
		panParts[0] = 0;
		int prevPartNumber =1;
		for(int i=0; i<nVertices; i++)
		{
			int iPartNumber = Convert::ToInt32(dr[i]["PartNumber"]);
			if( iPartNumber != prevPartNumber)
			{
				panParts[nParts]= i;
				nParts++;
			}
			padfX[i] = Convert::ToDouble(dr[i]["X"]);
			padfY[i] = Convert::ToDouble(dr[i]["Y"]);
			padfZ[i] = Convert::ToDouble(dr[i]["Z"]);
		}

		psObject = SHPCreateObject( nShapeType, -1, nParts, panParts, NULL,
			nVertices, padfX, padfY, NULL, NULL ); // note .. Z is null for now TO DO
		SHPWriteObject( hSHP, -1, psObject );
		SHPDestroyObject( psObject );

 		delete panParts;
		delete padfX;
		delete padfY;
		delete padfZ;
	}
	SHPClose( hSHP );
}
// public static...
DataTable^ ShapeFileIO::ShapesTable(String^ shpFilename) 
{

    DataTable ^table = gcnew DataTable("Shapes");    
    SHPHandle	hSHP;
    int		nShapeType, nEntities, i, iPart, vert;
    const char 	*pszPlus;
    double 	adfMinBound[4], adfMaxBound[4];

    nShapeType = -1;

  table->Columns->Add("Index",System::Type::GetType("System.Int32"));
  table->Columns->Add("ShapeType",System::Type::GetType("System.Int32"));
  table->Columns->Add("ShapeNumber",System::Type::GetType("System.Int32"));
  table->Columns->Add("VertexNumber",System::Type::GetType("System.Int32"));
  table->Columns->Add("PartNumber",System::Type::GetType("System.Int32"));
  table->Columns->Add("X",System::Type::GetType("System.Double"));
  table->Columns->Add("Y",System::Type::GetType("System.Double"));
  table->Columns->Add("Z",System::Type::GetType("System.Double"));

/* -------------------------------------------------------------------- */
/*      Open the passed shapefile.                                      */
/* -------------------------------------------------------------------- */
   	hSHP =GetShpHandle(shpFilename);
  
    if( hSHP == NULL )
    {
		Console::Write("Failed to Open ");
		Console::WriteLine(shpFilename);
	   return table;
    }

/* -------------------------------------------------------------------- */
/*      Print out the file bounds.                                      */
/* -------------------------------------------------------------------- */
    SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );
/* -------------------------------------------------------------------- */
/*	Skim over the list of shapes, printing all the vertices.	        */
/* -------------------------------------------------------------------- */
    
    vert = 1;
	for( i = 0; i < nEntities; i++ )
	{
		int		j;
		SHPObject	*psShape;

		psShape = SHPReadObject( hSHP, i );

		if( psShape->nSHPType == SHPT_NULL)
		{
		DataRow ^row = table->NewRow();

			//fprintf(ptrFileOut,"%d, %d, %d, %d, %d, %f, %f, %f\n",0,psShape->nSHPType,i+1, 0,0,0, 0,0);
			row[0] = nullptr; //,__box(0)); // 0 is undefined vertex index.
			row[1] = psShape->nSHPType;
			row[2] = nullptr; 
			row[3] = nullptr;
			row[4] = nullptr;
			row[5] = nullptr;
			row[6] = nullptr;
			row[7] = nullptr;
			table->Rows->Add(row);
		}
		else
		{
			for( j = 0, iPart = 1; j < psShape->nVertices; j++ )
			{
				const char	*pszPartType = "";
						DataRow ^row = table->NewRow();


				if( j == 0 && psShape->nParts > 0 )
					pszPartType = SHPPartTypeName( psShape->panPartType[0] );

				if( iPart < psShape->nParts
					&& psShape->panPartStart[iPart] == j )
				{
					pszPartType = SHPPartTypeName( psShape->panPartType[iPart] );
					iPart++;
					pszPlus = "+";
				}
				else
					pszPlus = " ";

				//fprintf(ptrFileOut,"%d, %d, %d, %d, %d, %f, %f, %f\n",vert,psShape->nSHPType,i+1,j+1,iPart,psShape->padfX[j], psShape->padfY[j],psShape->padfZ[j]);
			row[0] = vert;
			row[1] = psShape->nSHPType;
			row[2] = i+1;
			row[3] = j+1;
			row[4] = iPart;
			row[5] = psShape->padfX[j];
			row[6] = psShape->padfY[j];
			row[7] = psShape->padfZ[j];
			table->Rows->Add(row);
			vert++;
			}
		}
	
		SHPDestroyObject( psShape );
	}

	SHPClose( hSHP );
	//fclose(ptrFileOut);

#ifdef USE_DBMALLOC
	malloc_dump(2);
#endif

	// return(nShapeType);

return table;
}


// public
DataTable^ ShapeFileIO::IndexTable(String^ shpFilename) // input shapefile
{
	int length = shpFilename->Length;
    char *filename = new char[length+1];
    for(unsigned short idx = 0; idx<length; idx++)
		{
			filename[idx] = (char)shpFilename[idx];
		}
		filename[length] ='\0';
    SHPHandle	hSHP;
    int		nShapeType, nEntities, i, vert;
    double 	adfMinBound[4], adfMaxBound[4];
    
    nShapeType = -1;
    hSHP = SHPOpen( filename, "rb" );
  
    if( hSHP == NULL )
    {
	   printf( "Unable to open:%s\n", filename);
	   throw gcnew System::Exception("Error opening file");
    }
  DataTable ^table = gcnew DataTable();    
  table->Columns->Add("ShapeNumber",System::Type::GetType("System.Int32"));
  table->Columns->Add("ShapeType",System::Type::GetType("System.Int32"));
  table->Columns->Add("VertexCount",System::Type::GetType("System.Int32"));
  table->Columns->Add("PartCount",System::Type::GetType("System.Int32"));
  
  table->Columns->Add("XMin",System::Type::GetType("System.String"));
  table->Columns->Add("YMin",System::Type::GetType("System.String"));
  table->Columns->Add("ZMin",System::Type::GetType("System.String"));
  table->Columns->Add("MMin",System::Type::GetType("System.String"));

  table->Columns->Add("XMax",System::Type::GetType("System.String"));
  table->Columns->Add("YMax",System::Type::GetType("System.String"));
  table->Columns->Add("ZMax",System::Type::GetType("System.String"));
  table->Columns->Add("MMax",System::Type::GetType("System.String"));


    SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

    vert = 1;
    for( i = 0; i < nEntities; i++ )
    {
	DataRow^ row = table->NewRow();
	
      SHPObject	*psShape;
	    psShape = SHPReadObject( hSHP, i );

		row["ShapeNumber"] =i+1;
		row["ShapeType"] = psShape->nSHPType;
		row["VertexCount"] = psShape->nVertices;
		row["PartCount"] = psShape->nParts;
		row["XMin"] = psShape->dfXMin;
		row["YMin"] = psShape->dfYMin;
		row["ZMin"] = psShape->dfZMin;
		row["MMin"] = psShape->dfMMin;

		row["XMax"] = psShape->dfXMax;
		row["YMax"] = psShape->dfYMax;
		row["ZMax"] = psShape->dfZMax;
		row["MMax"] = psShape->dfMMax;

		table->Rows->Add(row);
        SHPDestroyObject( psShape );
    }

    SHPClose( hSHP );
 delete filename;
return table; 
}

DataTable^ ShapeFileIO::DBFTable(String ^filename)
{
	char	szTitle[12];
	int		nWidth, nDecimals;
	DBFHandle hDBF;
	hDBF = ShapeFileIO::GetDbfHandle(filename);
	int numColumns = DBFGetFieldCount(hDBF);
	DataTable ^table = gcnew DataTable("Attributes");

	for(int i=0; i<numColumns; i++)
	{
		DBFFieldType	eType;
		eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );

		String^ title = gcnew String(szTitle);
		DataColumn ^dataCol = gcnew DataColumn(title);
		dataCol->ExtendedProperties->Add("DBFWidth",nWidth.ToString());

		switch(eType)
		{
		case  FTString:
			dataCol->DataType = System::Type::GetType("System.String");
			break;
		case  FTInteger:
			dataCol->DataType = System::Type::GetType("System.Int32");
		    dataCol->ExtendedProperties->Add("DBFDecimals",nDecimals.ToString());
			break;
		case  FTDouble:
			dataCol->DataType = System::Type::GetType("System.Double");
			dataCol->ExtendedProperties->Add("DBFDecimals",nDecimals.ToString());
			break;
		case  FTInvalid:
			throw gcnew Exception("Invalid field type in dbf file. Or not implemented..");
			break;
		} 
			
	table->Columns->Add(dataCol);
	}

	table->MinimumCapacity= hDBF->nRecords;
	for(int iRecord = 0; iRecord <hDBF->nRecords; iRecord ++)
	{
		DataRow^ row = table->NewRow();
		String^ s ="";
		int ival = 0;
		double dval = 0;

		for(int i = 0; i < DBFGetFieldCount(hDBF); i++ )
		{
			DBFFieldType	eType;
			eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
			switch( eType )
			{
			case FTString:
				s = gcnew String(DBFReadStringAttribute( hDBF, iRecord, i ));
				row[i] =  s;	
				break;

			case FTInteger:
				ival = DBFReadIntegerAttribute( hDBF, iRecord, i ) ;
				row[i] =  ival;
				break;
			case FTDouble:
				dval = DBFReadDoubleAttribute( hDBF, iRecord, i );
				row[i] = dval;
				break;

			default:
				break;
			}

		}
		table->Rows->Add(row);
	}

	//Console::Write("Closing file ");
	//Console::WriteLine(filename);
	DBFClose( hDBF );
	return table;
}








