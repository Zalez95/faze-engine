#include <array>
#include <glm/gtc/type_ptr.hpp>
#include "se/app/loaders/MeshLoader.h"
#include "se/app/RawMesh.h"
#include "se/graphics/core/VertexBuffer.h"
#include "se/graphics/core/IndexBuffer.h"
#include "se/graphics/core/VertexArray.h"
#include "se/collision/HalfEdgeMeshExt.h"

namespace se::app {

	static void createInternalRingsMesh(RawMesh&, std::size_t, std::size_t, float, glm::vec2);


	graphics::Mesh MeshLoader::createGraphicsMesh(const RawMesh& rawMesh)
	{
		using namespace graphics;

		VertexArray vao;
		std::vector<VertexBuffer> vbos;

		if (!rawMesh.positions.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.positions.front()), 3 * rawMesh.positions.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::PositionAttribute), TypeId::Float, false, 3, 0);
			vao.unbind();
		}

		if (!rawMesh.normals.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.normals.front()), 3 * rawMesh.normals.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::NormalAttribute), TypeId::Float, false, 3, 0);
			vao.unbind();
		}

		if (!rawMesh.tangents.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.tangents.front()), 3 * rawMesh.tangents.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::TangentAttribute), TypeId::Float, false, 3, 0);
			vao.unbind();
		}

		if (!rawMesh.texCoords.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.texCoords.front()), 2 * rawMesh.texCoords.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::TexCoordAttribute0), TypeId::Float, false, 2, 0);
			vao.unbind();
		}

		if (!rawMesh.jointIndices.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.jointIndices.front()), 4 * rawMesh.jointIndices.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::JointIndexAttribute), TypeId::UnsignedShort, false, 4, 0);
			vao.unbind();
		}

		if (!rawMesh.jointWeights.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.setData(glm::value_ptr(rawMesh.jointWeights.front()), 4 * rawMesh.jointWeights.size());

			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::JointWeightAttribute), TypeId::Float, false, 4, 0);
			vao.unbind();
		}

		IndexBuffer ibo(rawMesh.faceIndices.data(), TypeId::UnsignedShort, rawMesh.faceIndices.size());
		vao.bind();
		ibo.bind();
		vao.unbind();

		return Mesh(std::move(vbos), std::move(ibo), std::move(vao));
	}


	std::pair<collision::HalfEdgeMesh, bool> MeshLoader::createHalfEdgeMesh(const RawMesh& rawMesh)
	{
		collision::HalfEdgeMesh heMesh;

		// Add the HEVertices
		std::vector<int> heVertexIndices;
		heVertexIndices.reserve(rawMesh.positions.size());
		for (const glm::vec3& position : rawMesh.positions) {
			heVertexIndices.push_back( collision::addVertex(heMesh, position) );
		}

		// Add the HEFaces
		bool allFacesLoaded = true;
		for (std::size_t i = 0; i < rawMesh.faceIndices.size(); i += 3) {
			std::array<int, 3> vertexIndices = {
				heVertexIndices[ rawMesh.faceIndices[i] ],
				heVertexIndices[ rawMesh.faceIndices[i+1] ],
				heVertexIndices[ rawMesh.faceIndices[i+2] ]
			};
			if (collision::addFace(heMesh, vertexIndices.begin(), vertexIndices.end()) < 0) {
				allFacesLoaded = false;
			}
		}

		// Validate the HEMesh
		return std::pair(heMesh, allFacesLoaded && collision::validateMesh(heMesh).first);
	}


	RawMesh MeshLoader::createSphereMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve((rings - 1) * segments + 2);
		rawMesh.faceIndices.reserve(6 * (rings - 2) * segments + 3 * 2 * segments);

		// Creates the bottom skullcap
		rawMesh.positions.push_back({ 0.0f, -radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(0);
			rawMesh.faceIndices.push_back(j + 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? j + 2 : 1);
		}

		// Creates the internal rings
		float ringAngle = glm::pi<float>() / rings;
		createInternalRingsMesh(rawMesh, segments, rings-2, radius, { ringAngle - glm::half_pi<float>(), glm::half_pi<float>() - ringAngle });

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 2 - segments + j);
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? rawMesh.positions.size() - 1 - segments + j : rawMesh.positions.size() - 2 - segments);
		}

		return std::move(rawMesh);
	}


	RawMesh MeshLoader::createDomeMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve(rings * segments + 1);
		rawMesh.faceIndices.reserve(6 * (rings - 2) * segments + 3 * 2 * segments);

		// Creates the internal rings
		float ringAngle = glm::pi<float>() / rings;
		createInternalRingsMesh(rawMesh, segments, rings-1, radius, { 0, glm::half_pi<float>() - ringAngle });

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 2 - segments + j);
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? rawMesh.positions.size() - 1 - segments + j : rawMesh.positions.size() - 2 - segments);
		}

		return std::move(rawMesh);
	}


	std::vector<glm::vec3> MeshLoader::calculateNormals(
		const std::vector<glm::vec3>& positions,
		const std::vector<unsigned short>& faceIndices
	) {
		std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));

		// The normal vector of every vertex is calculated as the sum of the
		// normal vectors of the faces it belongs to
		for (std::size_t i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			const glm::vec3& v1 = positions[faceIndices[i+1]] - positions[faceIndices[i]];
			const glm::vec3& v2 = positions[faceIndices[i+2]] - positions[faceIndices[i]];
			glm::vec3 normal = glm::cross(v1, v2);

			normals[faceIndices[i]]		+= normal;
			normals[faceIndices[i+1]]	+= normal;
			normals[faceIndices[i+2]]	+= normal;
		}

		// Normalize the normal vector of every vertex
		for (glm::vec3& normal : normals) {
			normal = glm::normalize(normal);
		}

		return normals;
	}


	std::vector<glm::vec3> MeshLoader::calculateTangents(
		const std::vector<glm::vec3>& positions,
		const std::vector<glm::vec2>& texCoords,
		const std::vector<unsigned short>& faceIndices
	) {
		std::vector<glm::vec3> tangents(positions.size(), glm::vec3(0.0f));

		// The tangent vector of every vertex is calculated as the sum of the
		// tangent vectors of the faces it belongs to
		for (std::size_t i = 0; i < faceIndices.size(); i+=3) {
			const glm::vec3& e1		= positions[faceIndices[i+1]] - positions[faceIndices[i]];
			const glm::vec3& e2		= positions[faceIndices[i+2]] - positions[faceIndices[i]];
			const glm::vec2& dUV1	= texCoords[faceIndices[i+1]] - texCoords[faceIndices[i]];
			const glm::vec2& dUV2	= texCoords[faceIndices[i+2]] - texCoords[faceIndices[i]];

			glm::vec3 tangent(0.0f);
			float invertedDeterminant = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
			tangent.x = invertedDeterminant * (dUV2.y * e1.x - dUV1.y * e2.x);
			tangent.y = invertedDeterminant * (dUV2.y * e1.y - dUV1.y * e2.y);
			tangent.z = invertedDeterminant * (dUV2.y * e1.z - dUV1.y * e2.z);
			tangents[faceIndices[i]]	+= tangent;
			tangents[faceIndices[i+1]]	+= tangent;
			tangents[faceIndices[i+2]]	+= tangent;
		}

		// Normalize the tangent vector of every vertex
		for (glm::vec3& tangent : tangents) {
			tangent = glm::normalize(tangent);
		}

		return tangents;
	}


	void createInternalRingsMesh(
		RawMesh& rawMesh,
		std::size_t segments, std::size_t rings, float radius,
		glm::vec2 latitude
	) {
		std::size_t currentRingIndex = rawMesh.positions.size();

		// Creates the vertices
		float segmentAngle = glm::two_pi<float>() / segments;
		float ringAngle = (latitude[1] - latitude[0]) / rings;

		for (std::size_t i = 0; i < rings + 1; ++i) {
			float currentRingLatitude = i * ringAngle + latitude[0];
			float currentRingRadius = radius * glm::cos(currentRingLatitude);

			float y = radius * glm::sin(currentRingLatitude);
			for (std::size_t j = 0; j < segments; ++j) {
				float currentSegmentLongitude = j * segmentAngle - glm::pi<float>();
				float x = currentRingRadius * glm::cos(currentSegmentLongitude);
				float z = currentRingRadius * glm::sin(currentSegmentLongitude);
				rawMesh.positions.push_back({ x, y, z });
			}
		}

		// Creates the face indices
		for (std::size_t i = 0; i < rings; ++i) {
			std::size_t previousRingIndex = currentRingIndex;
			currentRingIndex += segments;

			for (std::size_t j = 0; j < segments; ++j) {
				rawMesh.faceIndices.push_back(previousRingIndex + j);
				rawMesh.faceIndices.push_back(currentRingIndex + j);
				rawMesh.faceIndices.push_back((j + 1 < segments)? currentRingIndex + j + 1 : currentRingIndex);
				rawMesh.faceIndices.push_back(previousRingIndex + j);
				rawMesh.faceIndices.push_back((j + 1 < segments)? currentRingIndex + j + 1 : currentRingIndex);
				rawMesh.faceIndices.push_back((j + 1 < segments)? previousRingIndex + j + 1 : previousRingIndex);
			}
		}
	}

}