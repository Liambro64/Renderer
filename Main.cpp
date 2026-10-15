#include "Project.hpp"


int main()
{
	Vector3 position = {0, 0, 0};
	Quaternion rotation = {1, 0, 0, 0};
	float FOVradians = 90.0f;

	Vector2 screenSize = {800, 600};

	Rasterizer basicRasterizer = Rasterizer();
	WireframeRasterizer wireframeRasterizer = WireframeRasterizer();
	FilledWireframeRasterizer filledWireframeRasterizer = FilledWireframeRasterizer();

	Object cube({ //verticies
		{-1.0f, -1.0f,  1.0f}, // 0: Front-Bottom-Left
    	{ 1.0f, -1.0f,  1.0f}, // 1: Front-Bottom-Right
    	{ 1.0f,  1.0f,  1.0f}, // 2: Front-Top-Right
    	{-1.0f,  1.0f,  1.0f}, // 3: Front-Top-Left
    	{-1.0f, -1.0f, -1.0f}, // 4: Back-Bottom-Left
    	{ 1.0f, -1.0f, -1.0f}, // 5: Back-Bottom-Right
    	{ 1.0f,  1.0f, -1.0f}, // 6: Back-Top-Right
    	{-1.0f,  1.0f, -1.0f}  // 7: Back-Top-Left
	}, 
	{ //triangles
		{0, 1, 2}, {0, 2, 3}, // Right Face (x = 1)
    	{1, 5, 6}, {1, 6, 2}, // Back Face (z = 0)
    	{5, 4, 7}, {5, 7, 6}, // Left Face (x = 0)
    	{4, 0, 3}, {4, 3, 7}, // Top Face (y = 1)
    	{3, 2, 6}, {3, 6, 7}, // Bottom Face (y = 0)
    	{4, 5, 1}, {4, 1, 0}  // Front Face (z = 1)
	}, {0, 0, -5}, {1, 0, 0, 0}, {1, 1, 1}, 0x0000FF00, 0x00000000, &basicRasterizer);

	Object cube2({ //verticies
		{-1.0f, -1.0f,  1.0f}, // 0: Front-Bottom-Left
    	{ 1.0f, -1.0f,  1.0f}, // 1: Front-Bottom-Right
    	{ 1.0f,  1.0f,  1.0f}, // 2: Front-Top-Right
    	{-1.0f,  1.0f,  1.0f}, // 3: Front-Top-Left
    	{-1.0f, -1.0f, -1.0f}, // 4: Back-Bottom-Left
    	{ 1.0f, -1.0f, -1.0f}, // 5: Back-Bottom-Right
    	{ 1.0f,  1.0f, -1.0f}, // 6: Back-Top-Right
    	{-1.0f,  1.0f, -1.0f}  // 7: Back-Top-Left
	}, 
	{ //triangles
		{0, 1, 2}, {0, 2, 3}, // Right Face (x = 1)
    	{1, 5, 6}, {1, 6, 2}, // Back Face (z = 0)
    	{5, 4, 7}, {5, 7, 6}, // Left Face (x = 0)
    	{4, 0, 3}, {4, 3, 7}, // Top Face (y = 1)
    	{3, 2, 6}, {3, 6, 7}, // Bottom Face (y = 0)
    	{4, 5, 1}, {4, 1, 0}  // Front Face (z = 1)
	}, {0, 2.5, -5}, {1, 0, 0, 0}, {1, 1, 1}, 0x00FF0000, 0x00000000, &wireframeRasterizer); 

	Object cube3({ //verticies
		{-1.0f, -1.0f,  1.0f}, // 0: Front-Bottom-Left
    	{ 1.0f, -1.0f,  1.0f}, // 1: Front-Bottom-Right
    	{ 1.0f,  1.0f,  1.0f}, // 2: Front-Top-Right
    	{-1.0f,  1.0f,  1.0f}, // 3: Front-Top-Left
    	{-1.0f, -1.0f, -1.0f}, // 4: Back-Bottom-Left
    	{ 1.0f, -1.0f, -1.0f}, // 5: Back-Bottom-Right
    	{ 1.0f,  1.0f, -1.0f}, // 6: Back-Top-Right
    	{-1.0f,  1.0f, -1.0f}  // 7: Back-Top-Left
	}, 
	{ //triangles
		{0, 1, 2}, {0, 2, 3}, // Right Face (x = 1)
    	{1, 5, 6}, {1, 6, 2}, // Back Face (z = 0)
    	{5, 4, 7}, {5, 7, 6}, // Left Face (x = 0)
    	{4, 0, 3}, {4, 3, 7}, // Top Face (y = 1)
    	{3, 2, 6}, {3, 6, 7}, // Bottom Face (y = 0)
    	{4, 5, 1}, {4, 1, 0}  // Front Face (z = 1)
	}, {0, -2.5, -5}, {1, 0, 0, 0}, {1, 1, 1}, 0x000000FF, 0x00000000, &filledWireframeRasterizer); 
	Renderer renderer(position, rotation, {}, FOVradians, screenSize);

	renderer.addObject(cube);
	renderer.addObject(cube2);
	renderer.addObject(cube3);

	WindowManager windowManager(screenSize.x, screenSize.y, renderer);

	while (windowManager.running) {
	}

	return 0;
}
