#include "CNA/Internal/Graphics/ImageLoader.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <string>

namespace CNA::Internal::Graphics
{
    static ImageData surfaceToImageData(SDL_Surface* surface, const std::string& label)
    {
        // SDL2's SDL_ConvertSurfaceFormat takes the pixel-format enum value directly (matching
        // SDL3's SDL_ConvertSurface signature); SDL2's own SDL_ConvertSurface instead takes a
        // full SDL_PixelFormat* struct, which isn't what this call needs.
        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        if (!converted)
        {
            SDL_FreeSurface(surface);
            throw std::runtime_error("Failed to convert image to RGBA: " + label);
        }

        ImageData data;
        data.width  = converted->w;
        data.height = converted->h;
        std::size_t sz = static_cast<std::size_t>(data.width * data.height * 4);
        data.pixels.assign(
            static_cast<uint8_t*>(converted->pixels),
            static_cast<uint8_t*>(converted->pixels) + sz);

        // SpriteBatch uses XNA's AlphaBlend state, whose source colour factor is
        // One and therefore expects premultiplied-alpha textures.  PNGs decoded
        // by SDL_image are straight RGBA.  Leaving them straight made fully
        // transparent white pixels in the pad atlas add white to the frame on
        // Android's GLES renderer, producing solid pale rectangles around the
        // touch controls.  Convert decoded image assets once at the boundary.
        for (std::size_t pixel = 0; pixel < sz; pixel += 4)
        {
            const uint8_t alpha = data.pixels[pixel + 3];
            data.pixels[pixel + 0] = static_cast<uint8_t>(
                (static_cast<unsigned int>(data.pixels[pixel + 0]) * alpha + 127u) / 255u);
            data.pixels[pixel + 1] = static_cast<uint8_t>(
                (static_cast<unsigned int>(data.pixels[pixel + 1]) * alpha + 127u) / 255u);
            data.pixels[pixel + 2] = static_cast<uint8_t>(
                (static_cast<unsigned int>(data.pixels[pixel + 2]) * alpha + 127u) / 255u);
        }

        SDL_FreeSurface(converted);
        SDL_FreeSurface(surface);
        return data;
    }

    ImageData ImageLoader::Load(const std::string& assetName)
    {
        SDL_Surface* surface = IMG_Load(assetName.c_str());
        if (!surface)
            throw std::runtime_error("Failed to load image: " + assetName + " - " + SDL_GetError());
        return surfaceToImageData(surface, assetName);
    }

    ImageData ImageLoader::LoadFromMemory(const uint8_t* data, std::size_t size)
    {
        SDL_RWops* io = SDL_RWFromConstMem(data, static_cast<int>(size));
        if (!io)
            throw std::runtime_error(std::string("SDL_RWFromConstMem failed: ") + SDL_GetError());

        SDL_Surface* surface = IMG_Load_RW(io, 1);
        if (!surface)
            throw std::runtime_error(std::string("IMG_Load_RW failed: ") + SDL_GetError());

        return surfaceToImageData(surface, "<memory>");
    }
}
