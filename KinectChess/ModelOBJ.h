#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include "Transform.h"

using namespace std;

class ModelOBJ
{
public:
	struct Vertex
    {
        float position[3];
        float texCoord[2];
        float normal[3];
        float tangent[4];
        float bitangent[3];
    };

    struct Material
    {
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float shininess;        // [0 = min shininess, 1 = max shininess]
        float alpha;            // [0 = fully transparent, 1 = fully opaque]

        std::string name;
        std::string colorMapFilename;
        std::string bumpMapFilename;

		int	id;
		int startIndex;
		int triangleCount;
    };

	//A chess piece or square
	struct GroupObject
	{
		std::vector<Material *> materials;
		std::vector<int>		materialIds;
		std::string				objectName;
		std::string				groupName;
		int						index;
		std::vector<Vector3 *>	vertices;
		Vector3					center;
		Vector3					distance;
		GroupObject				*square;
		Transform				position;
	};

    ModelOBJ();
    ~ModelOBJ();

	vector<GroupObject *>	getObjects(void);
	GroupObject *getObject(int index);

    void destroy();
    bool import(const char *pszFilename, bool rebuildNormals = false);
    void reverseWinding();

    // Getter methods
    void getCenter(float &x, float &y, float &z) const;

    const int *getIndexBuffer() const;
    int getIndexSize() const;

    //const Material &getMaterial(int i) const; NWB
	const Material &getMaterial(int i) const;

    int getNumberOfIndices() const;
    int getNumberOfMaterials() const;
    int getNumberOfMeshes() const;
    int getNumberOfTriangles() const;
    int getNumberOfVertices() const;

    const std::string &getPath() const;

    const Vertex &getVertex(int i) const;
    const Vertex *getVertexBuffer() const;
    int getVertexSize() const;

    bool hasNormals() const;
    bool hasPositions() const;
    bool hasTangents() const;
    bool hasTextureCoords() const;



private:
    void findObjectCenter( GroupObject *currentObject );
    void addTrianglePosTexCoordNormal(GroupObject *currentObject, int index, int material,
        int v0, int v1, int v2,
        int vt0, int vt1, int vt2,
        int vn0, int vn1, int vn2);
    int addVertex(GroupObject *currentGroup, int hash, Vertex *pVertex);
	void findObjectPosition( GroupObject *obj );
	void findDistances();
	void buildObjects();
    void importGeometryFirstPass(FILE *pFile);
    void importGeometrySecondPass(FILE *pFile);
    bool importMaterials(const char *pszFilename);

    bool m_hasPositions;
    bool m_hasTextureCoords;
    bool m_hasNormals;
    bool m_hasTangents;

    int m_numberOfVertexCoords;
    int m_numberOfTextureCoords;
    int m_numberOfNormals;
    int m_numberOfTriangles;
    int m_numberOfMaterials;
	int m_numberOfObjects;
	int m_numberOfObjectMaterials;

    std::string m_directoryPath;

    std::vector<Material> m_materials;
    std::vector<Vertex> m_vertexBuffer;
	std::vector<Vertex *> m_vertexBufferPointers;
    std::vector<int> m_indexBuffer;
    std::vector<int> m_attributeBuffer;
    std::vector<float> m_vertexCoords;
    std::vector<float> m_textureCoords;
    std::vector<float> m_normals;


	std::vector<int> m_ObjectMaterialIds;
	std::vector<Material *> m_ObjectMaterials;

    std::map<std::string, int> m_materialCache;
    std::map<int, std::vector<int> > m_vertexCache;
	std::map<int, std::vector<int> > m_MaterialVertices;

	vector<GroupObject *> objects;
};

//-----------------------------------------------------------------------------

inline const int *ModelOBJ::getIndexBuffer() const
{ return &m_indexBuffer[0]; }

inline int ModelOBJ::getIndexSize() const
{ return static_cast<int>(sizeof(int)); }

inline const ModelOBJ::Material &ModelOBJ::getMaterial(int i) const
{ return m_materials[i]; }

inline int ModelOBJ::getNumberOfIndices() const
{ return m_numberOfTriangles * 3; }

inline int ModelOBJ::getNumberOfMaterials() const
{ return m_numberOfMaterials; }

inline int ModelOBJ::getNumberOfTriangles() const
{ return m_numberOfTriangles; }

inline int ModelOBJ::getNumberOfVertices() const
{ return static_cast<int>(m_vertexBuffer.size()); }

inline const std::string &ModelOBJ::getPath() const
{ return m_directoryPath; }

inline const ModelOBJ::Vertex &ModelOBJ::getVertex(int i) const
{ return m_vertexBuffer[i]; }

inline const ModelOBJ::Vertex *ModelOBJ::getVertexBuffer() const
{ return &m_vertexBuffer[0]; }

inline int ModelOBJ::getVertexSize() const
{ return static_cast<int>(sizeof(Vertex)); }

inline bool ModelOBJ::hasNormals() const
{ return m_hasNormals; }

inline bool ModelOBJ::hasPositions() const
{ return m_hasPositions; }

inline bool ModelOBJ::hasTangents() const
{ return m_hasTangents; }

inline bool ModelOBJ::hasTextureCoords() const
{ return m_hasTextureCoords; }

#endif
