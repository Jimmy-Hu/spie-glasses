//-----include header files, 引入標頭檔-----
#include <api/Detect.hpp>														//	include <api/Detect.hpp> header file, 引入<api/Detect.hpp>標頭檔
#include "opencv2/imgproc/imgproc.hpp"											//	include "opencv2/imgproc/imgproc.hpp" header file, 引入"opencv2/imgproc/imgproc.hpp"標頭檔

void MyGammaCorrection(cv::Mat& src, cv::Mat& dst, float fGamma)
{
    using namespace cv;
    CV_Assert(src.data);
    
    // accept only char type matrices
    CV_Assert(src.depth() != sizeof(uchar));  
    
    // build look up table
    unsigned char lut[256];
    for( int i = 0; i < 256; i++ )
    {
        lut[i] = saturate_cast<uchar>(pow((float)(i/255.0), fGamma) * 255.0f);
    }
    
    dst = src.clone();
    const int channels = dst.channels();
    switch(channels)
    {
        case 1:
        {
    
            MatIterator_<uchar> it, end;
            for( it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++ )
			{
				//*it = pow((float)(((*it))/255.0), fGamma) * 255.0;
				*it = lut[(*it)];
			}
            break;
        }
        case 3:
        {  
      
            MatIterator_<Vec3b> it, end;  
            for( it = dst.begin<Vec3b>(), end = dst.end<Vec3b>(); it != end; it++ )  
            {  
                //(*it)[0] = pow((float)(((*it)[0])/255.0), fGamma) * 255.0;  
                //(*it)[1] = pow((float)(((*it)[1])/255.0), fGamma) * 255.0;  
                //(*it)[2] = pow((float)(((*it)[2])/255.0), fGamma) * 255.0;  
                (*it)[0] = lut[((*it)[0])];  
                (*it)[1] = lut[((*it)[1])];  
                (*it)[2] = lut[((*it)[2])];  
            }  
            break;  
        }  
    }  
}  

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

	std::vector<std::string> Detect::detect_user(cv::Mat& image)				//	detect_user
	{
		using namespace std;
		using namespace dlib;
		std::vector<cv::Mat> tmp;
		try
		{
			cv::Mat image_temp;
			cv::cvtColor(image, image_temp, CV_BGR2HSV);
			cv::split(image_temp,tmp);
			cv::medianBlur(tmp[2], tmp[2], 3);
			cv::equalizeHist(tmp[2], tmp[2]);
			cv::merge(tmp,image_temp);
			cv::cvtColor(image_temp, image, CV_HSV2BGR);

			dlib::matrix<rgb_pixel> img;
			std::vector<string> person;

			cv_image<bgr_pixel> cimg(image);
			assign_image(img,cimg);
			std::vector<matrix<rgb_pixel>> faces;
			
			//bool flag = true;
			//std::thread mThread(getKey, cvWaitKey(20), &count, &flag, img);
			//mThread.join();

			for(auto face_rect : detector(img))
			{
				auto shape = sp(img, face_rect);
				matrix<rgb_pixel> face_chip;
				extract_image_chip(img, get_face_chip_details(shape,150,0.25), face_chip);
				faces.push_back(move(face_chip));
				//draw_rectangle(img, face, rgb_pixel(255,0,0), 1);         
				cv::rectangle(
					image,
					cv::Point(face_rect.left(),face_rect.top()),
					cv::Point(face_rect.right(),face_rect.bottom()),
					cv::Scalar(255,0,0));
					
					
			}

			std::vector<matrix<float,0,1>> face_descriptors = net(faces);
			std::vector<int> match_count(faces.size());
			std::vector<int> candidate(database_decr.size());
			std::vector<float> distance_list;

			for(size_t a = 0; a < faces.size(); ++a)
			{
				cv::Mat save = toMat(faces[a]);
				cv::imwrite(FILE_NAME + std::to_string(a) + ".jpg", save);
				
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
					person.push_back("NOT_RECOGNIZED");
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
		person.push_back("\n");
			return person;
		}
		catch (std::exception& e)
		{
			cerr << e.what() << endl;
		}
		return {};
	}
	
	

} // namespace api
