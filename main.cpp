#include "face_remover.hpp"

#include <crow.h>
#include <iostream>
#include <mutex>
#include <string>

std::mutex process_mutex; // Для синхронизации потоков

void process_video(const std::string &input_path, const std::string &output_path, const std::string &filename)
{
    std::lock_guard<std::mutex> lock(process_mutex);

    try {
        face_remover::video_processor video_proc(input_path, output_path);

        if (video_proc.open_file(filename)) {
            std::cout << "Processing video: " << filename << std::endl;
            video_proc.proc();
            std::cout << "Finished processing video: " << filename << std::endl;
        } else {
            std::cerr << "Error: Could not open file " << filename << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception during video processing: " << e.what() << std::endl;
    }
}

int main()
{
    crow::SimpleApp app;

    // Маршрут для обработки видео
    CROW_ROUTE(app, "/convert").methods("POST"_method)([](const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("input_path") || !body.has("output_path") || !body.has("filename")) {
            return crow::response(400, "Invalid JSON body. Required: input_path, output_path, filename.");
        }

        std::string input_path = body["input_path"].s();
        std::string output_path = body["output_path"].s();
        std::string filename = body["filename"].s();

        process_video(input_path, output_path, filename); // Запускаем обработку в отдельном потоке

        crow::json::wvalue response;
        response["fileName"] = filename;

        return crow::response(200, response);
    });

    // Запуск сервера
    app.port(8081).multithreaded().run();

    return 0;
}
