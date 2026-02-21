#include "FakeCursor.hpp"
#include "Geode/cocos/support/CCPointExtension.h"

namespace fakecursor {
    static geode::Ref<CCTexture2D> s_texture;
    static GLuint s_textureFilter = GL_LINEAR;
    static CursorTextureInfo s_cursorData;
    static float s_cursorScale = 1.f;
    static float s_offsetX = 0.f;
    static float s_offsetY = 0.f;

    static CursorTextureInfo createTexture() {
        CursorTextureInfo result;

        CURSORINFO ci = { 0 };
        ci.cbSize = sizeof(ci);
        if (!GetCursorInfo(&ci)) return result;

        HCURSOR cursor = ci.hCursor;

        ICONINFO ii = { 0 };
        if (!GetIconInfo(cursor, &ii)) return result;

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

        HDC screenDc = GetDC(nullptr);
        HDC memoryDc = CreateCompatibleDC(screenDc);

        BITMAPINFO bi = { 0 };
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = width;
        bi.bmiHeader.biHeight = height;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP newBitmap = CreateDIBSection(memoryDc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);

        if (newBitmap) {
            HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDc, newBitmap));
            DrawIconEx(memoryDc, 0, 0, cursor, width, height, 0, nullptr, DI_NORMAL);

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

                if (GetDIBits(
                    memoryDc, ii.hbmMask, 0, height, maskPixels.data(), &bi, DIB_RGB_COLORS
                )) {
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

            SelectObject(memoryDc, oldBitmap);
            DeleteObject(newBitmap);
        }

        DeleteDC(memoryDc);
        ReleaseDC(nullptr, screenDc);

        if (ii.hbmColor) DeleteObject(ii.hbmColor);
        if (ii.hbmMask) DeleteObject(ii.hbmMask);

        return result;
    }

    static CursorPos calculateCursorPos(int origW, int origH, int targetW, int targetH) {
        if (s_offsetX == 0 && s_offsetY == 0) {
            s_offsetX = (s_cursorData.width * s_cursorData.anchorX) * s_cursorScale;
            s_offsetY = (s_cursorData.height * (1.0f - s_cursorData.anchorY)) * s_cursorScale;
        }

        POINT p;
        GetCursorPos(&p);
        ScreenToClient(GetForegroundWindow(), &p);

        float scaleX = static_cast<float>(targetW) / static_cast<float>(origW);
        float scaleY = static_cast<float>(targetH) / static_cast<float>(origH);
        float scaledX = static_cast<float>(p.x) * scaleX;
        float scaledY = static_cast<float>(p.y) * scaleY;

        float glY = targetH - scaledY;

        float x = scaledX - s_offsetX;
        float y = glY - s_offsetY;

        return CursorPos{x, y};
    }

    void setScale(float scale) {
        s_cursorScale = scale;
        s_offsetX = 0.f;
        s_offsetY = 0.f;
    }

    void setFilter(std::string const& filter) {
        auto it = filterMap.find(filter);
        if (it != filterMap.end()) {
            s_textureFilter = it->second;
        }

        updateTextureParams();
    }

    void updateTextureParams() {
        if (s_texture == nullptr) return;

        ccTexParams params = {
            s_textureFilter, 
            s_textureFilter, 
            GL_CLAMP_TO_EDGE, 
            GL_CLAMP_TO_EDGE
        };
        
        s_texture->setTexParameters(&params);
    }

    bool init() {
        if (s_cursorData.success && s_texture != nullptr) return true;

        s_cursorData = createTexture();
        if (!s_cursorData.success) return false;

        auto newTexture = new CCTexture2D();
        newTexture->initWithData(
            s_cursorData.pixels.data(), 
            cocos2d::kCCTexture2DPixelFormat_RGBA8888, 
            s_cursorData.width, 
            s_cursorData.height, 
            CCSize(
                static_cast<float>(s_cursorData.width), 
                static_cast<float>(s_cursorData.height)
            )
        );
        newTexture->autorelease();
        s_texture = newTexture;

        updateTextureParams();

        return true;
    }

    void draw(int origW, int origH, int targetW, int targetH) {
        auto cursorPos = calculateCursorPos(origW, origH, targetW, targetH);
        float cursorW = s_cursorData.width * s_cursorScale;
        float cursorH = s_cursorData.height * s_cursorScale;

        // Save current attributes
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        // Save shader program
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(0);

        glDrawBuffer(GL_COLOR_ATTACHMENT0); 

        glViewport(0, 0, targetW, targetH);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, targetW, 0, targetH, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        // Start drawing the texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, s_texture->getName());

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

    void reset() {
        s_texture = nullptr;
        s_cursorData = CursorTextureInfo{};
        s_offsetX = 0.f;
        s_offsetY = 0.f;
    }
}