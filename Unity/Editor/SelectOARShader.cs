//
// SelectOARShader for oarconv by Fumi.Iseki 2015-2024 (C) v1.7.1
//
// see also https://github.com/MTSGJ/oarconv
//
//

using UnityEngine;
using UnityEditor;
using System.IO;
using System;


public sealed class SelectOARShader : AssetPostprocessor
{
    private string GeneralShader;
    private string TransShader;             // Alpha Blending
    private string TransCutShader;          // Alpha Cutoff
    private string TransCutSoftShader;      // Alpha Blending
    private string TransSpecShader;         // Alpha Blending + Specular
    private string TransCutSpecShader;      // Alpha Cutoff + Specular
    private string SpecularShader;
    private string BrightShader;
    private string GlowShader;
    private string TreeShader;

    private const string MaterialFolder = "Materials";
    private const string PhantomFolder  = "Phantoms";

    private float colorRed      = 0.0f;
    private float colorGreen    = 0.0f;
    private float colorBlue     = 0.0f;
    private float transparent   = 1.0f;
    private float cutoff        = 0.0f;
    private float shininess     = 0.0f;
    private float glow          = 0.0f;
    private float bright        = 0.0f;
    private float light         = 0.0f;
    private int   alphaMode     = 0;     // 0: NONE, 1: BLENDING, 2: MASKING, 3: EMISSIVE
    private bool  hasAlpha      = false;
//  private float shiftU        = 0.0f;
//  private float shiftV        = 0.0f;
//  private float scaleU        = 1.0f;
//  private float scaleV        = 1.0f;
//  private float rotate        = 0.0f;
    private char  kind          = 'O';        // Object

    private const string HDRP_Shader   = "HDRP/Lit";
    private const string URP_Shader    = "Universal Render Pipeline/Simple Lit";
    private const string BINP_Shader   = "Legacy Shaders/Diffuse";
    private const string NONE_Shader   = "NONE";

    private bool createdMaterialFolder = false;


    public SelectOARShader() : base()
    {
        InitShader();
    }



    void OnPreprocessModel()
    {
        //Debug.Log("Unity Version = " + UnityEngine.Application.unityVersion);
        string currentFolder = Path.GetDirectoryName(assetPath);
        if (!createdMaterialFolder && !AssetDatabase.IsValidFolder(currentFolder + "\\" + MaterialFolder)) {
            AssetDatabase.CreateFolder(currentFolder, MaterialFolder);
            createdMaterialFolder = true;
        }
        ModelImporter modelImporter = assetImporter as ModelImporter;

        modelImporter.materialImportMode = ModelImporterMaterialImportMode.ImportStandard;
        modelImporter.materialLocation = ModelImporterMaterialLocation.External;
        //modelImporter.materialLocation = ModelImporterMaterialLocation.External;

        /*
        if (!modelImporter.importSettingsMissing) {
        }
        modelImporter.animationType = ModelImporterAnimationType.None;
        */

        modelImporter.addCollider = true;
        if (currentFolder.EndsWith("/" + PhantomFolder) || currentFolder.EndsWith("\\" + PhantomFolder)) {
            modelImporter.addCollider = false;
        }
    }


