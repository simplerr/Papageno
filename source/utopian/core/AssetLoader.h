#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "utility/Module.h"
#include "utility/Common.h"
#include "vulkan\VulkanPrerequisites.h"

#define DEFAULT_NORMAL_MAP_TEXTURE "data/textures/flat_normalmap.png"

namespace Utopian
{
   class Model;

   enum GrassAsset
   {
      FLOWER_BOUNCING_BET_01_1 = 0,
      FLOWER_BOUNCING_BET_01_2,
      FLOWER_BOUNCING_BET_01_CROSS_1,
      FLOWER_BOUNCING_BET_01_DETAILED_1,
      FLOWER_BROWNRAY_KNAPWEED_01_1,
      FLOWER_BROWNRAY_KNAPWEED_01_2,
      FLOWER_BROWNRAY_KNAPWEED_01_CROSS_1,
      FLOWER_BROWNRAY_KNAPWEED_01_DETAILED_1,
      FLOWER_BROWNRAY_KNAPWEED_01_DETAILED_2,
      FLOWER_CHAMOMILE_01_1,
      FLOWER_CHAMOMILE_01_2,
      FLOWER_CHAMOMILE_01_CROSS_1,
      FLOWER_CHAMOMILE_01_DETAILED_1,
      FLOWER_CHAMOMILE_01_DETAILED_2,
      FLOWER_COMMON_CHICORY_01_1,
      FLOWER_COMMON_CHICORY_01_2,
      FLOWER_COMMON_CHICORY_01_CROSS_1,
      FLOWER_COMMON_CHICORY_01_DETAILED_1,
      FLOWER_COMMON_CHICORY_01_DETAILED_2,
      FLOWER_COMMON_POPPY_01_1,
      FLOWER_COMMON_POPPY_01_2,
      FLOWER_COMMON_POPPY_01_CROSS_1,
      FLOWER_COMMON_POPPY_01_DETAILED_1,
      FLOWER_COMMON_POPPY_01_DETAILED_2,
      FLOWER_COMMON_SAINT_JOHNS_WORT_01_1,
      FLOWER_COMMON_SAINT_JOHNS_WORT_01_2,
      FLOWER_COMMON_SAINT_JOHNS_WORT_01_CROSS_1,
      FLOWER_COMMON_SAINT_JOHNS_WORT_01_DETAILED_1,
      FLOWER_CORNFLOWER_01_1,
      FLOWER_CORNFLOWER_01_2,
      FLOWER_CORNFLOWER_01_CROSS_1,
      FLOWER_CORNFLOWER_01_DETAILED_1,
      FLOWER_CORNFLOWER_01_DETAILED_2,
      FLOWER_GOLDENROD_01_1,
      FLOWER_GOLDENROD_01_2,
      FLOWER_GOLDENROD_01_3,
      FLOWER_GOLDENROD_01_CROSS_1,
      FLOWER_GOLDENROD_01_CROSS_2,
      FLOWER_GOLDENROD_01_DETAILED_1,
      FLOWER_GOLDENROD_01_DETAILED_2,
      FLOWER_SUNROOT_01_1,
      FLOWER_SUNROOT_01_2,
      FLOWER_SUNROOT_01_CROSS_1,
      FLOWER_SUNROOT_01_DETAILED_1,
      FLOWER_SUNROOT_01_DETAILED_2,
      GRASS_MEADOW_01_1,
      GRASS_MEADOW_01_2,
      GRASS_MEADOW_01_3,
      GRASS_MEADOW_01_4,
      GRASS_MEADOW_01_5,
      GRASS_MEADOW_01_6,
      GRASS_MEADOW_01_CROSS_1,
      GRASS_MEADOW_01_CROSS_2,
      GRASS_MEADOW_01_CROSS_3,
      GRASS_MEADOW_01_DETAILED_1,
      GRASS_MEADOW_01_DETAILED_2,
      GRASS_MEADOW_02_1,
      GRASS_MEADOW_02_2,
      GRASS_MEADOW_02_3,
      GRASS_MEADOW_02_4,
      GRASS_MEADOW_02_5,
      GRASS_MEADOW_02_6,
      GRASS_MEADOW_02_CROSS_1,
      GRASS_MEADOW_02_CROSS_2,
      GRASS_MEADOW_02_CROSS_3,
      GRASS_MEADOW_02_DETAILED_1,
      GRASS_MEADOW_02_DETAILED_2,
      GRASS_MEADOW_03_1,
      GRASS_MEADOW_03_2,
      GRASS_MEADOW_03_3,
      GRASS_MEADOW_03_4,
      NUM_GRASS_ASSETS
   };

