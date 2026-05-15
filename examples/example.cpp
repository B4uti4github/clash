// ============================================================================
// EJEMPLO 2D - Sistema Solar
// Archivo: clash/examples/example.cpp
//
// Demuestra:
//  - Jerarquia de DisplayObjects (pivote → planeta → luna)
//  - Rotacion relativa: rotar el pivote mueve todo el hijo
//  - Camera 2D con zoom
// ============================================================================

#include "Clash.h"
#include "raylib.h"
#include <cmath>

int main() {

    Clash::ClashMasterContext master(800, 600, "Clash 2D - Sistema Solar");

    auto ctx = std::make_shared<Clash::Context>("solar", false);
    ctx->cam2D_x  = 400.0f;  // centrar camara en pantalla
    ctx->cam2D_y  = 300.0f;
    master.addContext(ctx);

    auto stage = ctx->getStage();

    // ----------------------------------------------------------------
    // Orbitas decorativas (primero = debajo de todo)
    // Lambda no captura nada: sin riesgo de ciclos
    // ----------------------------------------------------------------
    auto orbitas = ctx->create<Clash::DisplayObject>();
    orbitas->name = "orbitas";
    orbitas->draw = []() {
        DrawCircleLines(0, 0, 120, Fade(WHITE, 0.15f));
        DrawCircleLines(0, 0, 200, Fade(WHITE, 0.15f));
    };
    stage->addChild(orbitas);

    // ----------------------------------------------------------------
    // Sol
    // ----------------------------------------------------------------
    auto sol = ctx->create<Clash::DisplayObject>();
    sol->name = "sol";
    sol->draw = []() {
        DrawCircle(0, 0, 30, YELLOW);
        DrawCircleLines(0, 0, 30, ORANGE);
    };
    stage->addChild(sol);

    // ----------------------------------------------------------------
    // Tierra: pivotTierra (rota) → tierra (fijo a distancia) → luna
    //
    // Al rotar pivotTierra, la Tierra orbita el Sol.
    // Al rotar pivotLuna, la Luna orbita la Tierra.
    // No hace falta calcular seno/coseno manualmente.
    // ----------------------------------------------------------------
    auto pivotTierra = ctx->create<Clash::DisplayObject>();
    pivotTierra->name = "pivotTierra";
    sol->addChild(pivotTierra);

    auto tierra = ctx->create<Clash::DisplayObject>();
    tierra->name = "tierra";
    tierra->setPosition(120.0f, 0.0f);
    tierra->draw = []() {
        DrawCircle(0, 0, 14, BLUE);
        DrawCircleLines(0, 0, 14, DARKBLUE);
    };
    pivotTierra->addChild(tierra);

    auto pivotLuna = ctx->create<Clash::DisplayObject>();
    pivotLuna->name = "pivotLuna";
    tierra->addChild(pivotLuna);

    auto luna = ctx->create<Clash::DisplayObject>();
    luna->name = "luna";
    luna->setPosition(30.0f, 0.0f);
    luna->draw = []() {
        DrawCircle(0, 0, 5, LIGHTGRAY);
        DrawCircleLines(0, 0, 5, GRAY);
    };
    pivotLuna->addChild(luna);

    // ----------------------------------------------------------------
    // Marte con Fobos y Deimos
    // ----------------------------------------------------------------
    auto pivotMarte = ctx->create<Clash::DisplayObject>();
    pivotMarte->name = "pivotMarte";
    sol->addChild(pivotMarte);

    auto marte = ctx->create<Clash::DisplayObject>();
    marte->name = "marte";
    marte->setPosition(200.0f, 0.0f);
    marte->draw = []() {
        DrawCircle(0, 0, 10, RED);
        DrawCircleLines(0, 0, 10, MAROON);
    };
    pivotMarte->addChild(marte);

    auto pivotFobos = ctx->create<Clash::DisplayObject>();
    marte->addChild(pivotFobos);
    auto fobos = ctx->create<Clash::DisplayObject>();
    fobos->name = "fobos";
    fobos->setPosition(20.0f, 0.0f);
    fobos->draw = []() { DrawCircle(0, 0, 3, GRAY); };
    pivotFobos->addChild(fobos);

    auto pivotDeimos = ctx->create<Clash::DisplayObject>();
    marte->addChild(pivotDeimos);
    auto deimos = ctx->create<Clash::DisplayObject>();
    deimos->name = "deimos";
    deimos->setPosition(28.0f, 0.0f);
    deimos->draw = []() { DrawCircle(0, 0, 2, DARKGRAY); };
    pivotDeimos->addChild(deimos);

    // ----------------------------------------------------------------
    // Bucle principal
    // ----------------------------------------------------------------
    float t = 0.0f;

    master.run([&](Clash::ClashMasterContext&, float dt) {
        t += dt;

        // Velocidades orbitales (grados por segundo)
        pivotTierra->rotationZ  = t * (360.0f / 4.0f);    // vuelta cada 4s
        pivotLuna->rotationZ    = t * (360.0f / 1.0f);    // vuelta cada 1s
        pivotMarte->rotationZ   = t * (360.0f / 7.5f);    // vuelta cada 7.5s
        pivotFobos->rotationZ   = t * (360.0f / 0.3f);
        pivotDeimos->rotationZ  = t * (360.0f / 1.2f);
        sol->rotationZ          = t * 10.0f;

        // Zoom con rueda del raton
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            ctx->cam2D_zoom += wheel;
            if (ctx->cam2D_zoom < 0.1f){
                ctx->cam2D_zoom = 0.1f;
            } else if (ctx->cam2D_zoom > 3.0f){
                ctx->cam2D_zoom = 3.0f;
            }
        }

        // HUD (fuera del BeginMode2D, coordenadas de pantalla)
        DrawText("Sistema Solar - Clash 2D",       10, 10, 20, WHITE);
        DrawText("Rueda del raton: zoom",           10, 35, 14, LIGHTGRAY);
        DrawFPS(10, 560);
    });

    return 0;
}