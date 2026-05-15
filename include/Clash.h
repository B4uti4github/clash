#ifndef CLASH_H
#define CLASH_H

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "raylib.h"

namespace Clash {

class DisplayObject : public std::enable_shared_from_this<DisplayObject> {
public:
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
    float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
    float skewX = 0.0f, skewY = 0.0f, skewZX = 0.0f, skewZY = 0.0f;

    std::weak_ptr<DisplayObject> parent;
    std::vector<std::shared_ptr<DisplayObject>> children;

    // CORRECTO: weak_ptr en el draw para evitar ciclos de retencion.
    // Si necesitas referenciar al propio objeto desde su draw, usa:
    //   std::weak_ptr<DisplayObject> weak = obj;
    //   obj->draw = [weak]() { if (auto self = weak.lock()) { ... } };
    std::function<void()> draw = nullptr;

    bool visible = true;
    float alpha  = 1.0f;
    std::string name = "DisplayObject";

    DisplayObject() = default;
    virtual ~DisplayObject();

    void addChild(std::shared_ptr<DisplayObject> child);
    void removeChild(std::shared_ptr<DisplayObject> child);
    void removeFromParent();
    void removeAllChildren();

    std::shared_ptr<DisplayObject> getParent() const { return parent.lock(); }

    std::shared_ptr<DisplayObject> findChildByName(const std::string& searchName);
    std::vector<std::shared_ptr<DisplayObject>> findChildrenByName(const std::string& searchName);

    void setPosition(float px, float py, float pz = 0.0f) { x = px; y = py; z = pz; }
    void setScale(float sx, float sy, float sz = 1.0f)    { scaleX = sx; scaleY = sy; scaleZ = sz; }
    void setRotation(float rx, float ry, float rz)         { rotationX = rx; rotationY = ry; rotationZ = rz; }

    virtual void render();

    void dispose() {
        if (_disposed) return;
        _disposed = true;
        visible = false;
        if (auto p = parent.lock()) p->removeChild(shared_from_this());
        removeAllChildren();
        draw = nullptr;
    }

    // CORREGIDO: flag explicito, no inferido del estado
    bool isDisposed() const { return _disposed; }

    // CORREGIDO: descuenta el shared_ptr temporal
    long getRefCount() const {
        auto self = shared_from_this();
        return self.use_count() - 1;
    }

private:
    bool _disposed = false;
};

// ============================================================================
// FACTORY
// ============================================================================

template<typename T = DisplayObject, typename... Args>
std::shared_ptr<T> createDisplayObject(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// ============================================================================
// CONTEXT
// ============================================================================

class Context {
private:
    std::string name;
    std::shared_ptr<DisplayObject> stage;
    bool active = true;
    bool is3D   = false;
    bool _disposed = false;
    Camera3D camera3D = {};
    Camera2D camera2D = {};

public:
    float fov            = 45.0f;
    float cam2D_x        = 0.0f;
    float cam2D_y        = 0.0f;
    float cam2D_targetX  = 0.0f;
    float cam2D_targetY  = 0.0f;
    float cam2D_zoom     = 1.0f;
    float cam2D_rotation = 0.0f;

    Context(const std::string& ctxName, bool use3D = false);

    ~Context() {
        if (stage) {
            stage->removeAllChildren();
            stage.reset();
        }
    }

    std::string getName()  const { return name; }
    std::shared_ptr<DisplayObject> getStage() { return stage; }
    Camera3D& getCamera3D()       { return camera3D; }
    Camera2D& getCamera2D()       { return camera2D; }
    bool isActive()   const       { return active; }
    bool isContext3D() const       { return is3D; }
    void setActive(bool state)    { active = state; }

    template<typename T = DisplayObject, typename... Args>
    std::shared_ptr<T> create(Args&&... args) {
        return createDisplayObject<T>(std::forward<Args>(args)...);
    }

    std::shared_ptr<DisplayObject> createAndAddToStage();

    void dispose() {
        if (_disposed) return;
        _disposed = true;
        active = false;
        if (stage) {
            stage->removeAllChildren();
            stage.reset();
        }
    }

    bool isDisposed() const { return _disposed; }

    void syncCamera2D() {
        camera2D.offset   = { cam2D_x, cam2D_y };
        camera2D.target   = { cam2D_targetX, cam2D_targetY };
        camera2D.rotation = cam2D_rotation;
        camera2D.zoom     = cam2D_zoom;
    }

    void syncCamera3D() { camera3D.fovy = fov; }
};

// ============================================================================
// CLASHMASTERCONTEXT
// ============================================================================

class ClashMasterContext {
private:
    int width, height;
    std::string title;
    std::vector<std::shared_ptr<Context>> contexts;

public:
    ClashMasterContext(int w, int h, const std::string& t);
    ~ClashMasterContext();

    void addContext(std::shared_ptr<Context> ctx);
    void removeContext(const std::string& ctxName);
    std::shared_ptr<Context> getContext(const std::string& ctxName);

    void run(std::function<void(ClashMasterContext&, float)> userUpdateCallback);

    int getWidth()  const { return width;  }
    int getHeight() const { return height; }

    void clearAllContexts() {
        for (auto& ctx : contexts) if (ctx) ctx->dispose();
        contexts.clear();
    }

    bool hasContexts() const { return !contexts.empty(); }
};

} // namespace Clash

#endif