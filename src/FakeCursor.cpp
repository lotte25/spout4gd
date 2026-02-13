#include "FakeCursor.hpp"
#include <cstddef>
#include <winuser.h>

namespace FakeCursor {
    static GLuint textureID = 0;
    static CursorTextureInfo cursorData;
    static float cursorScale = 1.f;
    static float offsetX = 0.f;
    static float offsetY = 0.f;

    CursorTextureInfo CreateTexture() {
        CursorTextureInfo result;

        CURSORINFO ci = { 0 };
        ci.cbSize = sizeof(ci);
        if (!GetCursorInfo(&ci)) return result;

        HCURSOR hCursor = ci.hCursor;

        ICONINFO ii = { 0 };
        if (!GetIconInfo(hCursor, &ii)) return result;

        BITMAP bmp = { 0 };
        if (ii.hbmColor) {
            GetObject(ii.hbmColor, sizeof(bmp), &bmp);
        } else {
            GetObject(ii.hbmMask, sizeof(bmp), &bmp);
            bmp.bmHeight /= 2;
        }

        int width = bmp.bmWidth;
        int height = bmp.bmHeight;

        result.anchorX = static_cast<float>(ii.xHotspot) / static_cast<float>(width);
        result.anchorY = static_cast<float>(ii.yHotspot) / static_cast<float>(height); 
        result.width = width;
        result.height = height;

        HDC scrdc = GetDC(NULL);
        HDC memdc = CreateCompatibleDC(scrdc);

        BITMAPINFO bi = { 0 };
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = width;
        bi.bmiHeader.biHeight = height;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP hNewBitmap = CreateDIBSection(memdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);

        if (hNewBitmap) {
            HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(memdc, hNewBitmap));
            DrawIconEx(memdc, 0, 0, hCursor, width, height, 0, NULL, DI_NORMAL);

            size_t pixelCount = width * height;

            result.pixels.resize(pixelCount * 4);
            uint8_t* srcPixels = static_cast<uint8_t*>(bits);
            uint8_t* dstPixels = result.pixels.data();

            bool foundAlpha = false;

            for (int i = 0; i < width * height; ++i) {
                size_t idx = i * 4;

                uint8_t b = srcPixels[idx + 0];
                uint8_t g = srcPixels[idx + 1];
                uint8_t r = srcPixels[idx + 2];
                uint8_t a = srcPixels[idx + 3];
                
                if (a != 0) {
                    foundAlpha = true;
                }

                dstPixels[idx + 0] = r;
                dstPixels[idx + 1] = g;
                dstPixels[idx + 2] = b;
                dstPixels[idx + 3] = a;
            }

            if (!foundAlpha) {
                std::vector<uint32_t> maskPixels(pixelCount);

                if (GetDIBits(memdc, ii.hbmMask, 0, height, maskPixels.data(), &bi, DIB_RGB_COLORS)) {
                    for (size_t i = 0; i < pixelCount; ++i) {
                        bool isTransparent = (maskPixels[i] & 0xFF) != 0;
                        
                        if (!isTransparent) {
                            dstPixels[i*4 + 3] = 255;
                        } else {
                            dstPixels[i*4 + 3] = 0;
                        }
                    }
                }
            }

            result.success = true;

            SelectObject(memdc, hOldBitmap);
            DeleteObject(hNewBitmap);
        }

        DeleteDC(memdc);
        ReleaseDC(NULL, scrdc);

        if (ii.hbmColor) DeleteObject(ii.hbmColor);
        if (ii.hbmMask) DeleteObject(ii.hbmMask);

        return result;
    }

    CursorPos CalculateCursorPos(int h) {
        if (offsetX == 0 && offsetY == 0) {
            offsetX = (cursorData.width * cursorData.anchorX) * cursorScale;
            offsetY = (cursorData.height * (1.0f - cursorData.anchorY)) * cursorScale;
        }

        POINT p;
        GetCursorPos(&p);
        ScreenToClient(GetForegroundWindow(), &p);

        float glY = h - p.y;

        float x = static_cast<float>(p.x) - offsetX;
        float y = glY - offsetY;

        return CursorPos{x, y};
    }

    void setScale(float scale) {
        cursorScale = scale;
        offsetX = 0.f;
        offsetY = 0.f;
    }

    bool init() {
        if (cursorData.success && textureID != 0) return true;

        cursorData = CreateTexture();
        if (!cursorData.success) return false;

        auto texture = new CCTexture2D();
        texture->initWithData(
            cursorData.pixels.data(), 
            cocos2d::kCCTexture2DPixelFormat_RGBA8888, 
            cursorData.width, 
            cursorData.height, 
            CCSize(cursorData.width, cursorData.height)
        );
        GLuint defaultFilter = GL_NEAREST;
        ccTexParams params = {
            defaultFilter, 
            defaultFilter, 
            GL_CLAMP_TO_EDGE, 
            GL_CLAMP_TO_EDGE
        };
        texture->setTexParameters(&params);
 
        textureID = texture->getName();

        return true;
    }

    void draw(int w, int h) {
        auto cursorPos = CalculateCursorPos(h);
        float cursorW = cursorData.width * cursorScale;
        float cursorH = cursorData.height * cursorScale;

        // Save current attributes
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        // Save shader program
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0); 

        glViewport(0, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, w, 0, h, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        // Start drawing the texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); 
            glVertex2f(cursorPos.x, cursorPos.y); 

            glTexCoord2f(1.0f, 0.0f); 
            glVertex2f(cursorPos.x + cursorW, cursorPos.y);

            glTexCoord2f(1.0f, 1.0f); 
            glVertex2f(cursorPos.x + cursorW, cursorPos.y + cursorH);

            glTexCoord2f(0.0f, 1.0f); 
            glVertex2f(cursorPos.x, cursorPos.y + cursorH);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        // Restore matrix stack
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        // Restore all previous attributes
        glPopAttrib();
        // Restore shader program
        glUseProgram(oldProgram);
    }
}