# VA_learning

The collection of projects for practice in my process of learning video and audio.

## Folder Structure

- `simplest_mediadata_process`: This folder contains the code for practical demos of video and audio data processing, which I learned and implemented based on the introductory series articles on leixiaohua. [Here](https://blog.csdn.net/leixiaohua1020/) is his homepage on CSDN.

  - `attach`: Subfolder for attachment files, which include demo data files such as audio and video.
  
  - `output`: Subfolder for output files corresponding to the demo data.
  
  - `main.cpp`: The main entry file.
  
  - `pixel.cpp`: Source code file for handling RGB and YUV pixel data.
  - `pixel.h`: Header file containing declarations and definitions related to handling RGB and YUV pixel data.
  > Note: The implementation of `pixel.cpp` and `pixel.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50534150)
  
  - `pcm.cpp`: Source code file for handling PCM audio sample data.
  - `pcm.h`: Header file containing declarations and definitions related to handling PCM audio sample data.
  > Note: The implementation of `pcm.cpp` and `pcm.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50534316).
  
  - `h264.cpp`: Source code file for H.264 video stream parsing.
  - `h264.h`: Header file containing declarations and definitions related to H.264 video stream parsing.
  > Note: The implementation of `h264.cpp` and `h264.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50534369).
  
  - `aac.cpp`: Source code file for AAC audio stream parsing.
  - `aac.h`: Header file containing declarations and definitions related to AAC audio stream parsing.
  > Note: The implementation of `aac.cpp` and `aac.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50535042).
  
  - `flv.cpp`: Source code file for FLV format parsing.
  - `flv.h`: Header file containing declarations and definitions related to FLV format parsing.
  > Note: The implementation of `flv.cpp` and `flv.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50535082).
  
  - `udp.cpp`: Source code file for UDP-RTP protocol parsing.
  - `udp.h`: Header file containing declarations and definitions related to UDP-RTP protocol parsing.
  > Note: The implementation of `udp.cpp` and `udp.h` was developed with reference to [this article](http://blog.csdn.net/leixiaohua1020/article/details/50535230).
