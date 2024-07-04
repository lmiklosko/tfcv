#include "tfcv/image.hpp"
#include "tfcv/ml/processor.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace tfcv;

int main()
{
    cv::Mat mat = cv::imread("/home/lukasmiklosko/projects/Dusan/SeedcareONE/syngentaseedcareapp-native/samples/image.JPG");

    try
    {
        tfcv::Image image(std::move(mat));

        // ===== Inference =====
        auto ml = ml::IProcessor::create("/home/lukasmiklosko/projects/Dusan/SeedcareONE/syngentaseedcareapp-native/160.tflite");
        auto result = ml->run(std::span<const Image>{&image, 1});


        // ===== Post-processing =====
        Image outImg(result, 512, 512, 3);
        outImg.toOneHot();

        auto paperMask = outImg
            .channel(1)
            .depthwiseContour();

        auto rootMask = outImg
            .channel(0)
            .select(paperMask);

        auto rootImage = image
            .resize(512, 512)
            .addAlphaChannel(rootMask)
            .cropToFit(3);

        auto nPaper = paperMask.countNonZero();
        auto nRoots = rootMask.countNonZero();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}