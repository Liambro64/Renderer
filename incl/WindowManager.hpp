#pragma once
#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include "../Project.hpp"



class WindowManager

{
private:
	Display *display;
	Visual *visual;
	int depth;
	Window root;
	Window window;
	GC gc;
	int width;
	int height;
	int screen;
	std::thread thread;
	Renderer *m_renderer;

	bool windowSizeChanged;

	bool lookingUp;
	bool lookingDown;
	bool lookingLeft;
	bool lookingRight;

	bool movingForward;
	bool movingBackward;
	bool movingLeft;
	bool movingRight;

public:
	std::atomic_bool running;

	WindowManager(int w, int h, Renderer& renderer);
	~WindowManager();

	const std::vector<uint32_t>& Render();
	void ShowBuffer(const std::vector<uint32_t>& buffer);
	void ProcessEvents();
	void Update();

	void Stop();
	void FillPixelBuffer(uint32_t color);
	void FillArea(int x, int y, int w, int h, uint32_t color);
	void drawLine(Vector2 start, Vector2 end, uint32_t color = 0x00FFFFFF);
	void drawTriangle(Vector2 a, Vector2 b, Vector2 c, uint32_t color = 0x00FF0000);
	void drawTriangleEdges(Vector2 a, Vector2 b, Vector2 c, uint32_t color = 0x00FFFFFF, float threshold = 1.0f);
	void drawTriangleWithEdges(Vector2 a, Vector2 b, Vector2 c, uint32_t fillColor = 0x00FFFFFF, uint32_t edgeColor = 0x00FF0000, float threshold = 1.0f);
};

#endif // WINDOWMANAGER_H