// ============================================================================
// EJEMPLO 2D - Fisica
// Archivo: clash/examples/physics2d.cpp
// ============================================================================

#include "Clash.h"
#include "raylib.h"
#include <cmath>

int main() {

    Clash::ClashMasterContext master(800, 600, "Clash 2D - Fisica");

    auto ctx = std::make_shared<Clash::Context>("fisica", false);
    ctx->cam2D_x  = 400.0f;  // centrar camara en pantalla
    ctx->cam2D_y  = 300.0f;
    master.addContext(ctx);

    auto stage = ctx->getStage();


    // ----------------------------------------------------------------
    // pelota
    // ----------------------------------------------------------------
    auto pelota = ctx->create<Clash::DisplayObject>();
    pelota->name = "pelota";
    pelota->draw = []() {
        DrawCircle(0, 0, 30, RED);
        DrawCircleLines(0, 0, 30, BLACK);
    };
    stage->addChild(pelota);

    // ----------------------------------------------------------------
    // Bucle principal
    // ----------------------------------------------------------------
    float t = 0.0f;
    bool salto = false;   
    float gravedad = 0.0f;
    float impulso = 0.0f;
    float fuerza = 10.0f;
    master.run([&](Clash::ClashMasterContext&, float dt) {
        t += dt;
        if(IsKeyPressed(KEY_W)){
            salto = true;
        }
        if(salto == true){
            gravedad -= 0.1f;
            impulso -= fuerza + gravedad;
        }
        if(impulso > 0.0f) {
            impulso = 0.0f;
            gravedad = 0.0f;
            fuerza = fuerza / 2.0f;
            if(fuerza < 0.2f) fuerza = 10.0f;
            salto = false;
        }
        pelota->setPosition(0.0f, impulso);

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
        DrawText("Fisica - Clash 2D",       10, 10, 20, WHITE);
        DrawText("Rueda del raton: zoom",           10, 35, 14, LIGHTGRAY);
        DrawFPS(10, 560);
    });

    return 0;
}