package main

import rlg "rllights"
import rl "vendor:raylib"

main :: proc() {
    rl.InitWindow(800, 600, "lightiubng");
    rl.SetTargetFPS(60);

    rlg_ctx := rlg.CreateContext(1);
    rlg.SetContext(rlg_ctx);

    camera := rl.Camera {
        position = {2, 2, 2},
        target = {},
        up = {0, 1, 0},
        fovy = 60,
        projection = rl.CameraProjection.PERSPECTIVE,
    };

    cube := rl.LoadModelFromMesh(rl.GenMeshCube(1, 1, 1));

    rlg.UseLight(0, true);
    rlg.SetLightType(0, rlg.LightType.OMNI);
    rlg.SetLightXYZ(0, rlg.LightProperty.POSITION, 2, 2, 2);
    rlg.SetLightXYZ(0, rlg.LightProperty.COLOR, 0.2, 0.5, 1);

    for (!rl.WindowShouldClose()) {
        rl.UpdateCamera(&camera, rl.CameraMode.ORBITAL);
        rlg.SetViewPositionV(camera.position);

        rl.BeginDrawing();
        rl.ClearBackground(rl.BLACK);
        
        rl.BeginMode3D(camera);
        rlg.DrawModel(cube, {}, 1, rl.WHITE);
        rl.EndMode3D();

        rl.EndDrawing();
    }

    rl.UnloadModel(cube);
    rlg.DestroyContext(rlg_ctx);
    rl.CloseWindow();
}
