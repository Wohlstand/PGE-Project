/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2015 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scene.h"
#include <QString>
#include <graphics/window.h>
#include <graphics/gl_renderer.h>

#include <script/lua_event.h>
#include <script/bindings/core/events/luaevents_core_engine.h>

void Scene::construct()
{
    fader.setFull();
    fader.setFade(10, 0.0f, 0.02f); //!< Fade in scene when it was started
    running=true;
    doExit=false;
    _doShutDown=false;
    dif = 0;
    updateTickValue();    
}

void Scene::updateTickValue()
{
    uTickf = PGE_Window::TimeOfFrame;//1000.0f/(float)PGE_Window::TicksPerSecond;
    uTick = round(uTickf);
    if(uTick<=0) uTick=1;
    if(uTickf<=0) uTickf=1.0;
}

Scene::Scene()
{
    sceneType = _Unknown;
    construct();
}

Scene::Scene(TypeOfScene _type)
{
    sceneType = _type;
    construct();
}

Scene::~Scene()
{
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Reset modelview matrix
    glLoadIdentity();
}

void Scene::onKeyInput(int)
{}

void Scene::onKeyboardPressed(SDL_Scancode)
{}

void Scene::onKeyboardPressedSDL(SDL_Keycode, Uint16)
{}

void Scene::onKeyboardReleased(SDL_Scancode)
{}

void Scene::onKeyboardReleasedSDL(SDL_Keycode, Uint16)
{}

void Scene::onMouseMoved(SDL_MouseMotionEvent &)
{}

void Scene::onMousePressed(SDL_MouseButtonEvent &)
{}

void Scene::onMouseReleased(SDL_MouseButtonEvent &)
{}

void Scene::onMouseWheel(SDL_MouseWheelEvent &)
{}


void Scene::processEvents()
{
    SDL_Event event; //  Events of SDL
    while ( SDL_PollEvent(&event) )
    {
        if(PGE_Window::processEvents(event)!=0) continue;
        switch(event.type)
        {
            case SDL_QUIT:
                {
                    doExit          = true;
                    running         = false;
                    _doShutDown = true;
                    break;
                }// End work of program
            break;
            case SDL_KEYDOWN: // If pressed key
                onKeyboardPressedSDL(event.key.keysym.sym, event.key.keysym.mod);
                onKeyboardPressed(event.key.keysym.scancode);
            break;
            case SDL_KEYUP: // If released key
                onKeyboardReleasedSDL(event.key.keysym.sym, event.key.keysym.mod);
                onKeyboardReleased(event.key.keysym.scancode);
            break;
            case SDL_MOUSEBUTTONDOWN:
                onMousePressed(event.button);
            break;
            case SDL_MOUSEBUTTONUP:
                onMouseReleased(event.button);
            break;
            case SDL_MOUSEWHEEL:
                onMouseWheel(event.wheel);
            break;
            case SDL_MOUSEMOTION:
                onMouseMoved(event.motion);
            break;
        }
    }
}

LuaEngine *Scene::getLuaEngine()
{
    return nullptr;
}

void Scene::update()
{
    fader.tickFader(uTickf);
}

void Scene::updateLua()
{
    LuaEngine* sceneLuaEngine = getLuaEngine();
    clearRenderFunctions();//Clean up last rendered stuff
    if(sceneLuaEngine)
    {
        if(sceneLuaEngine->isValid() && !sceneLuaEngine->shouldShutdown()){
            //sceneLuaEngine->runGarbageCollector();
            LuaEvent loopEvent = BindingCore_Events_Engine::createLoopEvent(sceneLuaEngine, uTickf);
            sceneLuaEngine->dispatchEvent(loopEvent);
        }
    }
}

void Scene::render()
{
    if(!fader.isNull())
    {
        GlRenderer::renderRect(0, 0, PGE_Window::Width, PGE_Window::Height, 0.f, 0.f, 0.f, fader.fadeRatio());
    }

    const int sz = renderFunctions.size();
    const std::function<void()>* fn = renderFunctions.data();
    for(int i=0;i<sz; i++){//Call all render functions
        (fn[i])();
    }
}

void Scene::renderMouse()
{}

int Scene::exec()
{
    return 0;
}

Scene::TypeOfScene Scene::type()
{
    return sceneType;
}

void Scene::addRenderFunction(const std::function<void ()> &renderFunc)
{
    renderFunctions.push_back(renderFunc);
}

void Scene::clearRenderFunctions()
{
    renderFunctions.clear();
}

bool Scene::isVizibleOnScreen(PGE_RectF &rect)
{
    PGE_RectF screen(0, 0, PGE_Window::Width, PGE_Window::Height);
    return screen.collideRect(rect);
}

bool Scene::isVizibleOnScreen(double x, double y, double w, double h)
{
    PGE_RectF screen(0, 0, PGE_Window::Width, PGE_Window::Height);
    return screen.collideRect(x, y, w, h);
}

bool Scene::isExiting()
{
    return doExit;
}

bool Scene::doShutDown()
{
    return _doShutDown;
}

/**************************Fader*******************************/
bool Scene::isOpacityFadding()
{
    return fader.isFading();
}

void Scene::setFade(int speed, float target, float step)
{
    fader.setFade(speed, target, step);
}
/**************************Fader**end**************************/

/************waiting timer************/
void Scene::wait(float ms)
{
    if(floor(ms)<=0.0f) return;
    float totalDelay = floorf(ms+dif);
    //StTimePt delayed=StClock::now();//for accuracy
    //std::this_thread::sleep_for(std::chrono::milliseconds((long)(totalDelay)));
    //if(totalDelay>0.0f)
    SDL_Delay((Uint32)totalDelay);
    //StTimePt isnow=StClock::now();
    //printf("%f %f\n", totalDelay, (float)(std::chrono::duration_cast<std::chrono::nanoseconds>(isnow-delayed).count()/1000000.0f));
    //fflush(stdout);
    //dif = (ms+dif)-totalDelay;//-(float)(std::chrono::duration_cast<std::chrono::nanoseconds>(isnow-delayed).count()/1000000.0f);
}
/************waiting timer************/


QString Scene::errorString()
{
    return _errorString;
}
