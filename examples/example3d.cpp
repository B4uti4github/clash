// ============================================================================
// EJEMPLO 3D - Cubos giratorios jerarquicos
// Archivo: clash/examples/example3d.cpp
// ============================================================================

#include "Clash.h"
#include "raylib.h"
#include "rlgl.h"
#include <cmath>

int main() {

    Clash::ClashMasterContext master(900, 600, "Clash 3D - Cubos");

    auto ctx = std::make_shared<Clash::Context>("mundo", true);
    ctx->fov = 45.0f;
    master.addContext(ctx);

    auto stage = ctx->getStage();

    // ----------------------------------------------------------------
    // Suelo
    // ----------------------------------------------------------------
    auto suelo = ctx->create<Clash::DisplayObject>();
    suelo->draw = []() {
        DrawPlane({ 0.0f, 0.0f, 0.0f }, { 20.0f, 20.0f }, DARKGRAY);
        DrawGrid(20, 1.0f);
    };
    stage->addChild(suelo);

    // ----------------------------------------------------------------
    // Cubo padre (grande, en el centro)
    // ----------------------------------------------------------------
    auto cuboPadre = ctx->create<Clash::DisplayObject>();
    cuboPadre->setPosition(0.0f, 1.0f, 0.0f);
    cuboPadre->draw = []() {
        DrawCube({ 0.0f, 0.0f, 0.0f }, 1.5f, 1.5f, 1.5f, BLUE);
        DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 1.5f, 1.5f, 1.5f, WHITE);
    };
    stage->addChild(cuboPadre);

    // ----------------------------------------------------------------
    // Cubo hijo (mediano, orbita al padre)
    // Posicionado a distancia del padre, hereda su rotacion
    // ----------------------------------------------------------------
    auto cuboHijo = ctx->create<Clash::DisplayObject>();
    cuboHijo->setPosition(4.0f, 0.0f, 0.0f);
    cuboHijo->draw = []() {
        DrawCube({ 0.0f, 0.0f, 0.0f }, 0.9f, 0.9f, 0.9f, GREEN);
        DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 0.9f, 0.9f, 0.9f, WHITE);
    };
    cuboPadre->addChild(cuboHijo);

    // ----------------------------------------------------------------
    // Cubo nieto (chico, orbita al hijo)
    // ----------------------------------------------------------------
    auto cuboNieto = ctx->create<Clash::DisplayObject>();
    cuboNieto->setPosition(2.0f, 0.0f, 0.0f);
    cuboNieto->draw = []() {
        DrawCube({ 0.0f, 0.0f, 0.0f }, 0.5f, 0.5f, 0.5f, ORANGE);
        DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 0.5f, 0.5f, 0.5f, WHITE);
    };
    cuboHijo->addChild(cuboNieto);

    // ----------------------------------------------------------------
    // Camara fija mirando la escena
    // ----------------------------------------------------------------
    auto& cam = ctx->getCamera3D();
    cam.position = { 10.0f, 8.0f, 10.0f };
    cam.target   = {  0.0f, 1.0f,  0.0f };
    cam.up       = {  0.0f, 1.0f,  0.0f };

    float t = 0.0f;

    master.run([&](Clash::ClashMasterContext&, float dt) {
        t += dt;

        // Padre gira sobre si mismo
        cuboPadre->rotationY = t * 40.0f;

        // Hijo orbita al padre (rotando el padre lo arrastra)
        // y ademas gira sobre su propio eje
        cuboHijo->rotationY = t * 80.0f;

        // Nieto gira rapido sobre su eje
        cuboNieto->rotationY = t * 160.0f;
        cuboNieto->rotationX = t * 90.0f;

        DrawText("Clash 3D - Cubos jerarquicos", 10, 10, 20, WHITE);
        DrawText("El verde orbita al azul", 10, 35, 14, LIGHTGRAY);
        DrawText("El naranja orbita al verde", 10, 55, 14, LIGHTGRAY);
        DrawFPS(10, 570);
    });

    return 0;
}