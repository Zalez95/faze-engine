#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <memory>
#include <vector>
class FileReader;

namespace graphics {

	class Material;


	/**
	 * Class MaterialLoader, it's used to load the materials from the given
	 * files
	 */
	class MaterialLoader
	{
	private:	// Nested types
		/** Struct FILE_FORMAT, it holds the name, version and other data of
		 * our material file format */
		struct FILE_FORMAT
		{
			static const std::string	FILE_NAME;
			static const std::string	MATERIAL_FILE_EXTENSION;
			static const unsigned int	VERSION = 1;
			static const unsigned int	REVISION = 3;
		};

		typedef std::unique_ptr<Material> MaterialUPtr;

	public:		// Functions
		/** Creates a new MaterialLoader */
		MaterialLoader() {};

		/** Class destructor */
		~MaterialLoader() {};

		/** Parses the Materials in the given file and returns them
		 * 
		 * @note	the cursor of the FileReader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Materials that we want
		 *			to parse
		 * @return	a vector with the parsed Materials */
		std::vector<MaterialUPtr> load(FileReader* fileReader);
	private:
		/** Checks the header of the given file
		 *
		 * @param	fileReader the reader of the file with the header we want
		 *			to check
		 * @return	true if the file and version is ok, false otherwise */
		bool checkHeader(FileReader* fileReader);

		/** Parses the Materials in the given file and returns them
		 * 
		 * @param	fileReader the file reader with the materials that we want
		 *			to parse
		 * @return	a vector with the parsed materials */
		std::vector<MaterialUPtr> parseMaterials(FileReader* fileReader);

		/** Parses the Material at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Material */
		MaterialUPtr parseMaterial(FileReader* fileReader);
	};

}

#endif		// MESH_LOADER_H
