﻿#ifndef  __JBXL_OS_MESH_MAKER_H_
#define  __JBXL_OS_MESH_MAKER_H_

/**
@brief OpenSim用メッシュデータの生成 
@file  MeshMaker.h

@author  Fumi.Iseki 
@date    2014 9/16

@sa OpenSim/Region/Physics/Meshing/Meshmerizer.cs
*/

#include <vector>
#include <algorithm>

#include "Vector.h"
#include "Rotation.h"
#include "ColladaTool.h"

#include "PrimMesher.h"
#include "SculptMesher.h"
#include "TerrainMesher.h"
#include "MaterialTool.h"
#include "LogDocTool.h"

#include "Jpeg2KTool.h"
#include "TgaTool.h"

#include "gz_tool.h"
#include "llsd_tool.h"


namespace  jbxl {


///////////////////////////////////////////////////////////////////////////////////////////////
//

MeshObjectData* MeshObjectDataFromPrimShape(PrimBaseShape shape, tList* resourceList, bool useBrep, bool addParam);

PrimMesh        GeneratePrimMesh(PrimMeshParam param);
TriPolygonData* TriPolygonDataFromPrimMesh(PrimMesh primMesh, int* fnum, int* pnum);


///////////////////////////////////////////////////////////////////////////////////////////////
// for Scuplpted Prim
// @sa http://wiki.secondlife.com/wiki/Sculpted_Prims:_Technical_Explanation

ContourBaseData* ContourBaseDataFromSculptImage(MSGraph<uByte> grd,  int type);
ContourBaseData* ContourBaseDataFromSculptJP2K(const char* jpegfile, int type);

TriPolygonData*  TriPolygonDataFromSculptImage(MSGraph<uByte> grd,  int type, int* pnum);
TriPolygonData*  TriPolygonDataFromSculptJP2K(const char* jpegfile, int type, int* pnum);

TriPolygonData*  GenerateGrayScaleSculpt(int* pnum);


///////////////////////////////////////////////////////////////////////////////////////////////
// for LLMesh
// @sa http://wiki.secondlife.com/wiki/Mesh/Mesh_Asset_Format

TriPolygonData*  TriPolygonDataFromLLMesh(uByte* mesh, int sz, int* fnum, int* pnum);
TriPolygonData*  TriPolygonDataFromLLMeshFile(const char* filename,  int* fnum, int* pnum);

tXML*            GetLLsdXMLFromLLMesh(uByte* mesh, int sz, const char* key);

Vector<float>*   GetLLMeshPositionDomainMaxMin(tXML* xml, int facet, bool max=true);
UVMap<float>*    GetLLMeshTexCoordDomainMaxMin(tXML* xml, int facet, bool max=true);

#define  GetLLMeshPositionDomainMax(x, v)  GetLLMeshPositionDomainMaxMin((x), (v), true)
#define  GetLLMeshPositionDomainMin(x, v)  GetLLMeshPositionDomainMaxMin((x), (v), false)
#define  GetLLMeshTexCoordDomainMax(x, v)  GetLLMeshTexCoordDomainMaxMin((x), (v), true)
#define  GetLLMeshTexCoordDomainMin(x, v)  GetLLMeshTexCoordDomainMaxMin((x), (v), false)

float  LLMeshUint16toFloat(uByte* ptr, float max, float min);


///////////////////////////////////////////////////////////////////////////////////////////////
// for Terrain Mesh

ContourBaseData*  ContourBaseDataFromTerrainImage(MSGraph<float> grd, Vector<float> shift, bool left, bool right, bool top, bool bottom, bool autosea);
ContourBaseData*  ContourBaseDataFromTerrainR32(char* r32file, int xsize, int ysize, Vector<float> shift, bool autosea);            // 未使用

TriPolygonData*   TriPolygonDataFromTerrainImage(MSGraph<float> grd, int* pnum, Vector<float> shift, bool left, bool right, bool top, bool bottom, bool autosea);// 未使用
TriPolygonData*   TriPolygonDataFromTerrainR32(char* r32file, int* pnum, int xsize, int ysize, Vector<float> shift, bool autosea);   // 未使用



}       // namespace

#endif

