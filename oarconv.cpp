﻿/** 
@brief   Convert OAR Files to Collada Files

@author  Fumi.Iseki 
@date    2015 5/31
@version 1.1
*/

#include "oarconv.h"


using namespace jbxl;


int main(int argc, char** argv)
{
    Buffer inpdir = init_Buffer();
    Buffer outdir = init_Buffer();
    Buffer astdir = init_Buffer();
    Buffer infile = init_Buffer();
    Buffer cmmnd  = init_Buffer();
    Buffer tscale = init_Buffer();

    float xshift  = 0.0;
    float yshift  = 0.0;
    float zshift  = 0.0;

    int   strtnum = 0;
    int   stopnum = -1;
    int   format  = JBXL_3D_FORMAT_DAE;
    int   engine  = JBXL_3D_ENGINE_UNITY;

    bool  degeneracy = false;
    //bool  useBrep = false;
    bool  useBrep = true;
    bool  procJoints = false;

    for (int i=1; i<argc; i++) {
        if      (!strcmp(argv[i], "-i")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &inpdir); i++;}}
        else if (!strcmp(argv[i], "-f")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &infile); i++;}}
        else if (!strcmp(argv[i], "-o")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &outdir); i++;}}
        else if (!strcmp(argv[i], "-a")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &astdir); i++;}}
        else if (!strcmp(argv[i], "-t")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &tscale); i++;}}
        else if (!strcmp(argv[i], "-c")) { if (i!=argc-1){ copy_s2Buffer(argv[i+1], &cmmnd);  i++;}}
        else if (!strcmp(argv[i], "-s")) { if (i!=argc-1){ strtnum = atoi(argv[i+1]); i++;}}
        else if (!strcmp(argv[i], "-e")) { if (i!=argc-1){ stopnum = atoi(argv[i+1]); i++;}}
        else if (!strcmp(argv[i], "-x")) { if (i!=argc-1){ xshift = (float)atof(argv[i+1]); i++;}}
        else if (!strcmp(argv[i], "-y")) { if (i!=argc-1){ yshift = (float)atof(argv[i+1]); i++;}}
        else if (!strcmp(argv[i], "-z")) { if (i!=argc-1){ zshift = (float)atof(argv[i+1]); i++;}}
        //
        else if (!strcmp(argv[i], "-d")) { DebugMode = ON;}
        else if (!strcmp(argv[i], "-h")) { oarconv_help(stdout); exit(0);}
        else if (!strcmp(argv[i], "-v")) { fprintf(stdout, " Version is %s\n", OARCONV_VERSION); exit(0);}
        //
        else if (!strcmp(argv[i], "--unity")){ engine = JBXL_3D_ENGINE_UNITY;}  // for UNITY
        else if (!strcmp(argv[i], "--ue"))   { engine = JBXL_3D_ENGINE_UE;}     // for UE
        else if (!strcmp(argv[i], "--dae"))  { format = JBXL_3D_FORMAT_DAE;}    // DAEデータを出力
        else if (!strcmp(argv[i], "--obj"))  { format = JBXL_3D_FORMAT_OBJ;}    // OBJデータを出力
        else if (!strcmp(argv[i], "--fbx"))  { format = JBXL_3D_FORMAT_FBX;}    // FBXデータを出力
        else if (!strcmp(argv[i], "--stl"))  { format = JBXL_3D_FORMAT_STL_A;}  // STLデータを出力
        else if (!strcmp(argv[i], "--dg"))   { degeneracy = true;}
        else if (!strcmp(argv[i], "--joint")){ procJoints = true;}
        else if (!strcmp(argv[i], "--help")) { oarconv_help(stdout); exit(0);}
        else  {
            PRINT_MESG("%s : unknown option [%s]\n", argv[0], argv[i]);
            exit(1);
        }
    }

    if (inpdir.buf==NULL) {
        inpdir = make_Buffer_bystr("OAR");
    }
    float scale = (float)TRNT_DEFAULT_TEX_SCALE;
    if (tscale.buf!=NULL) {
        scale = (float)atof((char*)tscale.buf);
        if (scale<=0.0f) scale = (float)TRNT_DEFAULT_TEX_SCALE;
    }
    if (format!=JBXL_3D_FORMAT_DAE && format!=JBXL_3D_FORMAT_FBX) procJoints = false;

    init_rand();

    //////////////////////////////////////////////////////////////////////////////////////////////
    // main process
    OARTool oar;
    oar.SetEngine(engine);
    oar.SetDataFormat(format);
    oar.SetDegeneracy(degeneracy);
    oar.SetProcJoints(procJoints);
    oar.SetPathInfo((char*)inpdir.buf, (char*)outdir.buf, (char*)astdir.buf);
    oar.GetDataInfo();  // -f オプション（個別ファイル指定）があるので，成否は無視．
    oar.MakeOutputFolder();
    oar.SetTerrainShift(xshift, yshift, zshift);

    if (infile.buf!=NULL) {
        oar.objectsNum = 1;
        oar.GenerateSelectedDataFile((char*)infile.buf, useBrep, (char*)cmmnd.buf);
    }
    else {
        if (strtnum==0) {
            oar.ReadTerrainData();
            oar.SetTerrainTextureScale(scale);
            oar.GenerateTerrainDataFile();
            strtnum = 1;
        }
        if (stopnum!=0) {
            oar.GenerateObjectsDataFile(strtnum, stopnum, useBrep, (char*)cmmnd.buf);
        }
    }
    oar.free();
    
    del_tList(&AlphaChannelList);
    //
    free_Buffer(&inpdir);
    free_Buffer(&outdir);
    free_Buffer(&astdir);
    free_Buffer(&tscale);
    free_Buffer(&cmmnd);
    free_Buffer(&infile);

    return 0;
}