    void InitShader()
    {
        string _generalShader = HDRP_Shader;
        Shader shdr = Shader.Find(_generalShader);
        if (shdr == null) { 
            _generalShader = URP_Shader;
            shdr = Shader.Find(_generalShader);
        }
        if (shdr == null) {
            _generalShader = BINP_Shader;
            shdr = Shader.Find(_generalShader);
        }
        if (shdr == null) {
            GeneralShader = NONE_Shader;
        }
        else {
            GeneralShader = _generalShader;
        }

        //
        if (GeneralShader == HDRP_Shader) {
            TransShader         = "Unlit/Transparent";
            TreeShader          = "Unlit/Transparent";
            SpecularShader      = "HDRP/Lit";
        }
        else if (GeneralShader == URP_Shader) {
            TransShader         = "Unlit/Transparent";
            TreeShader          = "Unlit/Transparent";
            SpecularShader      = "Universal Render Pipeline/Simple Lit";
        }
        else if (GeneralShader == BINP_Shader) {
            TransShader         = "Legacy Shaders/Transparent/Diffuse";                  // Alpha Blending
            TransCutShader      = "Legacy Shaders/Transparent/Cutout/Diffuse";           // Alpha Cutoff
            TransCutSoftShader  = "Legacy Shaders/Transparent/Cutout/Soft Edge Unlit";   // Alpha Blending
            TransSpecShader     = "Legacy Shaders/Transparent/Specular";                 // Alpha Blending + Specular
            TransCutSpecShader  = "Legacy Shaders/Transparent/Cutout/Specular";          // Alpha Cutoff + Specular
            BrightShader        = "Legacy Shaders/Self-Illumin/Specular";
            SpecularShader      = "Standard";
            GlowShader          = "Standard";
            TreeShader          = "Legacy Shaders/Transparent/Cutout/Soft Edge Unlit";
        }

        Debug.Log("General Shadr = " + GeneralShader);
        return;
    }



    Material OnAssignMaterialModel(Material material, Renderer renderer)
    {
        //Debug.Log("Renderer = " + renderer.name);
        string materialName = material.name;
        //UnityEngine.Debug.Log("Material Name = " + materialName);
        string currentFolder = Path.GetDirectoryName(assetPath);
        string materialPath = string.Format("{0}\\{1}\\{2}.mat", currentFolder, MaterialFolder, materialName);

        Material mt = AssetDatabase.LoadAssetAtPath<Material>(materialPath);
        if (mt != null) {
            material = mt;
            return material;
        }
        
        if (GeneralShader == HDRP_Shader) {
            SetMaterialShader_HDRP(material);
        }
        else if (GeneralShader == HDRP_Shader) {
            SetMaterialShader_URP(material);
        }
        else { 
            SetMaterialShader_BRP(material);
        }        
        //
        AssetDatabase.CreateAsset(material, materialPath);

        return material;
    }



    void SetMaterialShader_HDRP(Material material)
    {
        string materialName = material.name;
        getParamsFromMaterialName(materialName);

        // Shader の選択 開始
        material.shader = Shader.Find(GeneralShader);
        if (material.HasProperty("_Smoothness")) material.SetFloat("_Smoothness", 0.0f); // 反射を止める

        if (kind == 'T' || kind == 'G') {   // Tree or Grass
            material.shader = Shader.Find(TreeShader);
        }
        // Alpha Channell
        else if (hasAlpha) {
            material.shader = Shader.Find(TransShader);
            if (cutoff>0.0f) {
                if (material.HasProperty("_AlphaCutoffEnable")) material.SetFloat("_AlphaCutoffEnable", 1.0f);
                if (material.HasProperty("_AlphaCutoff")) material.SetFloat("_AlphaCutoff", cutoff);
            }
        }
        // Shininess
        else if (shininess > 0.01f) {
            material.shader = Shader.Find(SpecularShader);
        }
        // Shader の選択終わり

        // Color
        if (material.HasProperty("_BaseColor")) {
            material.SetColor("_BaseColor", new Color(colorRed, colorGreen, colorBlue, transparent));
        }
        else {
            material.SetColor("_Color", new Color(colorRed, colorGreen, colorBlue, transparent));
        }

        // Shininess
        if (shininess > 0.01f) {
            if (material.HasProperty("_Smoothness")) material.SetFloat("_Smoothness", 0.5f);
            if (material.HasProperty("_Metallic")) material.SetFloat("_Metallic", shininess * 3.0f);
        }

        /*
        if (material.HasProperty("_EmissiveColor")) {
            //material.EnableKeyword("_EmissiveIntensity");
            Color col = material.GetColor("_Color");
            float fac = col.maxColorComponent;
            if (fac > 0.01f) {
                if (glow > 0.99f) glow = 0.99f;
                col = col * (glow / fac);
            }
            material.SetColor("_EmissiveColor", col);
            material.SetFloat("_EmissiveIntensity", 1.0f);
        }*/

        return;
    }



