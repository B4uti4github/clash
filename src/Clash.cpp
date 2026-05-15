#include "Clash.h"
#include "rlgl.h"
#include <cmath>
#include <algorithm>
#include <iostream>
 
namespace Clash {
 
// ============================================================================
// DISPLAYOBJECT
// ============================================================================
 
DisplayObject::~DisplayObject() {
    children.clear();
}
 
void DisplayObject::addChild(std::shared_ptr<DisplayObject> child) {
    if (!child || child.get() == this) return;
 
    auto oldParent = child->parent.lock();
    if (oldParent && oldParent.get() != this)
        oldParent->removeChild(child);
 
    try {
        child->parent = shared_from_this();
    } catch (const std::bad_weak_ptr&) {
        // El padre no fue creado con make_shared, no se puede asignar parent
    }
 
    children.push_back(child);
}
 
void DisplayObject::removeChild(std::shared_ptr<DisplayObject> child) {
    if (!child) return;
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        child->parent.reset();
        children.erase(it);
    }
}
 
void DisplayObject::removeFromParent() {
    auto p = parent.lock();
    if (p) p->removeChild(shared_from_this());
}
 
void DisplayObject::removeAllChildren() {
    for (auto& child : children) child->parent.reset();
    children.clear();
}
 
std::shared_ptr<DisplayObject> DisplayObject::findChildByName(const std::string& searchName) {
    // Busqueda recursiva (consistente con findChildrenByName)
    for (auto& child : children) {
        if (child->name == searchName) return child;
        auto found = child->findChildByName(searchName);
        if (found) return found;
    }
    return nullptr;
}
 
std::vector<std::shared_ptr<DisplayObject>> DisplayObject::findChildrenByName(const std::string& searchName) {
    std::vector<std::shared_ptr<DisplayObject>> results;
 
    std::function<void(DisplayObject*)> search = [&](DisplayObject* node) {
        for (auto& child : node->children) {
            if (child->name == searchName) results.push_back(child);
            search(child.get());
        }
    };
 
    search(this);
    return results;
}
 
void DisplayObject::render() {
    if (!visible) return;
 
    rlPushMatrix();
 
    // Traslacion
    rlTranslatef(x, y, z);
 
    // Rotaciones Euler
    if (rotationX != 0.0f) rlRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    if (rotationY != 0.0f) rlRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    if (rotationZ != 0.0f) rlRotatef(rotationZ, 0.0f, 0.0f, 1.0f);
 
    // Skew
    if (skewX != 0.0f || skewY != 0.0f || skewZX != 0.0f || skewZY != 0.0f) {
        float radX  = skewX  * DEG2RAD;
        float radY  = skewY  * DEG2RAD;
        float radZX = skewZX * DEG2RAD;
        float radZY = skewZY * DEG2RAD;
 
        Matrix skewMatrix = {
            1.0f,        tanf(radX),  0.0f,  0.0f,
            tanf(radY),  1.0f,        0.0f,  0.0f,
            tanf(radZX), tanf(radZY), 1.0f,  0.0f,
            0.0f,        0.0f,        0.0f,  1.0f
        };
        rlMultMatrixf(&skewMatrix.m0);
    }
 
    // Escalado
    rlScalef(scaleX, scaleY, scaleZ);
 
    // CORREGIDO: eliminado el BLACK_TEXTURE que no existe en Raylib.
    // El alpha por objeto se maneja pasando Color con alpha en cada draw callback,
    // por ejemplo: DrawRectangle(0, 0, 100, 100, Fade(RED, alpha));
 
    // Callback de dibujo
    if (draw) draw();
 
    // Hijos
    for (auto& child : children)
        child->render();
 
    rlPopMatrix();
}
 
// ============================================================================
// CONTEXT
// ============================================================================
 
Context::Context(const std::string& ctxName, bool use3D)
    : name(ctxName), is3D(use3D)
{
    stage = createDisplayObject<DisplayObject>();
    stage->name = "stage";
 
    camera3D.position   = { 0.0f, 10.0f, 10.0f };
    camera3D.target     = { 0.0f, 0.0f,  0.0f  };
    camera3D.up         = { 0.0f, 1.0f,  0.0f  };
    camera3D.fovy       = fov;
    camera3D.projection = CAMERA_PERSPECTIVE;
 
    camera2D.offset   = { 0.0f, 0.0f };
    camera2D.target   = { 0.0f, 0.0f };
    camera2D.rotation = 0.0f;
    camera2D.zoom     = 1.0f;
}
 
std::shared_ptr<DisplayObject> Context::createAndAddToStage() {
    auto obj = create<DisplayObject>();
    stage->addChild(obj);
    return obj;
}
 
// ============================================================================
// CLASHMASTERCONTEXT
// ============================================================================
 
ClashMasterContext::ClashMasterContext(int w, int h, const std::string& t)
    : width(w), height(h), title(t)
{
    InitWindow(width, height, title.c_str());
    SetTargetFPS(60);
}
 
ClashMasterContext::~ClashMasterContext() {
    for (auto& ctx : contexts) {
        if (ctx) {
            ctx->setActive(false);
            if (auto s = ctx->getStage()) s->removeAllChildren();
        }
    }
    contexts.clear();
 
    if (IsWindowReady()) CloseWindow();
}
 
void ClashMasterContext::addContext(std::shared_ptr<Context> ctx) {
    if (ctx) contexts.push_back(ctx);
}
 
void ClashMasterContext::removeContext(const std::string& ctxName) {
    contexts.erase(
        std::remove_if(contexts.begin(), contexts.end(),
            [&ctxName](const std::shared_ptr<Context>& c) {
                return c->getName() == ctxName;
            }),
        contexts.end()
    );
}
 
std::shared_ptr<Context> ClashMasterContext::getContext(const std::string& ctxName) {
    for (auto& ctx : contexts)
        if (ctx->getName() == ctxName) return ctx;
    return nullptr;
}
 
void ClashMasterContext::run(std::function<void(ClashMasterContext&, float)> userUpdateCallback) {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
 
        if (userUpdateCallback)
            userUpdateCallback(*this, dt);
 
        BeginDrawing();
        ClearBackground(DARKGRAY);
 
        for (auto& ctx : contexts) {
            if (!ctx || !ctx->isActive()) continue;
 
            if (ctx->isContext3D()) {
                ctx->syncCamera3D();
                BeginMode3D(ctx->getCamera3D());
                    ctx->getStage()->render();
                EndMode3D();
            } else {
                ctx->syncCamera2D();
                BeginMode2D(ctx->getCamera2D());
                    ctx->getStage()->render();
                EndMode2D();
            }
        }
 
        EndDrawing();
    }
}
 
} // namespace Clash