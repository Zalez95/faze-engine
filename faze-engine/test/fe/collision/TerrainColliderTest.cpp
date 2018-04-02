#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/TerrainCollider.h>
#include <fe/collision/ConvexPolyhedron.h>

#define TOLERANCE 0.000000001f


TEST(TerrainCollider, getAABB)
{
	const std::vector<float> heights = {
		-0.224407124f, -0.182230042f, -0.063670491f, -0.063680544f, -0.274178390f, -0.002076677f,
		0.240925990f, -0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f, -0.101790362f, -0.419971141f, -0.278538079f, 0.044960733f, -0.266057232f,
		0.251054237f, 0.476726697f, -0.422780143f, 0.063881184f, -0.266370011f, -0.139245431f,
		-0.279247346f, -0.234977409f, -0.294798492f, -0.247099806f, 0.002694404f, 0.378445211f,
		0.112437157f, 0.392135236f, 0.466178188f, -0.306503992f, -0.381612994f, -0.219027959f,
		0.112001758f, -0.283234569f, 0.367756026f, -0.288402094f, -0.006938715f, -0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f, -0.250951479f, 0.104189257f, -0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const glm::vec3 expectedMinimum(-0.5f, -0.427923002f, -0.5f);
	const glm::vec3 expectedMaximum(0.5f, 0.49946191f, 0.5f);

	fe::collision::TerrainCollider tc1(heights, xSize, zSize);
	fe::collision::AABB aabb1 = tc1.getAABB();
	EXPECT_EQ(aabb1.minimum, expectedMinimum);
	EXPECT_EQ(aabb1.maximum, expectedMaximum);
}


TEST(TerrainCollider, getAABBTransforms)
{
	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<float> heights = {
		-0.224407124f, -0.182230042f, -0.063670491f, -0.063680544f, -0.274178390f, -0.002076677f,
		0.240925990f, -0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f, -0.101790362f, -0.419971141f, -0.278538079f, 0.044960733f, -0.266057232f,
		0.251054237f, 0.476726697f, -0.422780143f, 0.063881184f, -0.266370011f, -0.139245431f,
		-0.279247346f, -0.234977409f, -0.294798492f, -0.247099806f, 0.002694404f, 0.378445211f,
		0.112437157f, 0.392135236f, 0.466178188f, -0.306503992f, -0.381612994f, -0.219027959f,
		0.112001758f, -0.283234569f, 0.367756026f, -0.288402094f, -0.006938715f, -0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f, -0.250951479f, 0.104189257f, -0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const glm::vec3 expectedMinimum(-9.358484268f, -8.048053741f, -2.782845735f);
	const glm::vec3 expectedMaximum(3.376655340f, 4.209253787f, 11.290613174f);

	fe::collision::TerrainCollider tc1(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	tc1.setTransforms(t * r * s);

	fe::collision::AABB aabb1 = tc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(aabb1.minimum[i] - expectedMinimum[i]), TOLERANCE);
		EXPECT_LE(abs(aabb1.maximum[i] - expectedMaximum[i]), TOLERANCE);
	}
}


TEST(Terrainlider, getOverlapingParts)
{
	const glm::vec3 scale(8.0f, 3.5f, 16.0f);
	const glm::vec3 translation(-3.24586f, -1.559f, 4.78164f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<float> heights = {
		-0.224407124f, -0.182230042f, -0.063670491f, -0.063680544f, -0.274178390f, -0.002076677f,
		0.240925990f, -0.427923002f, 0.499461910f, 0.320841177f, 0.431347578f, 0.199959035f,
		-0.225947124f, -0.101790362f, -0.419971141f, -0.278538079f, 0.044960733f, -0.266057232f,
		0.251054237f, 0.476726697f, -0.422780143f, 0.063881184f, -0.266370011f, -0.139245431f,
		-0.279247346f, -0.234977409f, -0.294798492f, -0.247099806f, 0.002694404f, 0.378445211f,
		0.112437157f, 0.392135236f, 0.466178188f, -0.306503992f, -0.381612994f, -0.219027959f,
		0.112001758f, -0.283234569f, 0.367756026f, -0.288402094f, -0.006938715f, -0.109673572f,
		-0.283075078f, 0.129306909f, 0.134741993f, -0.250951479f, 0.104189257f, -0.422417659f
	};
	const int xSize = 6, zSize = 8;
	const fe::collision::AABB aabb1(
		glm::vec3(-3.536325216f, -0.434814631f, 0.558086156f),
		glm::vec3(-2.536325216f, 0.565185368f, 1.558086156f)
	);

	fe::collision::TerrainCollider tc1(heights, xSize, zSize);
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r * s;

	tc1.setTransforms(transforms);

	auto result = tc1.getOverlapingParts(aabb1);

	std::vector<fe::collision::ConvexPolyhedron> expectedRes = {
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.5f, 0.240925982f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.300000011f, -0.427922993f, -0.357142865f),
			glm::vec3(-0.100000001f, 0.499461919f, -0.357142865f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.100000001f, 0.499461919f, -0.357142865f),
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.5f, -0.225947126f, -0.214285716f),
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.5f, 0.251054227f, -0.071428574f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f),
			glm::vec3(-0.5f, 0.251054227f, -0.071428574f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.300000011f, -0.101790361f, -0.214285716f),
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f)
		}),
		fe::collision::ConvexPolyhedron({
			glm::vec3(-0.100000001f, -0.419971138f, -0.214285716f),
			glm::vec3(-0.100000001f, -0.422780156f, -0.071428574f),
			glm::vec3(-0.300000011f, 0.476726710f, -0.071428574f)
		})
	};
	for (fe::collision::ConvexPolyhedron& cp : expectedRes) {
		cp.setTransforms(transforms);
	}

	ASSERT_EQ(result.size(), expectedRes.size());
	for (size_t i = 0; i < result.size(); ++i) {
		fe::collision::AABB aabb2 = result[i]->getAABB();
		fe::collision::AABB aabb3 = expectedRes[i].getAABB();
		for (int j = 0; j < 3; ++j) {
			EXPECT_LE(abs(aabb2.minimum[j] - aabb3.minimum[j]), TOLERANCE);
			EXPECT_LE(abs(aabb2.maximum[j] - aabb3.maximum[j]), TOLERANCE);
		}
	}
}