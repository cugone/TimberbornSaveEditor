#include "Game/Game.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/IFont.hpp"
#include "Engine/Core/OrthographicCameraController.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/TimeUtils.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/KeyCode.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Math/Vector2.hpp"

#include "Engine/Platform/PlatformUtils.hpp"
#include "Engine/Platform/Win.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Engine/Services/IAppService.hpp"
#include "Engine/Services/ServiceLocator.hpp"

#include "Engine/UI/UISystem.hpp"

#include "Engine/Game/GameBase.hpp"
#include "Engine/Game/GameSettings.hpp"

#include <Thirdparty/Imgui/imgui.h>

#include <string>


void Game::Initialize() noexcept {
    g_theRenderer->RegisterMaterialsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameMaterials));
    g_theRenderer->RegisterFontsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameFonts));

    _cameraController = OrthographicCameraController();
    _cameraController.SetPosition(Vector2::Zero);
}

void Game::BeginFrame() noexcept {
    /* DO NOTHING */
}

void Game::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    g_theRenderer->UpdateGameTime(deltaSeconds);

    HandlePlayerInput(deltaSeconds);

    _ui_camera2D.Update(deltaSeconds);
    _cameraController.Update(deltaSeconds);

    static std::string filename{};
    ImGui::Begin("Timberton Save Editor");
    {
        ImGui::BeginDisabled(true);
        ImGui::InputText("Filename", &filename);
        ImGui::EndDisabled();
        ImGui::SameLine();
        if(ImGui::Button("...##OFDButton")) {
            filename = FileDialogs::OpenFile("Timber Files (*.timber)\0*.timber\0All Files (*.*)\0*.*\0\0");
        }
    }
    ImGui::End();


}

void Game::Render() const noexcept {
    g_theRenderer->BeginRenderToBackbuffer();


    //World View
    g_theRenderer->SetMaterial(g_theRenderer->GetMaterial("__2D"));
    {
        const auto S = Matrix4::CreateScaleMatrix(Vector2::One);
        const auto R = Matrix4::I;
        const auto T = Matrix4::I;
        const auto M = Matrix4::MakeSRT(S, R, T);
        g_theRenderer->DrawQuad2D(M, Rgba::ForestGreen);
    }

    // HUD View
    {
        const auto ui_view_height = static_cast<float>(GetSettings()->GetWindowHeight());
        const auto ui_view_width = ui_view_height * _ui_camera2D.GetAspectRatio();
        const auto ui_view_extents = Vector2{ ui_view_width, ui_view_height };
        const auto ui_view_half_extents = ui_view_extents * 0.5f;
        const auto ui_cam_pos = Vector2::Zero;
        g_theRenderer->BeginHUDRender(_ui_camera2D, ui_cam_pos, ui_view_height);

        {
            const auto S = Matrix4::CreateScaleMatrix(Vector2::One * (1.0f + MathUtils::SineWaveDegrees(g_theRenderer->GetGameTime().count(), 1.0f)));
            static float r = 0.0f;
            const std::string text = "Abrams 2026 Template";
            const auto* font = g_theRenderer->GetDefaultFont();
            const auto T = Matrix4::I;
            const auto nT = Matrix4::CreateTranslationMatrix(-Vector2{ font->CalculateTextWidth(text), font->CalculateTextHeight() } * 0.5f);
            const auto R = Matrix4::Create2DRotationDegreesMatrix(r);
            static const float w = 90.0f;
            r += g_theRenderer->GetGameFrameTime().count() * w;
            const auto M = Matrix4::MakeRT(nT, Matrix4::MakeSRT(S, R, T));
            g_theRenderer->DrawTextLine(M, font, text);
        }
    }
}

void Game::EndFrame() noexcept {
    /* DO NOTHING */
}

const GameSettings* Game::GetSettings() const noexcept {
    return GameBase::GetSettings();
}

GameSettings* Game::GetSettings() noexcept {
    return GameBase::GetSettings();
}

void Game::HandlePlayerInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleKeyboardInput(deltaSeconds);
    HandleControllerInput(deltaSeconds);
    HandleMouseInput(deltaSeconds);
}

void Game::HandleKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::Esc)) {
        auto* app = ServiceLocator::get<IAppService>();
        app->SetIsQuitting(true);
        return;
    }
}

void Game::HandleControllerInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleDebugInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleDebugKeyboardInput(deltaSeconds);
    HandleDebugMouseInput(deltaSeconds);
}

void Game::HandleDebugKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputKeyboardCapture()) {
        return;
    }
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::F4)) {
        g_theUISystem->ToggleImguiDemoWindow();
    }
}

void Game::HandleDebugMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputMouseCapture()) {
        return;
    }
}
