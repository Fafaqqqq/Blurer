#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#define NET_PROTO_FILE "deploy.prototxt"
#define NET_CAFFE_FILE "res10_300x300_ssd_iter_140000.caffemodel"

namespace face_remover {

using video_capture = cv::VideoCapture;
using video_writer = cv::VideoWriter;
using video_frame = cv::Mat;
using network = std::shared_ptr<cv::dnn::Net>;

class frame_processor {
public:
    frame_processor(const network &net)
        : net_(net)
    {
    }

    void proc(video_frame &frame, network &net, float confThreshold)
    {
        video_frame blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300), cv::Scalar(104, 177, 123), false, false);
        net->setInput(blob);
        cv::Mat detections = net->forward();

        // Получаем указатель на данные
        float *data = (float *)detections.data;

        for (int i = 0; i < detections.size[2]; i++) {
            float confidence = data[i * 7 + 2];

            // Если уверенность выше порогового значения
            if (confidence > confThreshold) {
                int x1 = static_cast<int>(data[i * 7 + 3] * frame.cols);
                int y1 = static_cast<int>(data[i * 7 + 4] * frame.rows);
                int x2 = static_cast<int>(data[i * 7 + 5] * frame.cols);
                int y2 = static_cast<int>(data[i * 7 + 6] * frame.rows);

                // Ограничиваем координаты в пределах изображения
                x1 = std::max(0, std::min(x1, frame.cols - 1));
                y1 = std::max(0, std::min(y1, frame.rows - 1));
                x2 = std::max(0, std::min(x2, frame.cols - 1));
                y2 = std::max(0, std::min(y2, frame.rows - 1));

                if (abs(x1 - x2) == 0 || abs(y1 - y2) == 0) {
                    continue;
                }

                // Выделяем область лица
                cv::Rect faceRect(cv::Point(x1, y1), cv::Point(x2, y2));

                // Размытие лица
                cv::Mat faceROI = frame(faceRect);
                GaussianBlur(faceROI, faceROI, cv::Size(55, 55), 30);

                // Вставляем размытое лицо обратно в кадр
                faceROI.copyTo(frame(faceRect));
            }
        }
    }

private:
    network net_;
};

class video_processor {
public:
    video_processor(const std::string &input_path,
                    const std::string &output_path, const std::string &model_path, const std::string &protoPath,   const std::string &caffePath)
        : input_path_(input_path)
        , output_path_(output_path)
        , net_(
              std::make_shared<cv::dnn::Net>(
                  cv::dnn::readNetFromCaffe(model_path + protoPath, model_path + caffePath)))
        , frame_proc_(net_)
    {
        net_->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net_->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    }

    bool open_file(const std::string &filename)
    {
        filename_ = filename;
        return capture_.open(input_path_ + filename);
    }

    void proc()
    {
        video_frame frame;

        // Получаем свойства камеры для настройки VideoWriter
        int frameWidth = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
        int frameHeight = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
        double fps = capture_.get(cv::CAP_PROP_FPS);

        std::stringstream ss(filename_);
        std::string outfile;

        std::getline(ss, outfile, '.');

        // Настраиваем VideoWriter для сохранения видео
        video_writer videoWriter(
            // output_path_ + outfile + ".mp4",
            output_path_ + filename_,
            video_writer::fourcc('a', 'v', 'c', '1'),
            fps,
            cv::Size(frameWidth, frameHeight));

        if (!videoWriter.isOpened()) {
            std::cout << "Error: Could not open video writer. FILE: " << output_path_ + outfile + ".avi" << std::endl;
            return;
        }

        while (true) {
            capture_ >> frame;
            if (frame.empty()) {
                std::cout << "Empty frame" << std::endl;
                break;
            }

            frame_proc_.proc(frame, net_, 0.35);

            // Сохраняем обработанный кадр в видеофайл
            videoWriter.write(frame);
        }
    }

private:
    std::string input_path_;
    std::string output_path_;
    std::string filename_;

    network net_;
    video_capture capture_;
    frame_processor frame_proc_;
};
} // namespace face_remover