#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
#include "ModelOBJ.h"


ModelOBJ::ModelOBJ()
{
    m_hasPositions = false;
    m_hasNormals = false;
    m_hasTextureCoords = false;
    m_hasTangents = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;
    m_numberOfMaterials = 0;
}

ModelOBJ::~ModelOBJ()
{
    destroy();
}

vector<ModelOBJ::GroupObject *> ModelOBJ::getObjects(void)
{
	return objects;
}

ModelOBJ::GroupObject *ModelOBJ::getObject(int index)
{
	for ( int i = 0; i < (int)objects.size(); i++ )
	{
		if ( objects[i]->index == index )
			return objects[i];
	}
}

void ModelOBJ::destroy()
{
	/* TODO:
	del go->center
	del unused vars
	*/

	for ( int i = 0; i < (int)objects.size(); i++ )
	{
	    for ( int m = 0; m < (int)objects[i]->materials.size(); m++ )
	        delete objects[i]->materials[m];

        for ( int v = 0; v < (int)objects[i]->vertices.size(); v++ )
	        delete objects[i]->vertices[v];

        delete objects[i];
	}

    m_hasPositions = false;
    m_hasTextureCoords = false;
    m_hasNormals = false;
    m_hasTangents = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;
    m_numberOfMaterials = 0;

    m_directoryPath.clear();

    m_materials.clear();
    m_vertexBuffer.clear();
    m_indexBuffer.clear();
    m_attributeBuffer.clear();

    m_vertexCoords.clear();
    m_textureCoords.clear();
    m_normals.clear();

    m_materialCache.clear();
    m_vertexCache.clear();

    objects.clear();

	/*for ( int m = 0; m < (int)materials.size(); m++ )
	{
		if ( materials[m]->ambientMap != NULL )
			delete materials[m]->ambientMap;

		if ( materials[m]->diffuseMap != NULL )
			delete materials[m]->diffuseMap;

		if ( materials[m]->specularMap != NULL )
			delete materials[m]->specularMap;

		if ( materials[m]->emissionMap != NULL )
			delete materials[m]->emissionMap;

		if ( materials[m]->shininessMap != NULL )
			delete materials[m]->shininessMap;

		if ( materials[m]->transparencyMap != NULL )
			delete materials[m]->transparencyMap;

		if ( materials[m]->bumpMap != NULL )
			delete materials[m]->bumpMap;

		delete materials[m];
	}

	for ( int m = 0; m < (int)vertices.size(); m++ )
		delete vertices[m];

	for ( int m = 0; m < (int)normals.size(); m++ )
		delete normals[m];

	for ( int m = 0; m < (int)UVWs.size(); m++ )
		delete UVWs[m];

	for ( int i = 0; i < (int)objects.size(); i++ )
	{
		GroupObject *object = objects[i];
		for ( int f = 0; f < (int)object->faces.size(); f++ )
		{
			delete object->faces[f]->vertices;
			delete object->faces[f]->normals;
			delete object->faces[f]->UVWs;
			delete object->faces[f];
		}
	}
	UVWs.clear();
	normals.clear();
	vertices.clear();
	objects.clear();
	materials.clear();*/
}

bool ModelOBJ::import(const char *pszFilename, bool rebuildNormals)
{
    FILE *pFile = fopen(pszFilename, "r");

    if (!pFile)
        return false;

    // Extract the directory the OBJ file is in from the file name.
    // This directory path will be used to load the OBJ's associated MTL file.

    m_directoryPath.clear();

    std::string filename = pszFilename;
    std::string::size_type offset = filename.find_last_of('\\');

    if (offset != std::string::npos)
    {
        m_directoryPath = filename.substr(0, ++offset);
    }
    else
    {
        offset = filename.find_last_of('/');

        if (offset != std::string::npos)
            m_directoryPath = filename.substr(0, ++offset);
    }

    // Import the OBJ file.

    importGeometryFirstPass(pFile);
    rewind(pFile);
    importGeometrySecondPass(pFile);
    fclose(pFile);

    // Perform post import tasks.
	buildObjects();
	findDistances();

    return true;
}

