//Internal Libraries
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>

//External Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

//Definitions
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944
#endif

// Function to Check if audio file provided exists
bool fileExists(const char* filePath) {
    std::ifstream file(filePath);
    return file.good(); // Check if the file stream is in a good state
}

// Function to convert ImVec4 Color to SFML Color
sf::Color ImVec4ToSFMLColor(const ImVec4& color) {
    return sf::Color(static_cast<sf::Uint8>(color.x * 255),
        static_cast<sf::Uint8>(color.y * 255),
        static_cast<sf::Uint8>(color.z * 255),
        static_cast<sf::Uint8>(color.w * 255));
}

int main() {
    //Misc Vars
    static int currentShapeOption = 0; // Variable for current item in shape dropdown
    static bool isPlaying = false; //Play button bool
    
    //Vars for Settings
    static const char* shapeOptions[] = { "Line", "Circle" }; // Shape dropdown items list
    static ImVec4 visualColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Initial color (red)
    static ImVec4 backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // Initial color (red)
    static float lineScaleValue = 0.0025f;
    static float circleScaleValue = 0.005f; //Value for Circle scale
    static float circleRadiusValue = 100.f; //Int for radius

    char filePathBuffer[256] = ""; //Buffer for file path text box
    static char playingText[256] = "(None)";
    

    // Create SFML window
    sf::RenderWindow window(sf::VideoMode(800, 800), "Simple Audio Visualizer", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    // Create Music Elements
    sf::Music music;// Create a music object
    sf::SoundBuffer soundBuffer;// Create a sound buffer to hold audio data
    sf::Sound sound(soundBuffer);// Create a sound to play audio

    // Create Visual Elements
    sf::VertexArray lineform(sf::LineStrip);// Create VertexArray object for Line

    // Create Line Visual
    lineform.resize(window.getSize().x); // Adjust size based on window width
    for (size_t i = 0; i < lineform.getVertexCount(); ++i) {
        lineform[i].position = sf::Vector2f(i, 400); // Set initial position
    }

    const size_t numVertices = 360; // Define number of vertices for circle visual

    sf::VertexArray waveform(sf::LineStrip); // Create VertexArray object for Circle
    waveform.resize(numVertices); // Set the size of the vertex array

    // Create Circle Visual
    for (size_t i = 0; i < numVertices; ++i) {
        float angle = static_cast<float>(i) / numVertices * 2.f * M_PI;
        float radius = circleRadiusValue;//
        float x = 400.f + radius * std::cos(angle);
        float y = 400.f + radius * std::sin(angle);
        waveform[i].position = sf::Vector2f(x, y); // Set position in a circular pattern
    }
    
    sf::Clock deltaClock; // Create Clock object

    // Main loop
    while (window.isOpen()) {
        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);//
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());//

        ImGui::Begin("Settings");
        ImGui::Text("Shape:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##Dropdown", shapeOptions[currentShapeOption])) {
            for (int i = 0; i < IM_ARRAYSIZE(shapeOptions); i++) {
                bool isSelected = (currentShapeOption == i);
                if (ImGui::Selectable(shapeOptions[i], isSelected)) {
                    currentShapeOption = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
                ImGui::EndCombo();
        }
        if (ImGui::TreeNode("Additional Shape Settings")) {
            if (currentShapeOption == 0) {
                ImGui::Indent();
                ImGui::Text("Scale:");
                ImGui::SameLine();
                ImGui::InputFloat("##Line Scale Value", &lineScaleValue, 0.0001f, 0.0001f * 10.0f, "%.4f", ImGuiInputTextFlags_CharsDecimal);
                if (lineScaleValue < 0.0002f) lineScaleValue = 0.0002f;
                if (lineScaleValue > 0.02f) lineScaleValue = 0.02f;
                ImGui::Unindent();
            }
            else if (currentShapeOption == 1) {
                ImGui::Indent();
                ImGui::Text("Scale:");
                ImGui::SameLine();
                ImGui::InputFloat("##Float Value", &circleScaleValue, 0.0001f, 0.0001f * 10.0f, "%.4f", ImGuiInputTextFlags_CharsDecimal);
                if (circleScaleValue < 0.0005f) circleScaleValue = 0.0005f;
                if (circleScaleValue > 0.05f) circleScaleValue = 0.05f;
                ImGui::Text("Radius:");
                ImGui::SameLine();
                ImGui::InputFloat("##Radius Value2", &circleRadiusValue, 1.0f, 1.0f, "%.f", ImGuiInputTextFlags_CharsDecimal);
                if (circleRadiusValue < 25.0f) circleRadiusValue = 25.0f;
                if (circleRadiusValue > 250.0f) circleRadiusValue = 250.0f;
                ImGui::Unindent();
            }
            else {
                ImGui::Indent();
                ImGui::Text("Scale:");
                ImGui::Unindent();
            }
            ImGui::TreePop(); // End inner collapsible header
        }
        ImGui::Text("Line Color:");
        ImGui::SameLine();
        ImGui::ColorEdit4("##picker", (float*)&visualColor);
        ImGui::Text("Background Color:");
        ImGui::SameLine();
        ImGui::ColorEdit4("##picker2", (float*)&backgroundColor);
        
        if (ImGui::Button("Reset")) {
            visualColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            lineScaleValue = 0.0025f;
            circleScaleValue = 0.005f;
            circleRadiusValue = 100.f;
        }
        ImGui::End();

        //Second Window
        ImGui::Begin("Audio Player");
        
        ImGui::Text("File Path:");
        ImGui::SameLine();
        ImGui::InputText("##FilePathInput", filePathBuffer, sizeof(filePathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Import")) {
            if (fileExists(filePathBuffer) && music.openFromFile(filePathBuffer)) {
                strcpy_s(playingText, sizeof(playingText), filePathBuffer);

                if (!soundBuffer.loadFromFile(playingText)) {
                    std::cerr << "Failed to load sound buffer" << std::endl;
                    return 1;
                }

                sound.setLoop(true);
            }
            else {
                const char* newValue = "File path/type invalid.";
                strcpy_s(playingText, sizeof(playingText), newValue);
            }
        }
        ImGui::Text("Current File: ");
        ImGui::SameLine();
        ImGui::Text(playingText);//CHANGE

        if (isPlaying) {
            if (ImGui::Button("Pause")) { // Pause button clicked
                isPlaying = false;
                sound.pause();
            }
        }
        else {
            if (ImGui::Button("Play")) { // Play button clicked
                isPlaying = true;
                sound.play();
            }
        }
        
        ImGui::End();

        // Update audio visualization
        sf::Time playingOffset = sound.getPlayingOffset();
        size_t sampleOffset = playingOffset.asMilliseconds() * soundBuffer.getSampleRate() / 1000;

        for (size_t i = 0; i < lineform.getVertexCount(); ++i) {
            // Ensure valid sample index
            if (sampleOffset + i < soundBuffer.getSampleCount()) {
                float sample = soundBuffer.getSamples()[sampleOffset + i]; // Get sample value
                lineform[i].position.y = 400 - sample * lineScaleValue; // Adjust height based on sample value
                lineform[i].color = ImVec4ToSFMLColor(visualColor);
            }
        }
        
        for (size_t i = 0; i < waveform.getVertexCount(); ++i) {
            if (sampleOffset + i < soundBuffer.getSampleCount()) {
                float sample = soundBuffer.getSamples()[sampleOffset + i]; // Get sample value
                float scaleFactor = circleScaleValue;//
                float radius = circleRadiusValue + std::abs(sample) * scaleFactor;//
                float angle = static_cast<float>(i) / waveform.getVertexCount() * 2.f * M_PI;
                float x = 400.f + radius * std::cos(angle);
                float y = 400.f + radius * std::sin(angle);
                waveform[i].position = sf::Vector2f(x, y); // Set position with adjusted radius
                waveform[i].color = ImVec4ToSFMLColor(visualColor);
            }
        }
        
        window.clear(ImVec4ToSFMLColor(backgroundColor));

        if (currentShapeOption == 0) {
            window.draw(lineform);
        }
        else if (currentShapeOption == 1) {
            window.draw(waveform);
        }
        else {
            window.draw(lineform);
        }
        
        ImGui::SFML::Render(window);//

        // Display the window
        window.display();
    }

    return 0;
}
