#include <api/Detect.hpp>

namespace api {
    
Detect::Detect()
    : opened(false)
{
}

void Detect::open() {
    using namespace std;
    using namespace dlib;
    if (opened)
        return;
    try
    {
        detector = get_frontal_face_detector();
        deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
        deserialize("Training_official.dat") >> net;

        deserialize("database_name.dat") >> name;
        deserialize("database_decr.dat") >> database_decr;
        opened = true;
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

bool Detect::is_opened() {
    return opened;
}

std::vector<std::string> Detect::detect_user(cv::Mat image)
{
    using namespace std;
    using namespace dlib;
    try
    {
        dlib::matrix<rgb_pixel> img;
        std::vector<string> person;

        cv_image<bgr_pixel> cimg(image);
        assign_image(img,cimg);
        std::vector<matrix<rgb_pixel>> faces;

        for(auto face : detector(img))
        {
            auto shape = sp(img, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
            faces.push_back(move(face_chip));
        }

        std::vector<matrix<float,0,1>> face_descriptors = net(faces);
        std::vector<int> match_count(faces.size());
        std::vector<int> candidate(database_decr.size());
        std::vector<float> distance_list;

        for(size_t a = 0; a < faces.size(); ++a)
        {
            int preserve_index = 0;
            for(size_t b = 0; b < database_decr.size(); ++b)
            {
                float distant = length(face_descriptors[a]-database_decr[b]);
                distance_list.push_back(distant);
                if(distant < 0.33)
                {
                    preserve_index = b;
                    match_count[a]++;
                    candidate[b]++;
                }
            }
            if(match_count[a] == 0)
            {
                // std::cerr << "Not recognized!" << std::endl;
            }
            else if(match_count[a] != 1)
            {
                int index = 0;
                float distance_temp = distance_list[0];
                for(size_t c = 0; c < candidate.size(); c++)
                {
                    if(candidate[c] != 0)
                    {
                        if(distance_temp > distance_list[c])
                        {
                            index = c;
                            distance_temp = distance_list[c];
                        }
                    }
                }
                // std::cerr << "\"" << name[index] << "\" is here!" << "   with distance : " << distance_list[index] << std::endl;
                person.push_back(name[index]);
            }
            else if(match_count[a] == 1)
            {
                // std::cerr << "\"" << name[preserve_index] << "\" is here!" << "   with distance : " << distance_list[preserve_index] << std::endl;
                person.push_back(name[preserve_index]);
            }
        }
        // if(faces.size() != 0)
        //           std::cerr << "----------------------------------------" << std::endl;

        return person;
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
    }
    return {};
}

} // namespace api