void ModelOBJ::reverseWinding()
{
    int swap = 0;

    // Reverse face winding.
    for (int i = 0; i < static_cast<int>(m_indexBuffer.size()); i += 3)
    {
        swap = m_indexBuffer[i + 1];
        m_indexBuffer[i + 1] = m_indexBuffer[i + 2];
        m_indexBuffer[i + 2] = swap;
    }

    float *pNormal = 0;
    float *pTangent = 0;

    // Invert normals and tangents.
    for (int i = 0; i < static_cast<int>(m_vertexBuffer.size()); ++i)
    {
        pNormal = m_vertexBuffer[i].normal;
        pNormal[0] = -pNormal[0];
        pNormal[1] = -pNormal[1];
        pNormal[2] = -pNormal[2];

        pTangent = m_vertexBuffer[i].tangent;
        pTangent[0] = -pTangent[0];
        pTangent[1] = -pTangent[1];
        pTangent[2] = -pTangent[2];
    }
}

void ModelOBJ::addTrianglePosTexCoordNormal(GroupObject *currentObject, int index, int material, int v0,
                                            int v1, int v2, int vt0, int vt1,
                                            int vt2, int vn0, int vn1, int vn2)
{
    Vertex vertex =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };
    m_attributeBuffer[index] = material;

    vertex.position[0] = m_vertexCoords[v0 * 3];
    vertex.position[1] = m_vertexCoords[v0 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v0 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt0 * 2];
    vertex.texCoord[1] = m_textureCoords[vt0 * 2 + 1];
    vertex.normal[0] = m_normals[vn0 * 3];
    vertex.normal[1] = m_normals[vn0 * 3 + 1];
    vertex.normal[2] = m_normals[vn0 * 3 + 2];
    m_indexBuffer[index * 3] = addVertex(currentObject, v0, &vertex);

    vertex.position[0] = m_vertexCoords[v1 * 3];
    vertex.position[1] = m_vertexCoords[v1 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v1 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt1 * 2];
    vertex.texCoord[1] = m_textureCoords[vt1 * 2 + 1];
    vertex.normal[0] = m_normals[vn1 * 3];
    vertex.normal[1] = m_normals[vn1 * 3 + 1];
    vertex.normal[2] = m_normals[vn1 * 3 + 2];
    m_indexBuffer[index * 3 + 1] = addVertex(currentObject, v1, &vertex);

    vertex.position[0] = m_vertexCoords[v2 * 3];
    vertex.position[1] = m_vertexCoords[v2 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v2 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt2 * 2];
    vertex.texCoord[1] = m_textureCoords[vt2 * 2 + 1];
    vertex.normal[0] = m_normals[vn2 * 3];
    vertex.normal[1] = m_normals[vn2 * 3 + 1];
    vertex.normal[2] = m_normals[vn2 * 3 + 2];
    m_indexBuffer[index * 3 + 2] = addVertex(currentObject, v2, &vertex);
}

