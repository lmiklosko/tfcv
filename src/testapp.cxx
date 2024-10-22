#include "tfcv/image.hpp"
#include "tfcv/ml/processor.hpp"
#include "tfcv/ml/classifier.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


//using namespace tfcv;

int main()
{
    tfcv::Image image("/home/lukasmiklosko/projects/Dusan/OptiScout/syngenta-of-pest-detection/samples/cradle_test_image.jpg");

    image = image.resize(32, 32);
    std::vector<tfcv::Image> vec(96, image);

     tfcv::ml::Classifier classifier("/home/lukasmiklosko/projects/Dusan/OptiScout/syngenta-of-pest-detection/gcp.tflite");
    auto classes = classifier.classify(vec);

    cv::imshow("Image", image.underlying_handle());
    cv::waitKey();

//    try
//    {
//        tfcv::Image image(std::move(mat));
//
//        // ===== Inference =====
//        auto ml = tfcv::ml::IProcessor::create("/Users/lukasmiklosko/projects/lukas/tfcv/160.tflite");
//        auto result = ml->run(std::span<const Image>{&image, 1});
//
//
//        // ===== Post-processing =====
//        Image outImg(result, 512, 512, 3);
//        outImg.toOneHot();
//
//        auto paperMask = outImg
//            .channel(1)
//            .depthwiseContour();
//
//        auto rootMask = outImg
//            .channel(0)
//            .select(paperMask);
//
//        auto rootImage = image
//            .resize(512, 512)
//            .addAlphaChannel(rootMask)
//            .cropToFit(3);
//
//        auto nPaper = paperMask.countNonZero();
//        auto nRoots = rootMask.countNonZero();
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << e.what() << std::endl;
//    }
//    return 0;
}