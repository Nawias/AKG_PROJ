// Anim8orExport.h

#if !defined(__ANIM8OR_EXPORT_H)
#define __ANIM8OR_EXPORT_H 1

#pragma warning(disable:4244)   // No warnings on precision truncation
#pragma warning(disable:4305)   // No warnings on precision truncation, MSVC 6.0

typedef float color[4];

typedef struct Anim8orMaterial {
	color ambient;
	color diffuse;
	color specular;
	color emissive;
	float Ka, Kd, Ks, Ke;
	float Roughness, Brilliance;
	const char *ambientTexture;
	const char *diffuseTexture;
	const char *specularTexture;
	const char *emissiveTexture;
	const char *transparencyTexture;
	const char *bumpmapTexture;
	const char* environmentTexture;   // Dodane dla kompatybilnoœci z Anim8or 1.00
	const char* texture;              // Dodane dla kompatybilnoœci z Anim8or 1.00
} Anim8orMaterial;

typedef struct Anim8orMesh {
    const char *name;
    int nVertices;
    int nIndices;
    int *indices;
    unsigned char *matindices;
    float *coordinates;
    float *normals;
    float *texcoords;
    Anim8orMaterial *materials;
} Anim8orMesh;

typedef struct Anim8orObject {
    const char *name;
    int nMeshes;
    Anim8orMesh *meshes[];
} Anim8orObject;

#endif // __ANIM8OR_EXPORT_H