int ModelOBJ::addVertex(GroupObject *currentGroup, int hash, Vertex *pVertex)
{
    int index = -1;
    std::map<int, std::vector<int> >::const_iterator iter = m_vertexCache.find(hash);
    if (iter == m_vertexCache.end())
    {
        // Vertex hash doesn't exist in the cache.
        index = static_cast<int>(m_vertexBuffer.size());
        m_vertexBuffer.push_back(*pVertex);
        m_vertexCache.insert(std::make_pair(hash, std::vector<int>(1, index)));

		m_vertexBufferPointers.push_back( pVertex );
    }
    else
    {
        // One or more vertices have been hashed to this entry in the cache.

        const std::vector<int> &vertices = iter->second;
        const Vertex *pCachedVertex = 0;
        bool found = false;

        for (std::vector<int>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
        {
            index = *i;
            pCachedVertex = &m_vertexBuffer[index];

            if (memcmp(pCachedVertex, pVertex, sizeof(Vertex)) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            index = static_cast<int>(m_vertexBuffer.size());
            m_vertexBuffer.push_back(*pVertex);
            m_vertexCache[hash].push_back(index);

			m_vertexBufferPointers.push_back( pVertex );
        }
    }
	Vector3 *v = new Vector3;
	v->x = pVertex->position[0];
	v->y = pVertex->position[1];
	v->z = pVertex->position[2];
	currentGroup->vertices.push_back( v );
    return index;
}

void ModelOBJ::findDistances()
{
	for ( int i = 0; i < (int)objects.size(); i++ )
	{
		GroupObject *obj = objects[i];
		int pos = obj->objectName.rfind("Object",0);
		if ( obj->objectName != "Board" && pos == -1 ) //White or black game piece
			findObjectPosition(obj);
	}
}

void ModelOBJ::findObjectPosition( GroupObject *obj )
{
	for ( int i = 0; i < (int)objects.size(); i++ )
	{
		GroupObject *square = objects[i];
		int pos = square->objectName.rfind("Object",0);
		if ( pos == 0 ) //found a square
		{
			Vector3 distance = obj->center.Distance(square->center);
			if ( obj->distance.x == 0 || obj->distance.x > distance.x )
			{
				obj->distance = distance;
				obj->square = square;
			}

		}

	}
}

void ModelOBJ::buildObjects()
{
	Material *m = 0;
	int materialId = -1;
	int numMaterials = 0;

	//count number of materials
	for (int i = 0; i < static_cast<int>(m_attributeBuffer.size()); i++)
    {
        if (m_attributeBuffer[i] != materialId)
        {
            materialId = m_attributeBuffer[i];
            ++numMaterials;
        }
	}

	// Allocate memory for the materials and reset counters.
    m_numberOfObjectMaterials = numMaterials;
    m_materials.resize(m_numberOfObjectMaterials);
    numMaterials = 0;
    materialId = -1;

	// Build the meshes. One mesh for each unique material.
    for (int i = 0; i < static_cast<int>(m_attributeBuffer.size()); i++)
    {
        if (m_attributeBuffer[i] != materialId)
        {
            materialId = m_attributeBuffer[i];
            m = m_ObjectMaterials[materialId];
            m->startIndex = i * 3;
			m->triangleCount = 0;
            ++m->triangleCount;
        }
        else
        {
            ++m->triangleCount;
        }
    }

    // Sort the meshes based on its material alpha. Fully opaque meshes
    // towards the front and fully transparent towards the back.
    //std::sort(m_meshes.begin(), m_meshes.end(), MeshCompFunc);
}

void ModelOBJ::importGeometryFirstPass(FILE *pFile)
{
    m_hasTextureCoords = false;
    m_hasNormals = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;

    int v = 0;
    int vt = 0;
    int vn = 0;
    char buffer[256] = {0};
    std::string name;

    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'f':   // v, v//vn, v/vt, v/vt/vn.
            fscanf(pFile, "%s", buffer);

            if (strstr(buffer, "//")) // v//vn
            {
                sscanf(buffer, "%d//%d", &v, &vn);
                fscanf(pFile, "%d//%d", &v, &vn);
                fscanf(pFile, "%d//%d", &v, &vn);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d//%d", &v, &vn) > 0)
                    ++m_numberOfTriangles;
            }
            else if (sscanf(buffer, "%d/%d/%d", &v, &vt, &vn) == 3) // v/vt/vn
            {
                fscanf(pFile, "%d/%d/%d", &v, &vt, &vn);
                fscanf(pFile, "%d/%d/%d", &v, &vt, &vn);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d/%d/%d", &v, &vt, &vn) > 0)
                    ++m_numberOfTriangles;
            }
            else if (sscanf(buffer, "%d/%d", &v, &vt) == 2) // v/vt
            {
                fscanf(pFile, "%d/%d", &v, &vt);
                fscanf(pFile, "%d/%d", &v, &vt);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d/%d", &v, &vt) > 0)
                    ++m_numberOfTriangles;
            }
            else // v
            {
                fscanf(pFile, "%d", &v);
                fscanf(pFile, "%d", &v);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d", &v) > 0)
                    ++m_numberOfTriangles;
            }
            break;

        case 'm':   // mtllib
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            name = m_directoryPath;
            name += buffer;
            importMaterials(name.c_str());
            break;

        case 'v':   // v, vt, or vn
            switch (buffer[1])
            {
            case '\0':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfVertexCoords;
                break;

            case 'n':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfNormals;
                break;

            case 't':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfTextureCoords;

            default:
                break;
            }
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    m_hasPositions = m_numberOfVertexCoords > 0;
    m_hasNormals = m_numberOfNormals > 0;
    m_hasTextureCoords = m_numberOfTextureCoords > 0;

    // Allocate memory for the OBJ model data.
    m_vertexCoords.resize(m_numberOfVertexCoords * 3);
    m_textureCoords.resize(m_numberOfTextureCoords * 2);
    m_normals.resize(m_numberOfNormals * 3);
    m_indexBuffer.resize(m_numberOfTriangles * 3);
    m_attributeBuffer.resize(m_numberOfTriangles);

    // Define a default material if no materials were loaded.
    if (m_numberOfMaterials == 0)
    {
        Material defaultMaterial =
        {
            0.2f, 0.2f, 0.2f, 1.0f,
            0.8f, 0.8f, 0.8f, 1.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            0.0f,
            1.0f,
            std::string("default"),
            std::string(),
            std::string()
        };

        m_materials.push_back(defaultMaterial);
        m_materialCache[defaultMaterial.name] = 0;
    }
}

void ModelOBJ::importGeometrySecondPass(FILE *pFile)
{
    int v[3] = {0};
    int vt[3] = {0};
    int vn[3] = {0};
    int numVertices = 0;
    int numTexCoords = 0;
    int numNormals = 0;
    int numTriangles = 0;
    int activeMaterial = 0;
    char buffer[256] = {0};
    std::string name;
    std::map<std::string, int>::const_iterator iter;

	GroupObject *defaultObject = new GroupObject;
	GroupObject *currentGroup = defaultObject;
	Material *currentMaterial = NULL;

	int groupIndex = 0;
	string objectName;
	char oName[50];
	int materialId = 0;

    while (fscanf(pFile, "%s", buffer) != EOF)
    {
		if ( buffer[0] == 'f' ) // v, v//vn, v/vt, or v/vt/vn(only one accounted for ATM).
        {
            v[0]  = v[1]  = v[2]  = 0;
            vt[0] = vt[1] = vt[2] = 0;
            vn[0] = vn[1] = vn[2] = 0;

            fscanf(pFile, "%s", buffer);


            if (sscanf(buffer, "%d/%d/%d", &v[0], &vt[0], &vn[0]) == 3) // v/vt/vn
            {
                fscanf(pFile, "%d/%d/%d", &v[1], &vt[1], &vn[1]);
                fscanf(pFile, "%d/%d/%d", &v[2], &vt[2], &vn[2]);

                v[0] = (v[0] < 0) ? v[0] + numVertices - 1 : v[0] - 1;
                v[1] = (v[1] < 0) ? v[1] + numVertices - 1 : v[1] - 1;
                v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                vt[0] = (vt[0] < 0) ? vt[0] + numTexCoords - 1 : vt[0] - 1;
                vt[1] = (vt[1] < 0) ? vt[1] + numTexCoords - 1 : vt[1] - 1;
                vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;

                vn[0] = (vn[0] < 0) ? vn[0] + numNormals - 1 : vn[0] - 1;
                vn[1] = (vn[1] < 0) ? vn[1] + numNormals - 1 : vn[1] - 1;
                vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                addTrianglePosTexCoordNormal(currentGroup, numTriangles++, currentMaterial->id,
                    v[0], v[1], v[2], vt[0], vt[1], vt[2], vn[0], vn[1], vn[2]);

                v[1] = v[2];
                vt[1] = vt[2];
                vn[1] = vn[2];

                while (fscanf(pFile, "%d/%d/%d", &v[2], &vt[2], &vn[2]) > 0)
                {
                    v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;
                    vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;
                    vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                    addTrianglePosTexCoordNormal(currentGroup, numTriangles++, currentMaterial->id,
                        v[0], v[1], v[2], vt[0], vt[1], vt[2], vn[0], vn[1], vn[2]);

                    v[1] = v[2];
                    vt[1] = vt[2];
                    vn[1] = vn[2];
                }
            }
		} else if (buffer[0] == 'u' ) { // usemtl
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            name = buffer;
            iter = m_materialCache.find(buffer);
            activeMaterial = (iter == m_materialCache.end()) ? 0 : iter->second;

			for ( int i = 0; i < (int)m_materials.size(); i++ )
			{
				if ( name == m_materials[i].name )
				{
					Material *m = new Material;
					*m = m_materials[i];
					m->id = materialId;
					m->startIndex = -1;
					m->triangleCount = 0;
					materialId++;
					currentMaterial = m;
					currentGroup->materials.push_back(currentMaterial);
					m_ObjectMaterials.push_back( currentMaterial );
					break;
				}
			}
		} else if (buffer[0] == 'g' ) { //group
			if ( currentGroup->vertices.size() > 0 ) // && pos == 0
				findObjectCenter( currentGroup );

			fscanf (pFile, "%s", oName);
			objectName = oName;
			if ( objectName == "default" )
				currentGroup = defaultObject;
			else {
				GroupObject *object = new GroupObject;
				object->objectName = objectName;
				object->index = ++groupIndex;
				currentGroup = object;
				objects.push_back(object);
			}
		} else if (buffer[0] == 'v' ) { // v, vn, or vt.
            switch (buffer[1])
            {
            case '\0': // v
                fscanf(pFile, "%f %f %f",
                    &m_vertexCoords[3 * numVertices],
                    &m_vertexCoords[3 * numVertices + 1],
                    &m_vertexCoords[3 * numVertices + 2]);
                ++numVertices;
                break;

            case 'n': // vn
                fscanf(pFile, "%f %f %f",
                    &m_normals[3 * numNormals],
                    &m_normals[3 * numNormals + 1],
                    &m_normals[3 * numNormals + 2]);
                ++numNormals;
                break;

            case 't': // vt
                fscanf(pFile, "%f %f",
                    &m_textureCoords[2 * numTexCoords],
                    &m_textureCoords[2 * numTexCoords + 1]);
                ++numTexCoords;
                break;

            default:
                break;
            }
		} else {
            fgets(buffer, sizeof(buffer), pFile);
        }
    }
    if ( currentGroup->vertices.size() > 0 ) //For the last object loaded
        findObjectCenter( currentGroup );
}

void ModelOBJ::findObjectCenter( GroupObject *currentObject )
{
	vector<Vector3 *> vertices = currentObject->vertices;
	Vector3 *sumVector = new Vector3;

	for ( int n = 0; n < (int)currentObject->vertices.size(); n++ )
	{
		Vector3 *vertex = currentObject->vertices[n];
		sumVector->x += vertex->x;
		sumVector->y += vertex->y;
		sumVector->z += vertex->z;
	}
	currentObject->center += (*sumVector);
	currentObject->center /= vertices.size();
	delete sumVector;
}

bool ModelOBJ::importMaterials(const char *pszFilename)
{
    FILE *pFile = fopen(pszFilename, "r");

    if (!pFile)
        return false;

    Material *pMaterial = 0;
    int illum = 0;
    int numMaterials = 0;
    char buffer[256] = {0};

    // Count the number of materials in the MTL file.
    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'n': // newmtl
            ++numMaterials;
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    rewind(pFile);

    m_numberOfMaterials = numMaterials;
    numMaterials = 0;
    m_materials.resize(m_numberOfMaterials);

    // Load the materials in the MTL file.
    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'N': // Ns
            fscanf(pFile, "%f", &pMaterial->shininess);

            // Wavefront .MTL file shininess is from [0,1000].
            // Scale back to a generic [0,1] range.
            pMaterial->shininess /= 1000.0f;
            break;

        case 'K': // Ka, Kd, or Ks
            switch (buffer[1])
            {
            case 'a': // Ka
                fscanf(pFile, "%f %f %f",
                    &pMaterial->ambient[0],
                    &pMaterial->ambient[1],
                    &pMaterial->ambient[2]);
                pMaterial->ambient[3] = 1.0f;
                break;

            case 'd': // Kd
                fscanf(pFile, "%f %f %f",
                    &pMaterial->diffuse[0],
                    &pMaterial->diffuse[1],
                    &pMaterial->diffuse[2]);
                pMaterial->diffuse[3] = 1.0f;
                break;

            case 's': // Ks
                fscanf(pFile, "%f %f %f",
                    &pMaterial->specular[0],
                    &pMaterial->specular[1],
                    &pMaterial->specular[2]);
                pMaterial->specular[3] = 1.0f;
                break;

            default:
                fgets(buffer, sizeof(buffer), pFile);
                break;
            }
            break;

        case 'T': // Tr
            switch (buffer[1])
            {
            case 'r': // Tr
                fscanf(pFile, "%f", &pMaterial->alpha);
                pMaterial->alpha = 1.0f - pMaterial->alpha;
                break;

            default:
                fgets(buffer, sizeof(buffer), pFile);
                break;
            }
            break;

        case 'd':
            fscanf(pFile, "%f", &pMaterial->alpha);
            break;

        case 'i': // illum
            fscanf(pFile, "%d", &illum);

            if (illum == 1)
            {
                pMaterial->specular[0] = 0.0f;
                pMaterial->specular[1] = 0.0f;
                pMaterial->specular[2] = 0.0f;
                pMaterial->specular[3] = 1.0f;
            }
            break;

        case 'm': // map_Kd, map_bump
            if (strstr(buffer, "map_Kd") != 0)
            {
                fgets(buffer, sizeof(buffer), pFile);
                sscanf(buffer, "%s %s", buffer, buffer);
                pMaterial->colorMapFilename = buffer;
            }
            else if (strstr(buffer, "map_bump") != 0)
            {
                fgets(buffer, sizeof(buffer), pFile);
                sscanf(buffer, "%s %s", buffer, buffer);
                pMaterial->bumpMapFilename = buffer;
            }
            else
            {
                fgets(buffer, sizeof(buffer), pFile);
            }
            break;

        case 'n': // newmtl
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);

            pMaterial = &m_materials[numMaterials];
            pMaterial->ambient[0] = 0.2f;
            pMaterial->ambient[1] = 0.2f;
            pMaterial->ambient[2] = 0.2f;
            pMaterial->ambient[3] = 1.0f;
            pMaterial->diffuse[0] = 0.8f;
            pMaterial->diffuse[1] = 0.8f;
            pMaterial->diffuse[2] = 0.8f;
            pMaterial->diffuse[3] = 1.0f;
            pMaterial->specular[0] = 0.0f;
            pMaterial->specular[1] = 0.0f;
            pMaterial->specular[2] = 0.0f;
            pMaterial->specular[3] = 1.0f;
            pMaterial->shininess = 0.0f;
            pMaterial->alpha = 1.0f;
            pMaterial->name = buffer;
            pMaterial->colorMapFilename.clear();
            pMaterial->bumpMapFilename.clear();

			pMaterial->id = numMaterials;
            m_materialCache[pMaterial->name] = numMaterials;
            ++numMaterials;
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    fclose(pFile);
    return true;
}

