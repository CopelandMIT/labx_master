// ZED include
#include <sl/Camera.hpp>
#include "GLViewer.hpp"
#include "SK_Serializer.hpp"
#include <chrono>
#include <string>
#include <fstream>

// Main loop
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <output_file> <capture_duration_seconds>" << std::endl;
        return -1;
    }

    std::string output_filename = argv[1];
    int capture_duration_seconds = std::stoi(argv[2]);

    sl::Camera zed;
    sl::InitParameters init_parameters;
    init_parameters.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP;

    auto state = zed.open(init_parameters);
    if (state != sl::ERROR_CODE::SUCCESS)
    {
        std::cout << "Error opening ZED: " << state << std::endl;
        return -1;
    }

    state = zed.enablePositionalTracking();
    if (state != sl::ERROR_CODE::SUCCESS)
    {
        std::cout << "Error enabling positional tracking: " << state << std::endl;
        return -1;
    }

    sl::BodyTrackingParameters body_tracking_parameters;
    body_tracking_parameters.detection_model = sl::BODY_TRACKING_MODEL::HUMAN_BODY_MEDIUM;
    body_tracking_parameters.body_format = sl::BODY_FORMAT::BODY_38;
    body_tracking_parameters.enable_tracking = true;
    body_tracking_parameters.enable_body_fitting = false;

    state = zed.enableBodyTracking(body_tracking_parameters);
    if (state != sl::ERROR_CODE::SUCCESS)
    {
        std::cout << "Error enabling body tracking: " << state << std::endl;
        return -1;
    }

    GLViewer viewer;
    viewer.init(argc, argv);

    auto start_time = std::chrono::steady_clock::now();
    nlohmann::json bodies_json;
    sl::Bodies bodies;

    while (viewer.isAvailable())
    {
        auto elapsed_time = std::chrono::steady_clock::now() - start_time;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count() >= capture_duration_seconds)
        {
            break;
        }

        if (zed.grab() == sl::ERROR_CODE::SUCCESS)
        {
            zed.retrieveBodies(bodies);
            viewer.updateData(bodies);

            bodies_json[std::to_string(bodies.timestamp.getMilliseconds())] = sk::serialize(bodies);
        }
    }

    zed.close();

    if (!bodies_json.empty())
    {
        std::ofstream file_out(output_filename);
        file_out << std::setw(4) << bodies_json << std::endl;
        file_out.close();
        std::cout << "Successfully saved body data to " << output_filename << std::endl;
    }
    else
    {
        std::cout << "No body data to save." << std::endl;
    }

    return EXIT_SUCCESS;
}
