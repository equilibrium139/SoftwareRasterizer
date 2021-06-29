#include "Texture.h"
#include <SDL_image.h> 

std::optional<Texture> textureFromFile(const char* path)
{
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        std::cerr << "Unable to load image " << path << ". \nSDL_image error: " << IMG_GetError() << '\n';
        return std::nullopt;
    }
    SDL_Surface* argbSurface = surface;
    if (surface->format->format != SDL_PIXELFORMAT_ARGB8888) {
        argbSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
        SDL_FreeSurface(surface);
    }
    Texture texture((Color*)argbSurface->pixels, argbSurface->w, argbSurface->h);
    SDL_FreeSurface(argbSurface);
    return texture;
}