    void SetMaterialShader_URP(Material material)
    {
        string materialName = material.name;
        getParamsFromMaterialName(materialName);

        // Shader の選択 開始
        material.shader = Shader.Find(GeneralShader);

        if (kind == 'T' || kind == 'G') {   // Tree or Grass
            material.shader = Shader.Find(TreeShader);
        }

        // Alpha Channell
        else if (hasAlpha) {
            material.shader = Shader.Find(TransShader);
            if (cutoff > 0.0f) {
                if (material.HasProperty("_AlphaCutoffEnable")) material.SetFloat("_AlphaCutoffEnable", 1.0f);
                if (material.HasProperty("_AlphaCutoff")) material.SetFloat("_AlphaCutoff", cutoff);
            }
        }
        // Shininess
        else if (shininess > 0.01f) {
            material.shader = Shader.Find(SpecularShader);
        }
        // Shader の選択終わり

        //
        // Color
        if (material.HasProperty("_BaseColor")) {
            material.SetColor("_BaseColor", new Color(colorRed, colorGreen, colorBlue, transparent));
        }
        else {
            material.SetColor("_Color", new Color(colorRed, colorGreen, colorBlue, transparent));
        }

        // Shininess
        if (shininess > 0.01f) {
            if (material.HasProperty("_Metallic")) material.SetFloat("_Metallic", shininess * 3.0f);
        }
        
        return;
    }



    void SetMaterialShader_BRP(Material material)
    {
        string materialName = material.name;
        getParamsFromMaterialName(materialName);

        material.shader = Shader.Find(GeneralShader);

        if (kind == 'T' || kind == 'G') {   // Tree or Grass
            material.shader = Shader.Find(TreeShader);
        }
        //
        //else if (transparent < 0.99f) {
        else if (hasAlpha) {
            if (shininess > 0.01f) {
                if (cutoff > 0.01f) {   // Alpha Cutoff
                    material.shader = Shader.Find(TransCutSpecShader);
                    if (material.HasProperty("_Cutoff")) material.SetFloat("_Cutoff", cutoff);
                }
                else {   // Alpha Blending
                    //if (transparent >= 0.90f) {
                    if (!hasAlpha) {
                        material.shader = Shader.Find(TransCutSpecShader);
                        if (material.HasProperty("_Cutoff")) material.SetFloat("_Cutoff", 0.2f);
                    }
                    else {
                        material.shader = Shader.Find(TransSpecShader);
                    }
                }
                if (material.HasProperty("_Shininess")) material.SetFloat("_Shininess", 1.0f - shininess);
            }
            //
            else {
                if (cutoff > 0.01f) {   // Alpha Cutoff
                    material.shader = Shader.Find(TransCutShader);
                    if (material.HasProperty("_Cutoff")) material.SetFloat("_Cutoff", cutoff);
                }
                else {   // Alpha Blending
                    //if (transparent >= 0.90f) {
                    if (!hasAlpha) {
                        material.shader = Shader.Find(TransCutSoftShader);
                        if (material.HasProperty("_Cutoff")) material.SetFloat("_Cutoff", 0.9f);
                    }
                    else {
                        material.shader = Shader.Find(TransShader);
                    }
                }
            }
        }
        //
        else if (transparent < 0.9f) {
            material.shader = Shader.Find(TransShader);
            if (material.HasProperty("_Cutoff")) material.SetFloat("_Cutoff", cutoff);
        }
        //
        else if (glow > 0.02f) {
            material.EnableKeyword("_EMISSION");
            material.shader = Shader.Find(GlowShader);
            if (material.HasProperty("_EmissionColor")) {
                Color col = material.GetColor("_Color");
                float fac = col.maxColorComponent;
                if (fac > 0.01f) {
                    if (glow > 0.99f) glow = 0.99f;
                    col = col * (glow / fac);
                }
                material.SetColor("_EmissionColor", col);
            }
            if (material.HasProperty("_Glossiness")) material.SetFloat("_Glossiness", shininess);
        }
        //
        else if (bright > 0.01f) {
            material.shader = Shader.Find(BrightShader);
            if (material.HasProperty("_Shininess")) material.SetFloat("_Shininess", 1.0f - shininess);
        }
        //
        else if (shininess > 0.01f) {
            material.shader = Shader.Find(SpecularShader);
            if (material.HasProperty("_Metallic")) material.SetFloat("_Metallic", shininess / 2.0f);
            if (material.HasProperty("_Glossiness")) material.SetFloat("_Glossiness", 0.5f + shininess / 2.0f);
        }
        
        // Color
        if (material.HasProperty("_BaseColor")) {
            material.SetColor("_BaseColor", new Color(colorRed, colorGreen, colorBlue, transparent));
        }
        else {
            material.SetColor("_Color", new Color(colorRed, colorGreen, colorBlue, transparent));
        }
        
        return ;
    }



