#include "TerrainReader.h"
#include "../utils/FileReader.h"
#include "../game/Entity.h"

namespace loaders {

// Public functions
	TerrainReader::EntityUPtr TerrainReader::load(utils::FileReader& fileReader)
	{
		try {
			// 1. Get the input file
			if (fileReader.fail()) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Parse the Entity
			return parseEntity(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Terrain in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}

// Private functions
	TerrainReader::EntityUPtr TerrainReader::parseEntity(utils::FileReader& fileReader)
	{
		std::string name, heightMapPath;
		float size, maxHeight;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "size") {
				fileReader >> size;
			}
			else if (token == "height_map") {
				fileReader >> heightMapPath;
			}
			else if (token == "max_height") {
				fileReader >> maxHeight;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		std::unique_ptr<utils::Image> heightMap( mImageReader.read(heightMapPath, utils::ImageFormat::L_IMAGE) );
		return mTerrainLoader.createTerrain(name, size, *heightMap, maxHeight);
	}

}