   enum TreeAsset
   {
      POPLAR_PLANT_A_00 = NUM_GRASS_ASSETS,
      POPLAR_PLANT_A_CROSS_00,
      POPLAR_PLANT_B_00,
      POPLAR_PLANT_B_CROSS_00,
      POPLAR_PLANT_C_00,
      POPLAR_PLANT_C_CROSS_00,
      POPLAR_01,
      POPLAR_CROSS_01,
      POPLAR_02,
      POPLAR_CROSS_02,
      POPLAR_CROSS_03,
      POPLAR_SKAN_03,
      POPLAR_CROSS_04,
      POPLAR_SKAN_04,
      POPLAR_CROSS_05,
      //POPLAR_SKAN_05, // Broken
      POPLAR_CROSS_06,
      POPLAR_SKAN_06,
      POPLAR_CROSS_07,
      POPLAR_SKAN_07,
      POPLAR_CROSS_08,
      // POPLAR_SKAN_08, // Broken
      NUM_TREE_ASSETS
   };

   enum RockAsset
   {
      M_ROCK_01 = NUM_TREE_ASSETS,
      M_ROCK_02,
      M_ROCK_03,
      ROCK_01,
      ROCK_01_CUT,
      ROCK_02,
      ROCK_02_CUT,
      ROCK_03,
      ROCK_03_CUT,
      ROCK_04,
      ROCK_05,
      ROCK_05_CUT,
      ROCK_06,
      ROCK_07,
      S_ROCK_01,
      S_ROCK_02,
      S_ROCK_03,
      S_ROCK_04,
      S_ROCK_05,
      S_ROCK_06,
      NUM_ROCK_ASSETS
   };

   enum CliffAsset
   {
      CLIFF_BASE_01 = NUM_ROCK_ASSETS,
      CLIFF_BASE_02,
      CLIFF_BASE_03,
      CLIFF_BASE_04,
      CLIFF_BASE_05,
      CLIFF_BASE_06,
      CLIFF_BASE_07,
      CLIFF_OVERPAINT_01,
      CLIFF_OVERPAINT_02,
      CLIFF_OVERPAINT_03,
      CLIFF_OVERPAINT_04,
      CLIFF_PIECE_01,
      CLIFF_PIECE_02,
      CLIFF_PIECE_03,
      CLIFF_PIECE_04,
      CLIFF_PIECE_05,
      CLIFF_PIECE_06,
      CLIFF_PIECE_07,
      CLIFF_PIECE_08,
      CLIFF_PIECE_09,
      CLIFF_PIECE_10,
      NUM_CLIFF_ASSETS
   };

   enum BushAsset
   {
      GREY_WILLOW_01 = NUM_CLIFF_ASSETS,
      GREY_WILLOW_01_N,
      GREY_WILLOW_02,
      GREY_WILLOW_02_CROSS,
      GREY_WILLOW_02_N,
      GREY_WILLOW_03,
      GREY_WILLOW_03_CROSS,
      GREY_WILLOW_03_N,
      GREY_WILLOW_04,
      GREY_WILLOW_04_CROSS,
      GREY_WILLOW_04_N,
      MAPLE_BUSH_01,
      MAPLE_BUSH_01_CROSS,
      MAPLE_BUSH_02,
      MAPLE_BUSH_02_CROSS,
      MAPLE_BUSH_03,
      MAPLE_BUSH_03_CROSS,
      MAPLE_BUSH_04,
      MAPLE_BUSH_04_CROSS,
      NUM_BUSH_ASSETS
   };

   const uint32_t DELETE_ALL_ASSETS_ID = BushAsset::NUM_BUSH_ASSETS;

   struct Asset
   {
      Asset(uint32_t _id, std::string _model, std::string _texture, std::string _normalMap) {
         id = _id;
         model = _model;
         diffuseTexture = _texture;
         normalMap = _normalMap;
      }

      uint32_t id;
      std::string model;
      std::string diffuseTexture;
      std::string normalMap;
      std::string specularMap;
   };

   class AssetLoader : public Module<AssetLoader>
   {
   public:
      AssetLoader();

      void AddAsset(uint32_t id, std::string model, std::string texture = "-", std::string normalMap = "-");
      SharedPtr<Model> LoadAsset(uint32_t assetId);
      Asset FindAsset(uint32_t id);
      Asset GetAssetByIndex(uint32_t index) const;
      uint32_t GetNumAssets() const;
   private:
      std::vector<Asset> mAssets;

   };

   AssetLoader& gAssetLoader();
}