    private void getParamsFromMaterialName(string name)
    {
        int atr_len = 32;    // MTRL_ATTR_LEN /3*4 (36/3*4 = 32)
        //
        if (name.Length >= atr_len) {
            string sub = name.Substring(name.Length - atr_len, atr_len);
            string enc = sub.Replace('-', '/');
            enc = enc.Replace('$', '/');           // for old version
            //
            if (enc.Length == atr_len) {
                try {
                    byte[] dec = Convert.FromBase64String(enc);    // MTRL_ATTR_LEN (32/4*3 = 24)
                    colorRed    = 1.0f - (float)dec[0] / 255.0f;
                    colorGreen  = 1.0f - (float)dec[1] / 255.0f;
                    colorBlue   = 1.0f - (float)dec[2] / 255.0f;
                    transparent = 1.0f - (float)dec[3] / 255.0f;
                    cutoff      = (float)dec[4] / 255.0f;
                    shininess   = (float)dec[5] / 255.0f;
                    glow        = (float)dec[6] / 255.0f;
                    bright      = (float)dec[7] / 255.0f;
                    light       = (float)dec[8] / 255.0f;
                    alphaMode   = (int)dec[9]/10;           // 0: NONE, 1: BLENDING, 2: MASKING, 3: EMISSIVE
                    int has_alp = (int)dec[9]%10;           // 0: No, 1: YES
                    /*
                    short[] tmp = new short[1];
                    Buffer.BlockCopy(dec, 13, tmp, 0, 2);
                    shiftU      = (float)tmp[0] / 2000.0f;
                    Buffer.BlockCopy(dec, 15, tmp, 0, 2);
                    shiftV      = (float)tmp[0] / 2000.0f;
                    Buffer.BlockCopy(dec, 17, tmp, 0, 2);
                    scaleU      = (float)tmp[0] / 100.0f;
                    Buffer.BlockCopy(dec, 19, tmp, 0, 2);
                    scaleV      = (float)tmp[0] / 100.0f;
                    Buffer.BlockCopy(dec, 21, tmp, 0, 2);
                    rotate      = (float)tmp[0] / 2000.0f;
                    */
                    kind = (char)dec[23];
                    
                    if (cutoff > 0.9f) cutoff = 0.9f;
                    if (has_alp == 1) hasAlpha = true;
                    else              hasAlpha = false;
                }
                catch {
                    UnityEngine.Debug.Log("SelectOARShader: Base64 Decode Error = " + enc);
                }
            }
        }
    }
    
}
