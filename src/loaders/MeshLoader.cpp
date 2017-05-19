#include "MeshLoader.h"
#include <map>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include "../utils/FileReader.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/buffers/VertexBuffer.h"
#include "../graphics/buffers/IndexBuffer.h"
#include "../graphics/buffers/VertexArray.h"

namespace graphics {

// Static variables definition
	const std::string MeshLoader::FILE_FORMAT::FILE_NAME		= "FAZE_MSH_FILE";
	const std::string MeshLoader::FILE_FORMAT::FILE_EXTENSION	= ".fzmsh";

// Public Functions
	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLushort>& faceIndices
	) {
		auto vao = std::make_unique<VertexArray>();
		auto ibo = std::make_unique<IndexBuffer>(faceIndices.data(), faceIndices.size());
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		std::unique_ptr<VertexBuffer> tmp;

		tmp = std::make_unique<VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}


	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLfloat>& jointWeights,
		const std::vector<GLushort>& jointIndices,
		const std::vector<GLushort>& faceIndices
	) {
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		auto ibo = std::make_unique<IndexBuffer>(faceIndices.data(), faceIndices.size());
		auto vao = std::make_unique<VertexArray>();

		std::unique_ptr<VertexBuffer> tmp;
		tmp = std::make_unique<VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(jointWeights.data(), jointWeights.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_WEIGHT_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(jointIndices.data(), jointIndices.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_INDEX_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}


	std::vector<MeshLoader::MeshUPtr> MeshLoader::load(FileReader* fileReader)
	{
		try {
			// 1. Get the input file
			if (!fileReader || fileReader->fail()) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file\n");
			}

			// 3. Parse the Meshes
			return parseMeshes(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Mesh in the file \"" + fileReader->getCurrentFilePath() + "\":\n" + e.what());
		}
	}


	std::vector<GLfloat> MeshLoader::calculateNormals(
		const std::vector<GLfloat>& positions,
		const std::vector<GLushort>& faceIndices
	) const
	{
		std::vector<GLfloat> normals(positions.size(), 0);

		// Sum to the normal of every vertex, the normal of the faces 
		// which it belongs
		for (unsigned int i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			GLfloat v1_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+1]];
			GLfloat v1_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+1] + 1];
			GLfloat v1_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+1] + 2];
			glm::vec3 v1(v1_x, v1_y, v1_z);

			GLfloat v2_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+2]];
			GLfloat v2_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+2] + 1];
			GLfloat v2_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+2] + 2];
			glm::vec3 v2(v2_x, v2_y, v1_z);

			glm::vec3 normal = glm::cross(v1, v2);

			normals[3 * faceIndices[i]]			+= normal.x;
			normals[3 * faceIndices[i] + 1]		+= normal.y;
			normals[3 * faceIndices[i] + 2]		+= normal.z;

			normals[3 * faceIndices[i+1]]		+= normal.x;
			normals[3 * faceIndices[i+1] + 1]	+= normal.y;
			normals[3 * faceIndices[i+1] + 2]	+= normal.z;

			normals[3 * faceIndices[i+2]]		+= normal.x;
			normals[3 * faceIndices[i+2] + 1]	+= normal.y;
			normals[3 * faceIndices[i+2] + 2]	+= normal.z;
		}

		// Normalize the normal vector of every vertex
		for (unsigned int i = 0; i < normals.size(); i+=3) {
			GLfloat length	= sqrt( pow(normals[i], 2) + pow(normals[i+1], 2) + pow(normals[i+2], 2) );
			normals[i]		/= length;
			normals[i+1]	/= length;
			normals[i+2]	/= length;
		}

		return normals;
	}

