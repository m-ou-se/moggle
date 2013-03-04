// Copyright 2013 Maurice Bos
//
// This file is part of Moggle.
//
// Moggle is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Moggle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Moggle. If not, see <http://www.gnu.org/licenses/>.

#include <vector>
#include <sstream>
#include <string>
#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <moggle/core/gl.hpp>
#include <moggle/math/matrix.hpp>
#include <moggle/math/normalized.hpp>
#include <moggle/xxx/buffer.hpp>
#include <moggle/xxx/vertices.hpp>
#include <moggle/xxx/mesh.hpp>

namespace moggle {

mesh import_mesh(char const * file_name) {
	Assimp::Importer importer;

	aiScene const * ai_scene = importer.ReadFile(
		file_name,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenNormals |
		aiProcess_SortByPType
	);

	if (!ai_scene) throw std::runtime_error(std::string("Unable to import mesh: ") + importer.GetErrorString());

	aiMesh const & am = *ai_scene->mMeshes[0]; // TODO: What about multiple meshes?

	vertices vertices;

	{
		buffer<hvector4<float>> positions(am.mNumVertices);
		buffer<vector3<float>> normals(am.mNumVertices);
		for(unsigned int i = 0; i < am.mNumVertices; ++i) {
			positions[i] = { am.mVertices[i][0], am.mVertices[i][1], am.mVertices[i][2] };
			normals  [i] = { am. mNormals[i][0], am. mNormals[i][1], am. mNormals[i][2] };
		}
		vertices.attribute("position", std::move(positions));
		vertices.attribute("normal"  , std::move(normals  ));
	}

	for (size_t n = 0; n < am.GetNumColorChannels(); ++n) {
		buffer<hvector4<normalized_uint8_t>> colors(am.mNumVertices);
		for(unsigned int i = 0; i < am.mNumVertices; ++i) {
			colors[i] = {
				normalized_uint8_t::raw(am.mColors[n][i].r),
				normalized_uint8_t::raw(am.mColors[n][i].g),
				normalized_uint8_t::raw(am.mColors[n][i].b),
				normalized_uint8_t::raw(am.mColors[n][i].a)
			};
		}
		std::ostringstream name("color", std::ios_base::ate);
		if (n) name << (n + 1);
		vertices.attribute(name.str(), std::move(colors));
	}

	for (size_t n = 0; n < am.GetNumUVChannels(); ++n) {
		buffer<vector3<float>> uvs(am.mNumVertices);
		for(unsigned int i = 0; i < am.mNumVertices; ++i) {
			uvs[i] = { am.mTextureCoords[n][i][0], am.mTextureCoords[n][i][1], am.mTextureCoords[n][i][1] };
		}
		std::ostringstream name("texture_coordinate", std::ios_base::ate);
		if (n) name << (n + 1);
		vertices.attribute(name.str(), std::move(uvs));
	}

	buffer<GLushort> indices(am.mNumFaces * 3);

	for (size_t i = 0; i < am.mNumFaces; ++i) {
		indices[i*3  ] = am.mFaces[i].mIndices[0];
		indices[i*3+1] = am.mFaces[i].mIndices[1];
		indices[i*3+2] = am.mFaces[i].mIndices[2];
	}

	return mesh(std::move(vertices), std::move(indices));
}

}
