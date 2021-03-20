#include "TRDrawableMesh.h"

#include <map>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "TRTexture2D.h"
#include "TRShadingPipeline.h"

namespace TinyRenderer
{

	TRDrawableMesh::TRDrawableMesh(const std::string &filename)
	{
		loadMeshFromFile(filename);
	}

	void TRDrawableMesh::clear()
	{
		m_vertices_attrib.clear();
		std::vector<TRMeshFace>().swap(m_mesh_faces);
	}

	TRDrawableMesh& TRDrawableMesh::operator=(const TRDrawableMesh& mesh)
	{
		if (&mesh == this)
			return *this;
		m_vertices_attrib = mesh.m_vertices_attrib;
		m_mesh_faces = mesh.m_mesh_faces;
		return *this;
	}

	void TRDrawableMesh::loadMeshFromFile(const std::string &filename)
	{
		clear();

		//Refs: https://github.com/tinyobjloader/tinyobjloader

		tinyobj::ObjReaderConfig reader_config;

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(filename, reader_config)) 
		{
			if (!reader.Error().empty()) 
			{
				std::cerr << "TinyObjReader: " << reader.Error();
			}
			exit(1);
		}

		if (!reader.Warning().empty()) 
		{
			std::cout << "TinyObjReader: " << reader.Warning();
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		//Load the textures
		std::vector<glm::ivec4> matTextureIds;
		{
			//texDict is for avoiding redundant loading
			std::map<std::string, int> texDict;

			size_t pos = filename.find_last_of("/\\");
			std::string baseDir = "./";
			baseDir = ((pos != std::string::npos) ? filename.substr(0, pos) : reader_config.mtl_search_path) + "/";

			for (size_t m = 0; m < materials.size(); ++m)
			{
				//Note: we use the index returned from upload function for fetching the texture in shaders
				glm::ivec4 texIds(-1, -1, -1, -1);
				const tinyobj::material_t* mp = &materials[m];

				//Load the diffuse texture and upload to shading pipeline
				if (mp->diffuse_texname.length() > 0)
				{
					if (texDict.find(mp->diffuse_texname) != texDict.end())
					{
						//Already loaded
						texIds.x = texDict[mp->diffuse_texname];
					}
					else
					{
						TRTexture2D::ptr diffTex = std::make_shared<TRTexture2D>();
						bool success = diffTex->loadTextureFromFile(baseDir + mp->diffuse_texname);
						texIds.x = TRShadingPipeline::upload_texture_2D(diffTex);
						texDict.insert({ mp->diffuse_texname, texIds.x });
					}
				}

				//Load the specular texture and upload to shading pipeline
				if (mp->specular_texname.length() > 0)
				{
					if (texDict.find(mp->specular_texname) != texDict.end())
					{
						//Already loaded
						texIds.y = texDict[mp->specular_texname];
					}
					else
					{
						TRTexture2D::ptr specuTex = std::make_shared<TRTexture2D>();
						bool success = specuTex->loadTextureFromFile(baseDir + mp->specular_texname);
						texIds.y = TRShadingPipeline::upload_texture_2D(specuTex);
						texDict.insert({ mp->specular_texname, texIds.y });
					}
				}

				//Load the normal texture and upload to shading pipeline
				if (mp->bump_texname.length() > 0)
				{
					if (texDict.find(mp->bump_texname) != texDict.end())
					{
						//Already loaded
						texIds.z = texDict[mp->bump_texname];
					}
					else
					{
						TRTexture2D::ptr normTex = std::make_shared<TRTexture2D>();
						bool success = normTex->loadTextureFromFile(baseDir + mp->bump_texname);
						texIds.z = TRShadingPipeline::upload_texture_2D(normTex);
					}
				}

				//Load the emissive texture and upload to shading pipeline
				if (mp->emissive_texname.length() > 0)
				{
					if (texDict.find(mp->emissive_texname) != texDict.end())
					{
						//Already loaded
						texIds.w = texDict[mp->emissive_texname];
					}
					else
					{
						TRTexture2D::ptr glowTex = std::make_shared<TRTexture2D>();
						bool success = glowTex->loadTextureFromFile(baseDir + mp->emissive_texname);
						texIds.w = TRShadingPipeline::upload_texture_2D(glowTex);
						texDict.insert({ mp->emissive_texname, texIds.w });
					}
				}

				matTextureIds.push_back(texIds);
			}

		}

		//Geometry loading
		{
			for (size_t i = 0; i < attrib.vertices.size(); i += 3)
			{
				m_vertices_attrib.vpositions.push_back(
					glm::vec4(attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2], 1.0f));
				m_vertices_attrib.vcolors.push_back(
					glm::vec4(attrib.colors[i + 0], attrib.colors[i + 1], attrib.colors[i + 2], 1.0f));
			}
			for (size_t i = 0; i < attrib.normals.size(); i += 3)
			{
				m_vertices_attrib.vnormals.push_back(
					glm::vec3(attrib.normals[i + 0], attrib.normals[i + 1], attrib.normals[i + 2]));
			}
			for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
			{
				m_vertices_attrib.vtexcoords.push_back(
					glm::vec2(attrib.texcoords[i + 0], attrib.texcoords[i + 1]));
			}
			for (size_t s = 0; s < shapes.size(); ++s)
			{
				size_t index_offset = 0;
				for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f)
				{
					int fv = shapes[s].mesh.num_face_vertices[f];
					TRMeshFace face;
					for (size_t v = 0; v < fv && v < 3; ++v)
					{
						tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
						face.vposIndex[v] = idx.vertex_index;
						face.vnorIndex[v] = idx.normal_index;
						face.vtexIndex[v] = idx.texcoord_index;
					}
					//Material
					{
						if (shapes[s].mesh.material_ids[f] < materials.size())
						{
							const tinyobj::material_t* mp = &materials[shapes[s].mesh.material_ids[f]];
							face.kA = glm::vec3(mp->ambient[0], mp->ambient[1], mp->ambient[2]);
							face.kD = glm::vec3(mp->diffuse[0], mp->diffuse[1], mp->diffuse[2]);
							face.kS = glm::vec3(mp->specular[0], mp->specular[1], mp->specular[2]);
							face.kE = glm::vec3(mp->emission[0], mp->emission[1], mp->emission[2]);
							face.shininess = mp->shininess;
							face.diffuseMapTexId = matTextureIds[shapes[s].mesh.material_ids[f]].x;
							face.specularMapTexId = matTextureIds[shapes[s].mesh.material_ids[f]].y;
							face.normalMapTexId = matTextureIds[shapes[s].mesh.material_ids[f]].z;
							face.glowMapTexId = matTextureIds[shapes[s].mesh.material_ids[f]].w;
						}
					}

					//TBN matrix calculation for normal mapping
					//Refs: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
					{
						
						glm::vec3 edge1 = glm::vec3(m_vertices_attrib.vpositions[face.vposIndex[1]]) 
							- glm::vec3(m_vertices_attrib.vpositions[face.vposIndex[0]]);
						glm::vec3 edge2 = glm::vec3(m_vertices_attrib.vpositions[face.vposIndex[2]])
							- glm::vec3(m_vertices_attrib.vpositions[face.vposIndex[0]]);

						glm::vec2 deltaUV1 = glm::vec2(m_vertices_attrib.vtexcoords[face.vtexIndex[1]])
							- glm::vec2(m_vertices_attrib.vtexcoords[face.vtexIndex[0]]);
						glm::vec2 deltaUV2 = glm::vec2(m_vertices_attrib.vtexcoords[face.vtexIndex[2]])
							- glm::vec2(m_vertices_attrib.vtexcoords[face.vtexIndex[0]]);

						float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

						glm::vec3 tangent;
						tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
						tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
						tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

						glm::vec3 bitangent;
						bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
						bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
						bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

						face.tangent = glm::normalize(tangent);
						face.bitangent = glm::normalize(bitangent);
					}

					m_mesh_faces.push_back(face);
					index_offset += fv;
				}
			}
		}
		
	}

}