// Private functions
	bool MeshLoader::checkHeader(FileReader* fileReader)
	{
		const std::string FILE_VERSION = std::to_string(FILE_FORMAT::VERSION) + '.' + std::to_string(FILE_FORMAT::REVISION);
		bool ret = false;

		std::string fileName, fileVersion;
		if (fileReader->getParam(fileName)
			&& fileReader->getParam(fileVersion)
			&& fileName == FILE_FORMAT::FILE_NAME
			&& fileVersion == FILE_VERSION)
		{
			ret = true;
		}

		return ret;
	}

	
	std::vector<MeshLoader::MeshUPtr> MeshLoader::parseMeshes(FileReader* fileReader)
	{
		std::vector<MeshUPtr> meshes;
		unsigned int numMeshes = 0, meshIndex = 0;

		while (!fileReader->eof()) {
			std::string token;
			if (fileReader->getParam(token)) {
				if (token == "num_meshes") {
					fileReader->getParam(numMeshes);
					meshes.resize(numMeshes);
				}
				else if (token == "mesh") {
					if (meshIndex < numMeshes) {
						meshes[meshIndex] = parseMesh(fileReader);
					}
					++meshIndex;
				}
				else {
					throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader->getNumLines()) + '\n');
				}
			}
		}

		if (meshIndex != numMeshes) {
			throw std::runtime_error("Error: expected " + std::to_string(numMeshes) + " meshes, parsed " + std::to_string(meshIndex) + '\n');
		}

		return meshes;
	}


	MeshLoader::MeshUPtr MeshLoader::parseMesh(FileReader* fileReader)
	{
		std::string name;
		std::vector<GLfloat> positions, uvs;
		std::vector<GLushort> posIndices, uvIndices;
		unsigned int numPositions = 0, numUVs = 0, numFaces = 0, numJoints = 0;
		unsigned int positionIndex = 0, uvIndex = 0, faceIndex = 0, jointIndex = 0;
		
		std::string trash;
		fileReader->getParam(name);
		fileReader->getParam(trash);

		bool end = false;
		while (!end) {
			std::string token;
			fileReader->getParam(token);

			if (token == "num_positions") {
				fileReader->getParam(numPositions);
				positions.resize(3 * numPositions);
			}
			else if (token == "num_uvs") {
				fileReader->getParam(numUVs);
				uvs.resize(2 * numUVs);
			}
			else if (token == "num_faces") {
				fileReader->getParam(numFaces);
				posIndices.resize(3 * numFaces);
				if (numUVs > 0) { uvIndices.resize(3 * numFaces); }
			}
			else if (token == "num_joints") {
				fileReader->getParam(numJoints);
			}
			else if (token == "v") {
				if (positionIndex < numPositions) {
					fileReader->getParam(positions[3 * positionIndex]);
					fileReader->getParam(positions[3 * positionIndex + 1]);
					fileReader->getParam(positions[3 * positionIndex + 2]);
				}
				++positionIndex;
			}
			else if (token == "uv"){
				unsigned int vi;
				fileReader->getParam(vi);
				if (vi < numPositions) {
					fileReader->getParam(uvs[2 * vi]);
					fileReader->getParam(uvs[2 * vi + 1]);
				}
				++uvIndex;
			}
			else if (token == "f") {
				if (faceIndex < numFaces) {
					fileReader->getParam(trash);
					fileReader->getParam(posIndices[3 * faceIndex]);
					fileReader->getParam(posIndices[3 * faceIndex + 1]);
					fileReader->getParam(posIndices[3 * faceIndex + 2]);
					fileReader->getParam(trash);
					if (numUVs > 0) {
						fileReader->getParam(trash);
						fileReader->getParam(uvIndices[3 * faceIndex]);
						fileReader->getParam(uvIndices[3 * faceIndex + 1]);
						fileReader->getParam(uvIndices[3 * faceIndex + 2]);
						fileReader->getParam(trash);
					}
				}
				++faceIndex;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader->getNumLines()) + '\n');
			}
		}

		if (positionIndex != numPositions) {
			throw std::runtime_error("Error: expected " + std::to_string(numPositions) + " positions, parsed " + std::to_string(positionIndex) + '\n');
		}
		if (uvIndex != numUVs) {
			throw std::runtime_error("Error: expected " + std::to_string(numUVs) + " UVs, parsed " + std::to_string(uvIndex) + '\n');
		}
		if (faceIndex != numFaces) {
			throw std::runtime_error("Error: expected " + std::to_string(numFaces) + " faces, parsed " + std::to_string(faceIndex) + '\n');
		}

		return processMeshData(name, positions, uvs, posIndices, uvIndices);
	}


	MeshLoader::MeshUPtr MeshLoader::processMeshData(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLushort>& posIndices,
		const std::vector<GLushort>& uvIndices
	) {
		std::vector<GLfloat> positions2, uvs2, normals;
		std::vector<GLushort> faceIndices;

		if (!uvIndices.empty()) {
			faceIndices = std::vector<GLushort>(posIndices.size());
			std::map<std::pair<GLushort, GLushort>, GLushort> faceIndicesMap;

			for (unsigned int i = 0; i < faceIndices.size(); ++i) {
				unsigned int positionIndex	= posIndices[i];
				unsigned int uvIndex		= uvIndices[i];

				std::pair<GLushort, GLushort> mapKey(positionIndex, uvIndex);
				if (faceIndicesMap.find(mapKey) != faceIndicesMap.end()) {
					faceIndices[i] = faceIndicesMap[mapKey];
				}
				else {
					GLushort vertexIndex	= static_cast<GLushort>(positions2.size() / 3);

					positions2.push_back( positions[3 * positionIndex] );
					positions2.push_back( positions[3 * positionIndex + 1] );
					positions2.push_back( positions[3 * positionIndex + 2] );
					uvs2.push_back( uvs[2 * uvIndex] );
					uvs2.push_back( uvs[2 * uvIndex + 1] );
					faceIndicesMap[mapKey]	= vertexIndex;
					faceIndices[i]			= vertexIndex;
				}
			}
		}
		else {
			positions2	= positions;
			uvs2		= std::vector<GLfloat>(positions.size());
			faceIndices	= posIndices;
		}

		normals = calculateNormals(positions2, faceIndices);
		return createMesh(name, positions2, normals, uvs2, faceIndices);
	}

}
