#include "TRDrawableMesh.h"

#include <map>
#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "TRTexture2D.h"
#include "TRShadingPipeline.h"

namespace TinyRenderer
{
	TRDrawableSubMesh::TRDrawableSubMesh(const TRDrawableSubMesh& mesh)
		: m_vertices(mesh.m_vertices), m_indices(mesh.m_indices), m_drawingMaterial(mesh.m_drawingMaterial) {}

	TRDrawableSubMesh& TRDrawableSubMesh::operator=(const TRDrawableSubMesh& mesh)
	{
		if (&mesh == this)
			return *this;
		m_vertices = mesh.m_vertices;
		m_indices = mesh.m_indices;
		m_drawingMaterial = mesh.m_drawingMaterial;
		return *this;
	}

	void TRDrawableSubMesh::clear()
	{
		std::vector<TRVertex>().swap(m_vertices);
		std::vector<unsigned int>().swap(m_indices);
	}

	//----------------------------------------------AssimpImporterWrapper----------------------------------------------
	class AssimpImporterWrapper final
	{
	public:
		//textureDict is for avoiding redundant loading
		std::map<std::string, int> textureDict = {};
		std::string directory = "";
		bool generatedMipmap = false;

		TRDrawableSubMesh processMesh(aiMesh *mesh, const aiScene *scene)
		{
			TRDrawableSubMesh drawable;

			// data to fill
			std::vector<TRVertex> vertices;
			std::vector<unsigned int> indices;

			// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				TRVertex vertex;
				// positions
				vertex.m_vpositions = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
				// normals
				if (mesh->HasNormals())
				{
					vertex.m_vnormals = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				}
				// texture coordinates
				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vertex.m_vtexcoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
					// tangent
					vertex.m_vtangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
					// bitangent
					vertex.m_vbitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
				}
				else
				{
					vertex.m_vtexcoords = glm::vec2(0.0f, 0.0f);
				}

				vertices.push_back(vertex);
			}

			// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			// process materials
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			auto loadFunc = [&](aiTextureType type) -> int
			{
				for (int i = 0; i < material->GetTextureCount(type); ++i)
				{
					aiString str;
					material->GetTexture(type, i, &str);
					if (textureDict.find(str.C_Str()) != textureDict.end())
					{
						//Already loaded
						return textureDict[str.C_Str()];
					}
					else
					{
						TRTexture2D::ptr diffTex = std::make_shared<TRTexture2D>(generatedMipmap);
						bool success = diffTex->loadTextureFromFile(directory + '/' + str.C_Str());
						auto texId = TRShadingPipeline::uploadTexture2D(diffTex);
						textureDict.insert({ str.C_Str(), texId });
						return texId;
					}
				}
				return -1;
			};

			//Texture maps
			drawable.setDiffuseMapTexId(loadFunc(aiTextureType_DIFFUSE));
			drawable.setSpecularMapTexId(loadFunc(aiTextureType_SPECULAR));
			drawable.setNormalMapTexId(loadFunc(aiTextureType_HEIGHT));
			drawable.setGlowMapTexId(loadFunc(aiTextureType_EMISSIVE));

			drawable.setVertices(vertices);
			drawable.setIndices(indices);

			return drawable;
		}

		void processNode(aiNode *node, const aiScene *scene, std::vector<TRDrawableSubMesh> &drawables)
		{
			// process each mesh located at the current node
			for (unsigned int i = 0; i < node->mNumMeshes; i++)
			{
				// the node object only contains indices to index the actual objects in the scene. 
				// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				drawables.push_back(processMesh(mesh, scene));
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (unsigned int i = 0; i < node->mNumChildren; i++)
			{
				processNode(node->mChildren[i], scene, drawables);
			}
		}
	};

	//----------------------------------------------TRDrawableMesh----------------------------------------------

	void TRDrawableMesh::importMeshFromFile(const std::string &path, bool generatedMipmap)
	{
		for (auto &drawable : m_drawables)
		{
			drawable.clear();
		}
		m_drawables.clear();

		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals 
			| aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_SplitLargeMeshes | aiProcess_FixInfacingNormals);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			exit(1);
		}

		// retrieve the directory path of the filepath
		AssimpImporterWrapper wrapper;
		wrapper.generatedMipmap = generatedMipmap;
		wrapper.directory = path.substr(0, path.find_last_of('/'));
		wrapper.processNode(scene->mRootNode, scene, m_drawables);
		
	}

	void TRDrawableMesh::clear()
	{
		for (auto &drawable : m_drawables)
		{
			drawable.clear();
		}
	}

	TRDrawableMesh::TRDrawableMesh(const std::string &path, bool generatedMipmap)
	{
		importMeshFromFile(path, generatedMipmap);
	}

	unsigned int TRDrawableMesh::getDrawableMaxFaceNums() const
	{
		unsigned int num = 0;
		for (const auto &drawable : m_drawables)
		{
			num = std::max(num, (unsigned int)drawable.getIndices().size() / 3);
		}
		return num;
	}
}