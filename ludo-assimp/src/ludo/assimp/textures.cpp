/*
 * This file is part of ludo. See the LICENSE file for the full license governing this code.
 */

#include <FreeImagePlus.h>

#include "textures.h"

namespace ludo
{
  texture import_texture(const std::string& folder, const aiScene& assimp_scene, const import_object& mesh_object)
  {
    auto assimp_material = assimp_scene.mMaterials[assimp_scene.mMeshes[mesh_object.mesh_index]->mMaterialIndex];
    auto texture_path = aiString();
    assimp_material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path);
    if (!texture_path.length)
    {
      return {};
    }

    auto image = fipImage();
    image.load((folder + texture_path.C_Str()).c_str());

    auto bits_per_pixel = image.getBitsPerPixel();
    if (bits_per_pixel != 24 && bits_per_pixel != 32)
    {
      assert(false && "unsupported pixel datatype");
    }

    auto color_type = image.getColorType();
    if (color_type != FIC_RGB && color_type != FIC_RGBALPHA)
    {
      assert(false && "unsupported pixel components");
    }

    if (FI_RGBA_BLUE != 0)
    {
      assert(false && "unsupported platform (need to implement solution for this)");
    }

    auto texture = ludo::texture
    {
      .components = bits_per_pixel == 32 ? pixel_components::BGRA : pixel_components::BGR,
      .width = image.getWidth(),
      .height = image.getHeight()
    };
    init(texture);

    write(texture, reinterpret_cast<std::byte*>(image.accessPixels()));

    return texture;
  }
}
