/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GUI_RSC_H_INCLUDED
#define GUI_RSC_H_INCLUDED

#include <gui.h>

class ImagesManager
{
    private:
        static std::vector<std::string> paths;
        static std::vector<wxImage> images;
        static std::vector<wxBitmap> bitmaps;
        
        static int get_index(std::string path);
    public:
        static wxImage get_image(std::string path);
        static wxBitmap get_bitmap(std::string path);
};

#endif // GUI_RSC_H_INCLUDED
