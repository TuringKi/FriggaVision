/*
 *
 * This file is part of the open-source SeetaFace engine, which includes three modules:
 * SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
 *
 * This file is an example of how to use SeetaFace engine for face alignment, the
 * face alignment method described in the following paper:
 *
 *
 *   Coarse-to-Fine Auto-Encoder Networks (CFAN) for Real-Time Face Alignment, 
 *   Jie Zhang, Shiguang Shan, Meina Kan, Xilin Chen. In Proceeding of the
 *   European Conference on Computer Vision (ECCV), 2014
 *
 *
 * Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
 * Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
 *
 * The codes are mainly developed by Jie Zhang (a Ph.D supervised by Prof. Shiguang Shan)
 *
 * As an open-source face recognition engine: you can redistribute SeetaFace source codes
 * and/or modify it under the terms of the BSD 2-Clause License.
 *
 * You should have received a copy of the BSD 2-Clause License along with the software.
 * If not, see < https://opensource.org/licenses/BSD-2-Clause>.
 *
 * Contact Info: you can send an email to SeetaFace@vipl.ict.ac.cn for any problems.
 *
 * Note: the above information must be kept whenever or wherever the codes are used.
 *
 */

 // (C) 2016 chenbingfeng

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "cv.h"
#include "highgui.h"

#include "face_detection.h"
#include "face_alignment.h"


int main(int argc, char** argv)
{

  if (argc < 5) {
    std::cout << "USAGE face_fa detect_model align_model image_list_fn out_list_fn" << std::endl;
    return 0;
  }
  bool need_disp = argc == 6; // if need virtual
  // Initialize face detection model
  seeta::FaceDetection detector(argv[1]);
  detector.SetMinFaceSize(40);
  detector.SetScoreThresh(2.f);
  detector.SetImagePyramidScaleFactor(0.8f);
  detector.SetWindowStep(4, 4);

  // Initialize face alignment model 
  seeta::FaceAlignment point_detector(argv[2]);

  std::ifstream is;
  is.open(argv[3]);
  std::ofstream os;
  os.open(argv[4]);
  int cnt = 0;
  while ((is.rdstate() & std::ifstream::eofbit) == 0) {
    std::string line;
    std::getline(is, line);
    cnt++;
    if (cnt % 7 == 0)
      std::cout << "\r>>>> progress " << cnt << std::flush;


  //load image
  IplImage *img_grayscale = NULL;
  img_grayscale = cvLoadImage(line.c_str(), 0);
  if (img_grayscale == NULL)
  {
    std::cout << std::endl << "error img null" << std::endl;
    continue;
  }

  int pts_num = 5;
  int im_width = img_grayscale->width;
  int im_height = img_grayscale->height;
  unsigned char* data = new unsigned char[im_width * im_height];
  unsigned char* data_ptr = data;
  unsigned char* image_data_ptr = (unsigned char*)img_grayscale->imageData;
  int h = 0;
  for (h = 0; h < im_height; h++) {
    memcpy(data_ptr, image_data_ptr, im_width);
    data_ptr += im_width;
    image_data_ptr += img_grayscale->widthStep;
  }

  seeta::ImageData image_data;
  image_data.data = data;
  image_data.width = im_width;
  image_data.height = im_height;
  image_data.num_channels = 1;

  // Detect faces
  std::vector<seeta::FaceInfo> faces = detector.Detect(image_data);
  int32_t face_num = static_cast<int32_t>(faces.size());

  if (face_num == 0)
  {
    delete[]data;
    cvReleaseImage(&img_grayscale);

    continue;
  }

  // Detect 5 facial landmarks
  seeta::FacialLandmark points[5];
  point_detector.PointDetectLandmarks(image_data, faces[0], points);

  // write out results
  os << line << " " << faces[0].bbox.x << " " << faces[0].bbox.y << " " << faces[0].bbox.width << " " << faces[0].bbox.height;
  for (int i = 0; i<pts_num; i++)
  {
    os << " " << points[i].x << " " << points[i].y;
  }
  os << std::endl;

  // Visualize the results
  if (need_disp) {
    IplImage *img_color = cvLoadImage(line.c_str(), 1);
    cvRectangle(img_color, cvPoint(faces[0].bbox.x, faces[0].bbox.y), cvPoint(faces[0].bbox.x + faces[0].bbox.width - 1, faces[0].bbox.y + faces[0].bbox.height - 1), CV_RGB(255, 0, 0));
    for (int i = 0; i<pts_num; i++)
    {
      cvCircle(img_color, cvPoint(points[i].x, points[i].y), 2, CV_RGB(0, 255, 0), CV_FILLED);
    }
    cvSaveImage((line + ".r.jpg").c_str(), img_color);
    cvReleaseImage(&img_color);
  }
  

  // Release memory
  cvReleaseImage(&img_grayscale);
  delete[]data;
  }
  std::cout << std::endl << "EnD " << cnt << std::endl;

return 0;
}
