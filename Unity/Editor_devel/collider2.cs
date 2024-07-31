using UnityEngine;
using UnityEditor;
using UnityEditor.AssetImporters;
using System.IO; 

public class AddCollidersPostProcessor : AssetPostprocessor
{
    static void OnPostprocessAllAssets(string[] importedAssets, string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths)
    {
        foreach (string assetPath in importedAssets) {
            if (assetPath.EndsWith(".gltf") || assetPath.EndsWith(".glb")) {
                // GameObject�����[�h����
                GameObject importedObject = AssetDatabase.LoadAssetAtPath<GameObject>(assetPath);
                if (importedObject != null) {
                    Debug.Log($"Loaded GameObject: {importedObject.name} from path: {assetPath}");
                    importedObject.AddComponent<Collider>();
                    // �V�[���ɃC���X�^���X���쐬���A�R���C�_�[��ǉ�
                    GameObject instance = (GameObject)PrefabUtility.InstantiatePrefab(importedObject);
                    if (instance != null) {
                        //AddColliderAndRigidbody(instance);

                        // �f�o�b�O�p�̃��b�Z�[�W
                        Debug.Log($"Processed and added colliders to instance of: {importedObject.name}");
                    }
                    else {
                        Debug.LogError($"Failed to instantiate prefab for: {importedObject.name}");
                    }
                }
                else {
                    Debug.LogError($"Failed to load GameObject from path: {assetPath}");
                }
            }
        }
    }

    static void AddColliderAndRigidbody(GameObject obj)
    {
        if (obj == null) {
            Debug.LogWarning("The provided GameObject is null.");
            return;
        }

        MeshFilter meshFilter = obj.GetComponent<MeshFilter>();
        if (meshFilter != null) {
            if (meshFilter.sharedMesh == null) {
                Debug.LogWarning($"MeshFilter on {obj.name} has no sharedMesh.");
            }
            else {
                bool meshColliderAdded = false;

                // MeshCollider �̒ǉ������݂�
                try {
                    MeshCollider meshCollider = obj.AddComponent<MeshCollider>();
                    meshCollider.sharedMesh = meshFilter.sharedMesh;
                    meshColliderAdded = true;

                    // �f�o�b�O�p�̃��b�Z�[�W
                    Debug.Log($"Added MeshCollider to {obj.name}");
                }
                catch (System.Exception e) {
                    Debug.LogError($"Failed to add MeshCollider to {obj.name}: {e.Message}");
                }

                /*
                // MeshCollider �̒ǉ��Ɏ��s�����ꍇ�A��փR���C�_�[��ǉ�
                if (!meshColliderAdded) {
                    BoxCollider boxCollider = obj.AddComponent<BoxCollider>();
                    Debug.LogWarning($"Added BoxCollider to {obj.name} as a fallback");
                }

                // Rigidbody �R���|�[�l���g�̒ǉ�
                if (obj.GetComponent<Rigidbody>() == null) {
                    Rigidbody rb = obj.AddComponent<Rigidbody>();
                    rb.isKinematic = true; // �I�u�W�F�N�g���Î~���Ă���ꍇ
                    Debug.Log($"Added Rigidbody to {obj.name}");
                }*/
            }
        }
        else {
            Debug.Log($"No MeshFilter found on {obj.name}");
        }
    }
}