﻿
#ifndef __TEXTURE_H__
#define __TEXTURE_H__
//File ảnh để vẽ lên
#include "define.h"
#include "Singleton\dxdevice.h"
#include "Viewport.h"

DEFINE_FRAMEWORK_START

class Texture
{
public:
	Texture(void);
	~Texture(void);

	// release the image from memory
	void release();

	/*
	load image from file and store in LPDIRECT3DTEXTURE9
	@spriteHandle: sprite handle managed by directX, it work for load content.
	@filePath: the path of file
	@color: the color is removed when rendered
	*/
	HRESULT loadFromFile(LPD3DXSPRITE spriteHandle, LPWSTR filePath, D3DXCOLOR color = C_WHITE, D3DXCOLOR colorkey = COLOR_KEY);

	/*
	draw LPDIRECT3DTEXTURE9 to screen, directX manage draw to buffer, we dont care about buffer anymore.
	@spriteHandle: sprite handle managed by directX, it work for load content.
	@rect: a part of image want to draw, remmember RECT is defined: {top, left, right, bot} (4 lines)
	@center: a center of frame to draw:
	- if you chose NULL, topleft of image will be drawn at @position
	- if you chose x,y of frame, topleft of image will be drawn at @position - (x ,y)
	@postion: position to draw
	*/
	void render(LPD3DXSPRITE spriteHandle, const RECT* rect, const GVector3* center, const GVector3* position);

	/*
	render texture với các tham số tranform, gốc tọa độ top-left
	@position: vị trí
	@scale:
	@rotate: góc xoay theo số độ (0-360)
	@origin: điểm neo để transform
	@zIndex: độ sâu
	@translation: tịnh tiến
	*/
	void render(LPD3DXSPRITE spriteHandle, RECT* srcRect, GVector2 position, GVector2 scale, float rotate, GVector2 origin, float zIndex = 0, GVector2 translation = VECTOR2ZERO);

	/*
	render texture với các tham số tranform, trong viewport
	@viewport:
	@position: vị trí
	@scale:
	@rotate: góc xoay theo số độ (0-360)
	@origin: điểm neo để transform
	@zIndex: độ sâu
	@translation: tịnh tiến
	*/
	void render(LPD3DXSPRITE spriteHandle, RECT* srcRect, Viewport viewport, GVector2 position, GVector2 scale, float rotate, GVector2 origin, float zIndex = 0, GVector2 translation = VECTOR2ZERO);

	void setColor(D3DXCOLOR);
	D3DXCOLOR getColor();

	// get width of image
	int getWidth();
	//get height of image
	int getHeight();
private:
	LPDIRECT3DTEXTURE9	_texture;		// contain the image laoded from file
	D3DXIMAGE_INFO		_imageInfo;		// contain info such as width, height, extention of file...

	D3DXCOLOR			_color;			// usually, image file have back color, it is ignore when draw. We use PINK
};

typedef Texture* pTexture;

DEFINE_FRAMEWORK_END

#endif // !__TEXTURE_H__

