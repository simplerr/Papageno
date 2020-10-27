#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vulkan/RenderTarget.h>
#include <vulkan/handles/Semaphore.h>
#include "vulkan/VulkanPrerequisites.h"
#include "vulkan/VulkanApp.h"
#include "vulkan/ShaderBuffer.h"
#include "utility/Common.h"

using namespace Utopian;

class MiniCamera;
class Block;

struct BlockKey
{
	BlockKey(int32_t _x, int32_t _y, int32_t _z)
		: x(_x), y(_y), z(_z) {

	}

	int32_t x, y, z;
};

bool operator<(BlockKey const& a, BlockKey const& b);

/**
 * The concept from Nvidias article (http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html) is to generate a terrain mesh for each block
 * and write it to a vertex buffer that can be reused as long as the block is visible. Blocks are only generated when needed, i.e when they get visible.
 *
 * Sources:
 * http://paulbourke.net/geometry/polygonise/
 * http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html
 * http://www.icare3d.org/codes-and-projects/codes/opengl_geometry_shader_marching_cubes.html
 * https://0fps.net/2012/07/12/smooth-voxel-terrain-part-2/
 */
class MarchingCubes
{
public:
	UNIFORM_BLOCK_BEGIN(MarchingInputParameters)
		UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_PARAM(glm::vec4, offsets[8])
		UNIFORM_PARAM(glm::vec4, color)
		UNIFORM_PARAM(float, voxelSize)
		UNIFORM_PARAM(uint32_t, viewDistance)
		UNIFORM_PARAM(uint32_t, voxelsInBlock)
		UNIFORM_PARAM(float, time)
		UNIFORM_PARAM(uint32_t, flatNormals)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(CounterSSBO)
		UNIFORM_PARAM(uint32_t, numVertices)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(TerrainInputParameters)
		UNIFORM_PARAM(glm::mat4, projection)
		UNIFORM_PARAM(glm::mat4, view)
		UNIFORM_PARAM(glm::vec4, clippingPlane)
		UNIFORM_PARAM(glm::vec3, eyePos)
		UNIFORM_PARAM(float, pad)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(TerrainSettings)
		UNIFORM_PARAM(int, mode) // 0 = phong, 1 = normals, 2 = block cells
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(BrushInputParameters)
		UNIFORM_PARAM(glm::ivec3, startCoord)
		UNIFORM_PARAM(float, brushSize)
		UNIFORM_PARAM(float, brushStrength)
		UNIFORM_PARAM(int, mode) // 0 = add, 1 = subtract
		UNIFORM_PARAM(int, textureRegionSize)
	UNIFORM_BLOCK_END()

	// Intersection
	UNIFORM_BLOCK_BEGIN(IntersectionOutputSSBO)
		UNIFORM_PARAM(glm::vec3, brushPos)
	UNIFORM_BLOCK_END()

	UNIFORM_BLOCK_BEGIN(IntersectionInputUBO)
		UNIFORM_PARAM(glm::ivec2, mousePosition)
	UNIFORM_BLOCK_END()

	MarchingCubes(Utopian::Window* window);
	~MarchingCubes();

	void Run();

	void DestroyCallback();
	void UpdateCallback();
	void DrawCallback();

	/** Adds the blocks within the viewing distance range. */
	void UpdateBlockList();

	/** Generates the vertex buffer for newly added or modified blocks. */
	void GenerateBlocks();
	void RenderBlocks();

	void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void InitResources();
	void InitNoiseTextureEffect(Vk::Device* device);
	void InitBrushEffect(Vk::Device* device);
	void InitMarchingCubesEffect(Vk::Device* device, uint32_t width, uint32_t height);
	void InitTerrainEffect(Vk::Device* device, uint32_t width, uint32_t height);
	void InitIntersectionEffect(Vk::Device* device, uint32_t width, uint32_t height);
	void GenerateNoiseTexture();
	void ApplyTerrainBrush();
	void ActivateBlockRegeneration();
	void QueryBrushPosition();
	glm::ivec3 GetBlockCoordinate(glm::vec3 position);

	Vk::VulkanApp* mVulkanApp;
	Utopian::Window* mWindow;
	SharedPtr<MiniCamera> mCamera;
	const glm::vec3 mOrigin = glm::vec3(256000.0f);

	// Marching cubes
	std::map<BlockKey, Block*> mBlockList;
	SharedPtr<Vk::Effect> mMarchingCubesEffect;
	SharedPtr<Utopian::Vk::Texture> mEdgeTableTexture;
	SharedPtr<Utopian::Vk::Texture> mTriangleTableTexture;
	MarchingInputParameters mMarchingInputParameters;
	CounterSSBO mCounterSSBO;
	const int32_t mVoxelsInBlock = 32;
	const int32_t mVoxelSize = 10;
	const int32_t mViewDistance = 4;

	// Terrain
	SharedPtr<Vk::RenderTarget> mTerrainRenderTarget;
	SharedPtr<Vk::Image> mTerrainColorImage;
	SharedPtr<Vk::Image> mTerrainPositionImage;
	SharedPtr<Vk::Image> mTerrainDepthImage;
	SharedPtr<Vk::Effect> mTerrainEffect;
	SharedPtr<Vk::Effect> mTerrainEffectWireframe;
	SharedPtr<Vk::Semaphore> mTerrainCompletedSemaphore;
	SharedPtr<Vk::CommandBuffer> mTerrainCommandBuffer;
	TerrainInputParameters mTerrainInputParameters;
	TerrainSettings mTerrainSettings;

	// Noise rendering
	SharedPtr<Vk::Effect> mNoiseEffect;
	SharedPtr<Vk::Image> mSdfImage;
	SharedPtr<Vk::Sampler> mNoiseSampler;
	const uint32_t mNoiseTextureSize = 256;

	// Terrain brush
	SharedPtr<Vk::Effect> mBrushEffect;
	BrushInputParameters mBrushInputParameters;
	const uint32_t mBrushTextureRegion = 32;

	// Intersection
	SharedPtr<Vk::Effect> mIntersectionEffect;
	IntersectionOutputSSBO mIntersectionOutputSSBO;
	IntersectionInputUBO mIntersectionInputUBO;
	glm::vec3 mBrushPos;

	// Settings
	bool mStaticPosition = true;
	bool mWireframe = false;
};
