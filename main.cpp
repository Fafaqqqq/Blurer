#include "face_remover.hpp"

#include <crow.h>
#include <iostream>
#include <mutex>
#include <string>

std::mutex process_mutex; // Для синхронизации потоков

void process_video(const std::string &input_path, const std::string &output_path, const std::string &filename,
                   const std::string &model_path, const std::string &protoPath,   const std::string &caffePath)
{
    // std::lock_guard<std::mutex> lock(process_mutex);

    try {
        face_remover::video_processor video_proc(input_path, output_path, model_path, protoPath, caffePath);

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

void process_video(const std::string &input_path, const std::string &output_path, const std::string &filename)
{
    // std::lock_guard<std::mutex> lock(process_mutex);

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
        if (!body || !body.has("inputPath") || !body.has("outputPath") || !body.has("inputFileName")) {
            return crow::response(400, "Invalid JSON body. Required: input_path, output_path, filename.");
        }

        std::string input_path = body["inputPath"].s();
        std::string filename = body["inputFileName"].s();
        std::string output_path = body["outputPath"].s();

        if (!body.has("modelInfo") ||
            !body["modelInfo"].has("modelsPath") ||
            !body["modelInfo"].has("protoFile") ||
            !body["modelInfo"].has("caffeFile")) {
            return crow::response(400, "Invalid JSON body. Required: modelsPath, caffeFile, protoFile.");
        }

        if (body["modelInfo"]["protoFile"].s() == "proto_cars_lisence.prototxt" &&
            body["modelInfo"]["caffeFile"].s() == "caffe_cars_lisence.caffemodel") {
            process_video(input_path, output_path, filename); // Запускаем обработку в отдельном потоке
        } else {
            std::string model_path = body["modelInfo"]["modelsPath"].s();
            std::string model_proto = body["modelInfo"]["protoFile"].s();
            std::string model_caffe = body["modelInfo"]["caffeFile"].s();

            std::cout << model_path + model_proto << std::endl;
            std::cout << model_path + model_caffe << std::endl;

            process_video(input_path, output_path, filename, model_path, model_proto, model_caffe);
        }

        crow::json::wvalue response;
        response["fileName"] = filename;

        return crow::response(200, response);
    });

    // Запуск сервера
    app.port(8081).multithreaded().run();

    return 0;
}
